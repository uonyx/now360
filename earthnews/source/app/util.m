//
//  ui.c
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#import "util.h"
#import "ui_ctrlr.h"
#import <UIKit/UIKit.h>
#import <sys/utsname.h>
#import <sys/types.h>
#import <sys/xattr.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define STATUS_BAR_DISPLAY_TIMER (5.0f)
#define MAX_PROFANITY_WORD_COUNT (128)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_font *g_font [NUM_FONT_IDS];
static UIActivityIndicatorView *g_activityIndicatorView = nil;
static int g_activityRefCount = 0;
static screen_fade_type_t g_screenFadeType;
static float g_screenFadeOpacity = 1.0f;
static anim_t g_screenFade;

static status_bar_msg_t g_msg = STATUS_BAR_MSG_NONE;
static float g_statusTimer = STATUS_BAR_DISPLAY_TIMER;
static cx_texture *g_status_msg_icon = NULL;
static cx_colour g_status_msg_colour [NUM_STATUS_BAR_MSGS];
static NSString *g_status_msg_text [NUM_STATUS_BAR_MSGS];

static unsigned int g_profanityWordCount = 0;
static char *g_profanityWords [MAX_PROFANITY_WORD_COUNT] = { NULL };

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_init_create_fonts (void);
static void util_deinit_destroy_fonts (void);
static bool util_init_activity_indicator (const void *rootvc);
static void util_deinit_activity_indicator (void);
static void util_init_screen_fade (void);
static void util_deinit_screen_fade (void);
static void util_init_status_bar (void);
static void util_deinit_status_bar (void);
static void util_init_profanity_filter (void);
static void util_deinit_profanity_filter (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_init (const void *rootvc)
{  
  util_init_activity_indicator (rootvc);
  
  util_init_status_bar ();
  
  util_init_screen_fade ();
  
  util_init_profanity_filter ();
  
  //NSLog(@"%@", [NSTimeZone knownTimeZoneNames]);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_deinit (void)
{
  util_deinit_activity_indicator ();
    
  util_deinit_status_bar ();
  
  util_deinit_screen_fade ();
  
  util_deinit_profanity_filter ();
  
  util_deinit_destroy_fonts ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_thread_init (void)
{
  util_init_create_fonts ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int util_get_dst_offset_secs (const char *tzn)
{  
  CX_ASSERT (tzn);
  
  NSTimeZone *tz = [NSTimeZone timeZoneWithName:[NSString stringWithCString:tzn encoding:NSASCIIStringEncoding]];
  
  int dstOffsetSecs = (tz == nil) ? 0 : (int) [tz daylightSavingTimeOffset];
  
  return dstOffsetSecs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_font *util_get_font (font_id_t fontId)
{
  CX_ASSERT ((fontId > FONT_ID_INVALID) && (fontId < NUM_FONT_IDS));
  
  return g_font [fontId];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_anim_start (anim_t *anim, anim_type_t type, float duration, anim_finished_callback fn, void *fndata)
{
  CX_ASSERT (anim);
  
  bool ret = false;
  
  if (!anim->on)
  {
    anim->on = true;
    anim->duration = duration;
    anim->now = 0.0f;
    anim->t = 0.0f;
    anim->type = type;
    anim->fn = fn;
    anim->fndata = fndata;
    
    ret = true;
  }
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_anim_stop (anim_t *anim)
{
  CX_ASSERT (anim);
  
  if (anim->on)
  {
    anim->on = false;
    anim->duration = 0.0f;
    anim->now = 0.0f;
    anim->t = 0.0f;
    anim->type = ANIM_TYPE_INVALID;
    anim->fn = NULL;
    anim->fndata = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_anim_update (anim_t *anim, float deltaTime)
{
  CX_ASSERT (anim);
  
  if (anim->on)
  {  
    anim->now += deltaTime;
    anim->t = anim->now / anim->duration;
    anim->t = cx_clamp (anim->t, 0.0f, 1.0f);
    
    if (anim->now > anim->duration)
    {
      if (anim->fn)
      {
        anim->fn (anim->fndata);
      }
      
      anim->on = false;
    }
  }
  
  return anim->on;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_activity_indicator_set_active (bool active)
{
  if (active)
  {
    if (g_activityRefCount <= 0)
    {
      [g_activityIndicatorView startAnimating];
    }
    else
    {
      g_activityRefCount += 1;
    }
  }
  else
  {
    if (g_activityRefCount <= 0)
    {
      [g_activityIndicatorView stopAnimating];
    }
    else
    {
      g_activityRefCount -= 1;
    }
  }
} 

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_activity_indicator_get_active (void)
{
  return [g_activityIndicatorView isAnimating];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_status_bar_set_msg (status_bar_msg_t msg)
{
  CX_ASSERT (msg > STATUS_BAR_MSG_NONE);
  CX_ASSERT (msg < NUM_STATUS_BAR_MSGS);
  
  if (msg != g_msg)
  {
    g_statusTimer = STATUS_BAR_DISPLAY_TIMER;
  }
  
  g_msg = msg;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_status_bar_render (void)
{
  if ((g_msg > STATUS_BAR_MSG_NONE) && (g_msg < NUM_STATUS_BAR_MSGS))
  {    
    cx_texture *icon = g_status_msg_icon;
    NSString *tag = g_status_msg_text [g_msg];
    cx_colour *colour = &g_status_msg_colour [g_msg];
    
    CX_ASSERT (icon);
    CX_ASSERT (tag);
    
    NSString *nstext = NSLocalizedString (tag, nil);
    const char *text = [nstext cStringUsingEncoding:NSUTF8StringEncoding];
    
    float alpha = 1.0f;
    
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    g_statusTimer -= deltaTime;
    
    if (g_statusTimer <= 0.0f)
    {
      g_msg = STATUS_BAR_MSG_NONE;
      alpha = 0.0f;
    }
    else 
    {
      const float fadeCutoff = STATUS_BAR_DISPLAY_TIMER * 0.1f;
      
      if (g_statusTimer < fadeCutoff) // bottom end
      {
        alpha = g_statusTimer / fadeCutoff;
      }
      else if (g_statusTimer > (STATUS_BAR_DISPLAY_TIMER - fadeCutoff)) // top end
      {
        alpha = (STATUS_BAR_DISPLAY_TIMER - g_statusTimer) / fadeCutoff;
      }
    }
    
    colour->a = alpha;
    
    // icon
    
    float w = 36.0f;
    float h = 24.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;
    
    // image
    
    float bw = (float) icon->width;
    float bh = (float) icon->height;
    
    float bx1 = x1 + ((w - bw) * 0.5f);
    float by1 = y1 + ((h - bh) * 0.5f);
    float bx2 = bx1 + bw;
    float by2 = by1 + bh;
    
    cx_draw_quad (bx1, by1, bx2, by2, 0.0f, 0.0f, colour, icon);
    
    // text
    
    const cx_font *font = util_get_font (FONT_ID_DEFAULT_14);
    CX_ASSERT (font);
    
    float xoffset = 0.0f;
    float yoffset = 2.0f;
    
    float x = x1 + w + xoffset;
    float y = 0.0f + yoffset;
    
    cx_font_render (font, text, x, y, 0.0f, 0, colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_screen_fade_render (float deltaTime)
{
  if (util_anim_update (&g_screenFade, deltaTime))
  {
  }
  
  float opacity = g_screenFadeType ? g_screenFade.t : (1.0f - g_screenFade.t);
  
  if (fabsf (opacity - 0.0f) > CX_EPSILON)
  {
    float screenWidth = cx_gdi_get_screen_width ();
    float screenHeight = cx_gdi_get_screen_height ();
    
    cx_colour colour = *cx_colour_black ();
    colour.a *= (opacity * g_screenFadeOpacity);
    
    cx_draw_quad (0.0f, 0.0f, screenWidth, screenHeight, 0.0f, 0.0f, &colour, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_screen_fade_trigger (screen_fade_type_t type, float opacity, float secs, anim_finished_callback fn, void *fndata)
{
  if (type != g_screenFadeType)
  {
    util_anim_stop (&g_screenFade);
  }
  
  if (util_anim_start (&g_screenFade, ANIM_TYPE_LINEAR, secs, fn, fndata))
  {
    g_screenFadeType = type;
    g_screenFadeOpacity = opacity;
    
    return true;
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

device_type_t util_get_device_type (void)
{
  device_type_t deviceType = DEVICE_TYPE_INVALID;
  
  struct utsname systemInfo;
  
  uname (&systemInfo);
  
  const char *devicTypeStr = systemInfo.machine;
  
  if (strstr (devicTypeStr, "iPad1"))
  {
    deviceType = DEVICE_TYPE_IPAD1;
  }
  else if (strstr (devicTypeStr, "iPad2"))
  {
    deviceType = DEVICE_TYPE_IPAD2;
  }
  else if (strstr (devicTypeStr, "iPad3"))
  {
    deviceType = DEVICE_TYPE_IPAD3;
  }
  else if (strstr (devicTypeStr, "iPad"))
  {
    // future devices
    deviceType = DEVICE_TYPE_UNKNOWN;
  }
  
  return deviceType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_profanity_filter (char *text)
{
  cx_util_word_filter (text, (const char **) g_profanityWords, g_profanityWordCount, '*');
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int util_get_translation (char *dest, int destSize, const char *tag)
{
  CX_ASSERT (tag);
  
  NSString *nstag = [NSString stringWithCString:tag encoding:NSUTF8StringEncoding];
  NSString *nstext = NSLocalizedString (nstag, nil);
  const char *text = [nstext cStringUsingEncoding:NSUTF8StringEncoding];
  
  int len = cx_sprintf (dest, destSize, "%s", text);
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_add_skip_backup_attribute_to_path (const char *filepath, cx_file_storage_base storage)
{
  bool success = false;
  
  char storagePath [512];
  
  cx_file_storage_path (storagePath, 512, filepath, storage);
  
  u_int8_t b = 1;
  
  int r = setxattr (storagePath, "com.apple.MobileBackup", &b, 1, 0, 0);
  
  success = (r == 0);
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_init_status_bar (void)
{
  g_status_msg_icon = cx_texture_create_from_file ("data/images/ui/warning-16.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  CX_ASSERT (g_status_msg_icon);
  
  g_status_msg_text [STATUS_BAR_MSG_CONNECTION_ERROR] = @"TXT_NO_INTERNET_CONNECTION"; //"Network Connection Error",
  g_status_msg_text [STATUS_BAR_MSG_NEWS_COMMS_ERROR] = @"TXT_NEWS_CONNECTION_ERROR";
  g_status_msg_text [STATUS_BAR_MSG_TWITTER_COMMS_ERROR] = @"TXT_TWITTER_CONNECTION_ERROR";
  g_status_msg_text [STATUS_BAR_MSG_WEATHER_COMMS_ERROR] = @"TXT_WEATHER_CONNECTION_ERROR";
  g_status_msg_text [STATUS_BAR_MSG_IOS_MEMORY_WARNING] = @"TXT_IOS_MEMORY_WARNING";
  
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_CONNECTION_ERROR], 0.93f, 0.2f, 0.2f, 1.0f); // red
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_NEWS_COMMS_ERROR], 0.93f, 0.93f, 0.2f, 1.0f); // yellow
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_TWITTER_COMMS_ERROR], 0.93f, 0.93f, 0.2f, 1.0f); // yellow
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_WEATHER_COMMS_ERROR], 0.93f, 0.93f, 0.2f, 1.0f); // yellow
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_IOS_MEMORY_WARNING], 0.93f, 0.5f, 0.0f, 1.0f); // orange
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_status_bar (void)
{
  cx_texture_destroy (g_status_msg_icon);
}
  
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_init_screen_fade (void)
{
  memset (&g_screenFade, 0, sizeof (g_screenFade));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_screen_fade (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_init_create_fonts (void)
{
  // support CJK code points for mplus-1c-bold
  cxu32 cjkCodePtStart = 19968;
  cxu32 cjkCodePtEnd = 26230;
  cxu32 cjkCodePtsCount = (cjkCodePtEnd - cjkCodePtStart) + 1;
  cxu32 cjkCodePts [cjkCodePtsCount];
  for (cxu32 i = cjkCodePtStart, c = 0; c < cjkCodePtsCount; ++i, ++c)
  {
    cjkCodePts [c] = i;
  }
  
  {
    const char *fontname = "data/fonts/mplus-1c-bold.ttf";
    
    cx_str_unicode_block blocks [] =
    {
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
    };
    
    cxu32 codePts [] =
    {
      0x2103, // degree celsius
      0x2109, // degree farenheit,
    };
    
    cxu32 codePtsCount = sizeof (codePts) / sizeof (cxu32);
    cxu32 blocksCount = sizeof (blocks) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_DEFAULT_12] = cx_font_create (fontname, 14.0f, blocks, blocksCount, codePts, codePtsCount);
    g_font [FONT_ID_DEFAULT_14] = cx_font_create (fontname, 15.0f, blocks, blocksCount, codePts, codePtsCount);
  }
  
  {
    // use for audio track display and clock
    const char *fontname = "data/fonts/mplus-1c-bold.ttf";
  
    cx_str_unicode_block blocks0 [] =
    {
      //CX_STR_UNICODE_BLOCK_CJK_FULL,
      CX_STR_UNICODE_BLOCK_CYRILLIC,
      CX_STR_UNICODE_BLOCK_GREEK_COPTIC,
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A,
      CX_STR_UNICODE_BLOCK_KATAKANA,
      CX_STR_UNICODE_BLOCK_HIRAGANA,
    };
    
    cxu32 blocksCount0 = sizeof (blocks0) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_DEFAULT_16] = cx_font_create (fontname, 16.0f, blocks0, blocksCount0, cjkCodePts, cjkCodePtsCount);
  }

  {
    // news font: suppport for english-only news
    const char *fontname = "data/fonts/mplus-1c-bold.ttf"; // no arabic support
    
    cx_str_unicode_block blocks1 [] =
    {
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS,
      CX_STR_UNICODE_BLOCK_GENERAL_PUNCTUATION,
    };
    
    cxu32 codePts1 [] =
    {
      0x2103, // degree celsius
      0x2109, // degree farenheit,
      0x2122, // TM
      0x2126, // omega
    };
    
    cxu32 codePtsCount1 = sizeof (codePts1) / sizeof (cxu32);
    cxu32 blocksCount1 = sizeof (blocks1) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_NEWS_18] = cx_font_create (fontname, 20.0f, blocks1, blocksCount1, codePts1, codePtsCount1);
  }
  
  {
    // twitter: support for as many languages as is possible
    const char *fontname = "data/fonts/mplus-1c-medium.ttf"; // no arabic support
    
    cx_str_unicode_block blocks2 [] =
    {
      //CX_STR_UNICODE_BLOCK_CJK_FULL,
      CX_STR_UNICODE_BLOCK_CYRILLIC,
      CX_STR_UNICODE_BLOCK_GREEK_COPTIC,
      CX_STR_UNICODE_BLOCK_HEBREW,
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_1_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_B,
      CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS,
      CX_STR_UNICODE_BLOCK_GENERAL_PUNCTUATION,
      CX_STR_UNICODE_BLOCK_KATAKANA,
      CX_STR_UNICODE_BLOCK_HIRAGANA,
      CX_STR_UNICODE_BLOCK_LETTERLIKE_SYMBOLS,
    };
    
    cxu32 blocksCount2 = sizeof (blocks2) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_TWITTER_16] = cx_font_create (fontname, 18.0f, blocks2, blocksCount2, cjkCodePts, cjkCodePtsCount);
  }
  
#if CX_DEBUG
  for (int i = FONT_ID_INVALID + 1; i < NUM_FONT_IDS; ++i)
  {
    CX_ASSERT (g_font [i]);
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_destroy_fonts (void)
{
  for (unsigned int i = 0; i < NUM_FONT_IDS; ++i)
  {
    cx_font_destroy (g_font [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool util_init_activity_indicator (const void *rootvc)
{
  CX_ASSERT (rootvc);
  
  UIViewController *rootViewCtrlr = (UIViewController *) rootvc;
  
  g_activityIndicatorView = [[UIActivityIndicatorView alloc] initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleWhite];
  
  CGRect frame = g_activityIndicatorView.frame;
  frame.origin.x = 8.0f;
  frame.origin.y = 2.0f;
  
  [g_activityIndicatorView setFrame:frame];
  
  [rootViewCtrlr.view addSubview:g_activityIndicatorView];
  
  g_activityRefCount = 0;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_activity_indicator (void)
{
  [g_activityIndicatorView removeFromSuperview];
  [g_activityIndicatorView release];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_init_profanity_filter (void)
{
  cxu8 *buffer = NULL;
  cxu32 bufferSize = 0;
  
  if (cx_file_storage_load_contents (&buffer, &bufferSize, "data/profanity.list", CX_FILE_STORAGE_BASE_RESOURCE))
  {
    g_profanityWordCount = cx_str_explode (g_profanityWords, MAX_PROFANITY_WORD_COUNT, (const char *) buffer, ',');
    
    cx_free (buffer);
  }

#if (CX_DEBUG && 0) // debug test
  const char *text = "piece of shit punkass classic assassin ass in the wind bitch";
  
  char t [256];
  
  cx_strcpy (t, 256, text);
  
  CX_LOG_CONSOLE (1, "%s", t);
  
  util_profanity_filter (t);
  
  CX_LOG_CONSOLE (1, "%s", t);
  
  CX_DEBUG_BREAKABLE_EXPR;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_profanity_filter (void)
{
  for (cxu32 i = 0; i < g_profanityWordCount; ++i)
  {
    cx_free (g_profanityWords [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
