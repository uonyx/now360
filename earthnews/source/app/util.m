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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define STATUS_BAR_DISPLAY_TIMER (5.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_font *g_font [NUM_FONT_SIZES];
static UIActivityIndicatorView *g_activityIndicatorView = nil;
static int g_activityRefCount = 0;
static screen_fade_type_t g_screenFadeType;
static float g_screenFadeOpacity = 1.0f;
static anim_t g_screenFade;

static status_bar_msg_t g_msg = STATUS_BAR_MSG_NONE;
static float g_statusTimer = STATUS_BAR_DISPLAY_TIMER;
static cx_texture *g_statug_msg_icon [NUM_STATUS_BAR_MSGS] = { 0 };
static cx_colour g_statug_msg_colour [NUM_STATUS_BAR_MSGS];
static const char *g_statug_msg_text [NUM_STATUS_BAR_MSGS] =
{
  //"Network Connection Error",
  "No Internet Connection",
  "News Service Connection Error",
  "Weather Service Connection Error",
  "Twitter Connection Error",
};

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool util_init (const void *rootvc)
{
  util_init_create_fonts ();
  
  util_init_activity_indicator (rootvc);
  
  util_init_status_bar ();
  
  util_init_screen_fade ();
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_deinit (void)
{
  util_deinit_activity_indicator ();
  
  util_deinit_destroy_fonts ();
  
  util_deinit_status_bar ();
  
  util_deinit_screen_fade ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int util_get_dst_offset_secs (const char *tzname)
{
#if 0
  NSLog(@"%@", [NSTimeZone knownTimeZoneNames]);
  
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
  
  if (*tzname == 0)
  {
    CX_DEBUG_BREAKABLE_EXPR;
  }
  
  NSTimeZone *tz = [NSTimeZone timeZoneWithName:[NSString stringWithCString:tzname encoding:NSASCIIStringEncoding]];
  
  int dstOffsetSecs = (tz == nil) ? 0 : (int) [tz daylightSavingTimeOffset];
  
  return dstOffsetSecs;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_font *util_get_font (font_size_t size)
{
  CX_ASSERT ((size > FONT_SIZE_INVALID) && (size < NUM_FONT_SIZES));
  
  return g_font [size];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void util_render_fps (void)
{
#if 0
  static float fps = 0.0f;
  static float totalTime = 0.0f;
  static unsigned int frameCount = 0;
  
  frameCount++;
  
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  //CX_DEBUGLOG_CONSOLE (1, "%.2f", deltaTime);
  
  totalTime += deltaTime;
  
  if (totalTime >= 1.0f)
  {
    fps = (float) frameCount / totalTime;
    frameCount = 0;
    totalTime = 0.0f;
  }
  
  fps = 1.0f/ deltaTime;
  
  char fpsStr [32];
  
  cx_sprintf (fpsStr, 32, "%.2f", fps);
  
  const cx_font *font = util_get_font (FONT_SIZE_12); 
  
  float sw = 1024.0f;
  float sh = 768.0f;
  
  cx_font_render (font, fpsStr, (sw - 24.0f) - 2.0f, sh - 12.0f, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_red ());
#endif
  
#if 1
  
  const unsigned int textLen = 255 - 32; // 161;
  const unsigned int textBufLen = textLen + 1;
  
  unsigned char text [textBufLen];
  
  for (unsigned char i = 0; i < textLen; ++i)
  {
    unsigned char ch = i + 32;
    
    text [i] = ch;
  }
  
  text [textLen] = 0;
  
  const cx_font *font = util_get_font (FONT_SIZE_12);
  
  cx_font_render (font, (const char *) text, 1.0f, 744.0f, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_cyan ());
  
#endif
  
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
    
    return true;
  }
  else
  {
    return false;
  }
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
    cx_texture *icon = g_statug_msg_icon [g_msg];
    const char *text = g_statug_msg_text [g_msg];
    cx_colour *colour = &g_statug_msg_colour [g_msg];
    
    CX_ASSERT (icon);
    CX_ASSERT (text);
    
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
    
    const cx_font *font = util_get_font (FONT_SIZE_14);
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

static void util_init_status_bar (void)
{
  g_statug_msg_icon [STATUS_BAR_MSG_CONNECTION_ERROR] = cx_texture_create_from_file ("data/images/ui/warning-16.png", CX_FILE_STORAGE_BASE_RESOURCE);
  g_statug_msg_icon [STATUS_BAR_MSG_NEWS_COMMS_ERROR] = cx_texture_create_from_file ("data/images/ui/warning-16.png", CX_FILE_STORAGE_BASE_RESOURCE);
  g_statug_msg_icon [STATUS_BAR_MSG_WEATHER_COMMS_ERROR] = cx_texture_create_from_file ("data/images/ui/warning-16.png", CX_FILE_STORAGE_BASE_RESOURCE);
  g_statug_msg_icon [STATUS_BAR_MSG_TWITTER_COMMS_ERROR] = cx_texture_create_from_file ("data/images/ui/warning-16.png", CX_FILE_STORAGE_BASE_RESOURCE);
  
  CX_ASSERT (g_statug_msg_icon [STATUS_BAR_MSG_CONNECTION_ERROR]);
  CX_ASSERT (g_statug_msg_icon [STATUS_BAR_MSG_NEWS_COMMS_ERROR]);
  CX_ASSERT (g_statug_msg_icon [STATUS_BAR_MSG_WEATHER_COMMS_ERROR]);
  CX_ASSERT (g_statug_msg_icon [STATUS_BAR_MSG_TWITTER_COMMS_ERROR]);
  
  cx_colour_set (&g_statug_msg_colour [STATUS_BAR_MSG_CONNECTION_ERROR], 0.9f, 0.2f, 0.2f, 1.0f);
  cx_colour_set (&g_statug_msg_colour [STATUS_BAR_MSG_TWITTER_COMMS_ERROR], 0.9f, 0.9f, 0.2f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_status_bar (void)
{
  for (int i = 0; i < NUM_STATUS_BAR_MSGS; ++i)
  {
    cx_texture *texture = g_statug_msg_icon [i];
    
    if (texture)
    {
      cx_texture_destroy (texture);
    }
  }
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
  //const char *fontname = "data/fonts/verdana.ttf";
  const char *fontname = "data/fonts/tahoma.ttf";
  
  g_font [FONT_SIZE_10] = cx_font_create (fontname, 10.0f);
  g_font [FONT_SIZE_12] = cx_font_create (fontname, 12.0f);
  g_font [FONT_SIZE_14] = cx_font_create (fontname, 14.0f);
  g_font [FONT_SIZE_16] = cx_font_create (fontname, 16.0f);
  g_font [FONT_SIZE_18] = cx_font_create (fontname, 18.0f);
  g_font [FONT_SIZE_20] = cx_font_create (fontname, 20.0f);
  g_font [FONT_SIZE_24] = cx_font_create (fontname, 24.0f); 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void util_deinit_destroy_fonts (void)
{
  for (unsigned int i = 0; i < NUM_FONT_SIZES; ++i)
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
