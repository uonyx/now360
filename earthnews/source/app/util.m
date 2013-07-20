//
//  ui.c
//  earthnews
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
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
#if 0
  //NSTimeZone *tz2 = [NSTimeZone timeZoneWithAbbreviation:@"EST"]; // new york
  //NSTimeZone *tz2 = [NSTimeZone timeZoneWithAbbreviation:@"GMT"]; // uk
  //NSTimeZone *tz2 = [NSTimeZone timeZoneWithAbbreviation:@"BST"]; // uk
  //NSTimeZone *tz2 = [NSTimeZone timeZoneWithAbbreviation:@"CET"]; // europe
  NSTimeZone *tz2 = [NSTimeZone timeZoneWithName:@"Australia/Sydney"];
  
  bool dst2 = [tz2 isDaylightSavingTime];
  (void) dst2;
  
  NSTimeInterval dst2OffsetSecs = [tz2 daylightSavingTimeOffset];
  (void) dst2OffsetSecs;
  
  NSTimeInterval dst2OffsetDateSecs = [tz2 daylightSavingTimeOffsetForDate:[NSDate date]];
  (void) dst2OffsetDateSecs;
  
#if 0
  NSCalendarUnit unitFlags = NSYearCalendarUnit | NSMonthCalendarUnit |  NSDayCalendarUnit | NSHourCalendarUnit | NSMinuteCalendarUnit | NSSecondCalendarUnit;
  
  NSCalendar *gregorian = [[NSCalendar alloc] initWithCalendarIdentifier: NSGregorianCalendar];
  
  [gregorian setTimeZone:tz2];
  
  NSDate *date = [NSDate date];
  
  NSDateComponents *dateComponents = [gregorian components:unitFlags fromDate:date];
  
  NSInteger year = [dateComponents year];
  NSInteger month = [dateComponents month];
  NSInteger day = [dateComponents day];
  NSInteger hour = [dateComponents hour];
  NSInteger minute = [dateComponents minute];
  NSInteger second = [dateComponents second];
  
  (void) year;
  (void) month;
  (void) day;
  (void) hour;
  (void) minute;
  (void) second;
  
  [gregorian release];
#endif
#endif
  
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
      
      if (g_statusTimer < fadeCutoff)
      {
        alpha = g_statusTimer / fadeCutoff;
      }
      
      if (g_statusTimer > (STATUS_BAR_DISPLAY_TIMER - fadeCutoff))
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

void util_profanity_filter (const char *text)
{
  cx_util_word_filter (text, (const char **) g_profanityWords, g_profanityWordCount, '*');
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
  
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_CONNECTION_ERROR], 0.9f, 0.2f, 0.2f, 1.0f); // red
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_NEWS_COMMS_ERROR], 0.9f, 0.9f, 0.2f, 1.0f); // yellow
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_TWITTER_COMMS_ERROR], 0.9f, 0.9f, 0.2f, 1.0f); // yellow
  cx_colour_set (&g_status_msg_colour [STATUS_BAR_MSG_WEATHER_COMMS_ERROR], 0.9f, 0.9f, 0.2f, 1.0f); // yellow
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
  {
    // earth text and music 
    //const char *fontname = "data/fonts/tahoma.ttf";
    //const char *fontname = "data/fonts/UnDotum.ttf";
    //const char *fontname = "data/fonts/mplus-1c-light.ttf";
    //const char *fontname = "data/fonts/mplus-1c-regular.ttf";
    //const char *fontname = "data/fonts/mplus-1c-medium.ttf";
    const char *fontname = "data/fonts/mplus-1c-bold.ttf";
    
    cx_str_unicode_block blocks [] =
    {
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
    };
    
    cxu32 codePts [] =
    {
      0xb0, // degree symbol
    };
    
    cxu32 codePtsCount = sizeof (codePts) / sizeof (cxu32);
    
    cxu32 blocksCount = sizeof (blocks) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_DEFAULT_12] = cx_font_create (fontname, 14.0f, blocks, blocksCount, codePts, codePtsCount);
    g_font [FONT_ID_DEFAULT_14] = cx_font_create (fontname, 15.0f, blocks, blocksCount, codePts, codePtsCount);
    g_font [FONT_ID_DEFAULT_16] = cx_font_create (fontname, 16.0f, blocks, blocksCount, NULL, 0);
  }
  
  {
    //const char *fontname = "data/fonts/UnDotum.ttf";
    const char *fontname = "data/fonts/mplus-1c-bold.ttf";
  
    cx_str_unicode_block blocks0 [] =
    {
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A
    };
    
    cxu32 blocksCount0 = sizeof (blocks0) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_MUSIC_16] = cx_font_create (fontname, 16.0f, blocks0, blocksCount0, NULL, 0);
  }
  
  {
    //const char *fontname = "data/fonts/UnDotumBold.ttf";
    //const char *fontname = "data/fonts/mplus-1c-regular.ttf";
    //const char *fontname = "data/fonts/mplus-1c-medium.ttf";
    const char *fontname = "data/fonts/mplus-1c-bold.ttf";
    
    cx_str_unicode_block blocks1 [] =
    {
      //CX_STR_UNICODE_BLOCK_ARABIC,
      CX_STR_UNICODE_BLOCK_CJK_FULL,
      CX_STR_UNICODE_BLOCK_CYRILLIC,
      CX_STR_UNICODE_BLOCK_GREEK_COPTIC,
      CX_STR_UNICODE_BLOCK_HEBREW,
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A,
      CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS,
    };
    
    cxu32 blocksCount1 = sizeof (blocks1) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_NEWS_18] = cx_font_create (fontname, 20.0f, blocks1, blocksCount1, NULL, 0);
  }
  
  {
    //const char *fontname = "data/fonts/CODE2000.TTF";
    //const char *fontname = "data/fonts/UnDotum.ttf";
    //const char *fontname = "data/fonts/mplus-1c-regular.ttf";
    //const char *fontname = "data/fonts/mplus-1c-bold.ttf";
    const char *fontname = "data/fonts/mplus-1c-medium.ttf";
    
    cx_str_unicode_block blocks2 [] =
    {
      //CX_STR_UNICODE_BLOCK_ARABIC,
      CX_STR_UNICODE_BLOCK_CJK_FULL,
      CX_STR_UNICODE_BLOCK_CYRILLIC,
      CX_STR_UNICODE_BLOCK_GREEK_COPTIC,
      CX_STR_UNICODE_BLOCK_HEBREW,
      CX_STR_UNICODE_BLOCK_LATIN_BASIC,
      CX_STR_UNICODE_BLOCK_LATIN_SUPPLEMENT,
      CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A,
      CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS
    };
    
    cxu32 blocksCount2 = sizeof (blocks2) / sizeof (cx_str_unicode_block);
    
    g_font [FONT_ID_TWITTER_16] = cx_font_create (fontname, 18.0f, blocks2, blocksCount2, NULL, 0);
  }
  
  //cx_font_set_scale (g_font [FONT_ID_TWITTER_16], 16.0f / 14.0f, 16.0f / 14.0f);
  
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
