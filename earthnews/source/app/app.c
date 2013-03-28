//
//  app.c
//  earthnews
//
//  Created by Ubaka Onyechi on 02/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "app.h"
#include "util.h"
#include "camera.h"
#include "feeds.h"
#include "earth.h"
#include "browser.h"
#include "worker.h"
#include "ui_ctrlr.h"
#include "audio.h"
#include "settings.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_PROJECTION_PERSPECTIVE_NEAR   (0.1f)
#define CAMERA_PROJECTION_PERSPECTIVE_FAR    (100.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR  (-1.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_FAR   (1.0f)

#define CAMERA_MIN_FOV (40.0f)
#define CAMERA_MAX_FOV (75.0f)
#define CAMERA_START_FOV (50.0f)

#define CITY_INDEX_INVALID (-1)

#define NUM_LOADING_STAGES 3

#define DEBUG_SHOW_TEMPERATURE 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum 
{
  COMMS_FEEDS_PENDING,
} comms_status_t;

typedef enum
{
  APP_STATE_INVALID,
  APP_STATE_INIT,
  APP_STATE_UPDATE,
  APP_STATE_DEINIT,
} app_state_t;

typedef enum
{
  APP_LOAD_STAGE_INVALID,
  APP_LOAD_STAGE_1_BEGIN,
  APP_LOAD_STAGE_1,
  APP_LOAD_STAGE_1_END,
  APP_LOAD_STAGE_2_BEGIN,
  APP_LOAD_STAGE_2,
  APP_LOAD_STAGE_2_END,
  APP_LOAD_STAGE_3_BEGIN,
  APP_LOAD_STAGE_3,
  APP_LOAD_STAGE_3_END,
} app_load_stage_t;

typedef struct
{
  cx_vec4 *renderPos;
  cx_vec2 *opacity; // x - point, y - text
} render2d_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static earth_t *s_earth = NULL;
static camera_t *s_camera = NULL;
static browser_rect_t s_browserRect;
static render2d_t s_render2dInfo;

#define NEW_ROTATION 1

#if NEW_ROTATION

#define CAMERA_ROTATION_SPEED_EXPONENT_SWIPE (1.0f)
#define CAMERA_ROTATION_SPEED_EXPONENT_CITY_SELECT (4.0f)

static cx_vec2 s_rotAngle;
static cx_vec2 s_rotTarget;
static cxf32 s_rotSpeedExp = 1.0f;

#else
static cx_vec2 s_rotTouchBegin;
static cx_vec2 s_rotTouchEnd;
static cx_vec2 s_rotationSpeed;
static cx_vec2 s_rotationAngle;
#endif

static feed_twitter_t *s_twitterFeeds = NULL;
static feed_news_t *s_newsFeeds = NULL;
static feed_weather_t *s_weatherFeeds = NULL;

static int s_selectedCity = CITY_INDEX_INVALID;
static int s_currentWeatherCity = CITY_INDEX_INVALID;
static bool s_refreshWeather = false;

static cx_thread *s_ldThread = NULL;
static cx_thread_monitor s_ldThreadMonitor;
static app_state_t s_state = APP_STATE_INVALID;

app_load_stage_t s_loadStage = APP_LOAD_STAGE_1_BEGIN;
static cx_texture *s_ldImages [NUM_LOADING_STAGES];
static cx_texture *s_logoTex = NULL;
static bool s_renderLogo = false;
static anim_t s_logoFade;

static cx_texture *glowtex = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static float app_get_zoom_opacity (void);
static bool app_get_city_index_valid (int index);
//static int  app_get_selected_city (void);
//static void app_set_selected_city (int index);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_handle_touch_event (const input_touch_event *event);
static void app_input_handle_gesture_event (const input_gesture_event *event);
static void app_input_touch_began (float x, float y);
static void app_input_touch_moved (float x, float y, float px, float py);
static void app_input_touch_ended (float x, float y);
static void app_input_zoom (float factor);
static int  app_input_touch_earth (float screenX, float screenY, float screenWidth, float screenHeight);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_transition (void *userdata);
static void app_load_stage_update (void);
static void app_load_stage_render (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_camera (float deltaTime);
static void app_update_earth (void);
static void app_update_feeds (void);
static void app_update_feeds_news (void);
static void app_update_feeds_twitter (void);
static void app_update_feeds_weather (void);
static void app_update_feeds_weather_refresh (void);

static void app_render_3d (void);
static void app_render_3d_earth (void);
static void app_render_2d (void);
static void app_render_2d_earth (void);
static void app_render_2d_feeds (void);
static void app_render_2d_local_clock (void);
static void app_render_2d_screen_fade (void);
static void app_render_2d_logo (void);
static void app_render_2d_logo_fade_out (void *data);
static void app_render_load (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_thread_exit_status app_init_load (void *userdata)
{
  cx_gdi_shared_context_create ();
  
  //
  // earth data
  //
  
  s_earth = earth_create ("data/earth_data.json", 1.0f, 128, 64);
  
  settings_set_city_names (s_earth->data->names, s_earth->data->count);
  
  glowtex = cx_texture_create_from_file ("data/textures/glowcircle.gb32-16.png");
  
  //
  // feeds
  //
  
  feeds_init ();
  
  int cityCount = s_earth->data->count;
  
  s_twitterFeeds = cx_malloc (sizeof (feed_twitter_t) * cityCount);
  memset (s_twitterFeeds, 0, sizeof (feed_twitter_t) * cityCount);
  
  s_newsFeeds = cx_malloc (sizeof (feed_news_t) * cityCount);
  memset (s_newsFeeds, 0, sizeof (feed_news_t) * cityCount);
  
  s_weatherFeeds = cx_malloc (sizeof (feed_weather_t) * cityCount);
  memset (s_weatherFeeds, 0, sizeof (feed_weather_t) * cityCount);

  //
  // 2d render info
  //
  
  s_render2dInfo.renderPos = cx_malloc (sizeof (cx_vec4) * cityCount);
  memset (s_render2dInfo.renderPos, 0, (sizeof (cx_vec4) * cityCount));
  
  s_render2dInfo.opacity = cx_malloc (sizeof (cx_vec2) * cityCount);
  memset (s_render2dInfo.opacity, 0, (sizeof (cx_vec2) * cityCount));
  
  //
  // other
  //
#if NEW_ROTATION
  memset (&s_rotAngle, 0, sizeof (s_rotAngle));
  memset (&s_rotTarget, 0, sizeof (s_rotTarget));
#else
  memset (&s_rotTouchBegin, 0, sizeof (s_rotTouchBegin));
  memset (&s_rotTouchEnd, 0, sizeof (s_rotTouchEnd));
  memset (&s_rotationSpeed, 0, sizeof (s_rotationSpeed));
  memset (&s_rotationAngle, 0, sizeof (s_rotationAngle));
#endif
  
  app_update_feeds_weather_refresh ();
  
  cx_gdi_shared_context_destroy ();
  
  cx_thread_monitor_signal (&s_ldThreadMonitor);
  
  return CX_THREAD_EXIT_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_init (void *rootvc, void *gctx, float width, float height)
{ 
  CX_ASSERT (rootvc);
  
  //
  // engine
  //
  
  cx_engine_init_params params;
  memset (&params, 0, sizeof (params));
  params.screenWidth = (int) width;
  params.screenHeight = (int) height;
  params.graphicsContext = gctx;
  
  cx_engine_init (CX_ENGINE_INIT_ALL, &params);
  
  //
  // settings
  //
  
  settings_init (rootvc, "data/settings.json");
  
  //
  // globals
  //
  
  util_init (rootvc);
  
  //
  // audio
  //
  
  audio_init (rootvc);
  
  
  //
  // ui
  //
  
  ui_ctrlr_init (width, height);
  
  
  //
  // browser
  //
  
  browser_init (rootvc);
  
  memset (&s_browserRect, 0, sizeof (s_browserRect));
  
  s_browserRect.width  = 778.0f;
  s_browserRect.height = 620.0f;
  s_browserRect.posX   = 0.0f + ((width - s_browserRect.width) * 0.5f);
  s_browserRect.posY   = 0.0f + ((height - s_browserRect.height) * 0.5f);
  
  //
  // camera
  //
  
  s_camera = camera_create (width / height, CAMERA_START_FOV);
  
  //
  // loading screen
  //
  
  s_logoTex = cx_texture_create_from_file ("data/loading/now360-500px.png");
  s_ldImages [0] = cx_texture_create_from_file ("data/loading/uonyechi.com.png");
  s_ldImages [1] = cx_texture_create_from_file ("data/loading/nasacredit.png");
  s_ldImages [2] = cx_texture_create_from_file ("data/loading/gear.png");
  
  //
  // loading thread
  //
  
  s_state = APP_STATE_INIT;
  
  s_ldThread = cx_thread_create ("init_load", CX_THREAD_TYPE_JOINABLE, app_init_load, NULL);
  
  cx_thread_monitor_init (&s_ldThreadMonitor);
  
  cx_thread_start (s_ldThread);
  
  //
  // test code
  //
  
#if 0
  cx_file file0, file1, file2;
  cx_file_load (&file0, "data/rss.txt");
  cx_file_load (&file1, "data/twitter.txt");
  cx_file_load (&file2, "data/weather2.txt");
  
  feed_news_t rssFeed;
  memset (&rssFeed, 0, sizeof (feed_news_t));
  feeds_news_parse (&rssFeed, file0.data, file0.size);
  
  feed_twitter_t tweets;
  memset (&tweets, 0, sizeof (feed_twitter_t));
  feeds_twitter_parse (&tweets, file1.data, file1.size);
  
  feed_weather_t weather;
  memset(&weather, 0, sizeof (feed_weather_t));
  feeds_weather_parse (&weather, file2.data, file2.size);
  
  feed_news_item_t *rssItem = rssFeed.items;
  while (rssItem)
  {
    CX_DEBUGLOG_CONSOLE(1, "====rss=====");
    CX_DEBUGLOG_CONSOLE(1, rssItem->date);
    CX_DEBUGLOG_CONSOLE(1, rssItem->link);
    CX_DEBUGLOG_CONSOLE(1, rssItem->title);
    rssItem = rssItem->next;
  }
  
  feed_twitter_tweet_t *twItem = tweets.items;
  while (twItem)
  {
    CX_DEBUGLOG_CONSOLE(1, "===twitter===");
    CX_DEBUGLOG_CONSOLE(1, twItem->date);
    CX_DEBUGLOG_CONSOLE(1, twItem->userhandle);
    CX_DEBUGLOG_CONSOLE(1, twItem->username);
    CX_DEBUGLOG_CONSOLE(1, twItem->text);
    twItem = twItem->next;
  }
  
  CX_DEBUG_BREAKABLE_EXPR;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_deinit (void)
{
  feeds_deinit ();
  
  input_deinit ();
  
  util_deinit ();
  
  cx_engine_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_view_resize (float width, float height)
{
  s_camera->aspectRatio = width / height;
  
  cx_gdi_set_screen_dimensions ((int) width, (int) height);
  
  ui_ctrlr_screen_resize (width, height);
  
  s_browserRect.width  = 778.0f;
  s_browserRect.height = 620.0f;
  s_browserRect.posX   = 0.0f + ((width - s_browserRect.width) * 0.5f);
  s_browserRect.posY   = 0.0f + ((height - s_browserRect.height) * 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_update (void)
{
  cx_system_time_update ();

  switch (s_state) 
  {
    case APP_STATE_INIT: 
    {
      if (cx_thread_monitor_wait_timed (&s_ldThreadMonitor, 0))
      {
        // fade screen in
        util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 3.0f, app_render_2d_logo_fade_out, NULL);
        
        //
        // input
        //
        
        input_init ();
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_BEGIN, app_input_handle_touch_event);
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_END, app_input_handle_touch_event);
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_MOVE, app_input_handle_touch_event);
        input_register_gesture_event_callback (INPUT_GESTURE_TYPE_PINCH, app_input_handle_gesture_event);
        
        s_state = APP_STATE_UPDATE;
      }
      
      break; 
    }
      
    case APP_STATE_UPDATE:
    {
      input_update ();
      
      float deltaTime = (float) cx_system_time_get_delta_time ();
      
      deltaTime = cx_min (deltaTime, (1.0f / 60.0f));
      
      app_update_camera (deltaTime);
      
      app_update_earth ();
      
      app_update_feeds ();
      
      break;
    }
      
    default: 
    { 
      break; 
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render (void)
{
  cx_gdi_clear (cx_colour_black ());
  
  switch (s_state) 
  {
    case APP_STATE_INIT: 
    { 
      app_render_load (); 
      
      break; 
    }
    
    case APP_STATE_UPDATE:
    {
      app_render_3d ();
      
      app_render_2d ();
      
      break;
    }
      
    default: 
    { 
      break; 
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static float app_get_zoom_opacity (void)
{
  float fov = s_camera->fov;
  
  //float opacity = 1.0f - cx_smoothstep (CAMERA_MIN_FOV, 56.0f, fov);
  float opacity = 1.0f - cx_smoothstep (CAMERA_START_FOV, CAMERA_START_FOV + 10.0f, fov);
  
  return opacity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool app_get_city_index_valid (int index)
{
  CX_ASSERT (s_earth);
  CX_ASSERT (s_earth->data);
  
  return (index > CITY_INDEX_INVALID) && (index < s_earth->data->count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_transition (void *userdata)
{
  CX_ASSERT (userdata);
  
  app_load_stage_t nextStage = (int) userdata;
  
  s_loadStage = nextStage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_update (void)
{
  const float deltaTime = (float) cx_system_time_get_delta_time ();
  
  switch (s_loadStage) 
  {
    case APP_LOAD_STAGE_1_BEGIN:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_1);
      break;
    }
      
    case APP_LOAD_STAGE_1:
    {
      static float timer = 3.0f;
      timer -= deltaTime;
      
      if (timer <= 0.0f)
      {
        s_loadStage = APP_LOAD_STAGE_1_END;
      }
      
      break;
    }
      
    case APP_LOAD_STAGE_1_END:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_2_BEGIN);
      break;
    }
      
    case APP_LOAD_STAGE_2_BEGIN:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_2);
      break;
    }
      
    case APP_LOAD_STAGE_2:
    {
      static float timer = 3.0f;
      timer -= deltaTime;
      
      if (timer <= 0.0f)
      {
        s_loadStage = APP_LOAD_STAGE_2_END;
      }
      
      break;
    }
      
    case APP_LOAD_STAGE_2_END:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_OUT, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_3_BEGIN);
      break;
    }
      
    case APP_LOAD_STAGE_3_BEGIN:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_3);
      s_renderLogo = true;
      break;
    }
      
    case APP_LOAD_STAGE_3:
    {
      break;
    }
      
    default:
    {
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_render (void)
{
  const float deltaTime = (float) cx_system_time_get_delta_time ();
  const float screenWidth = cx_gdi_get_screen_width ();
  const float screenHeight = cx_gdi_get_screen_height ();
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  switch (s_loadStage) 
  {
    case APP_LOAD_STAGE_1_BEGIN:
    case APP_LOAD_STAGE_1:
    case APP_LOAD_STAGE_1_END:
    {
      cx_texture *tex = s_ldImages [0];
      CX_ASSERT (tex);
      
      float tw = (float) tex->width;
      float th = (float) tex->height;
      float tx = 0.0f + (screenWidth - tw) * 0.5f;
      float ty = 0.0f + (screenHeight - th) * 0.5f;
      
      cx_colour col = *cx_colour_white ();
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, tex);
      
      break;
    }
      
    case APP_LOAD_STAGE_2_BEGIN:
    case APP_LOAD_STAGE_2:
    case APP_LOAD_STAGE_2_END:
    {
      cx_texture *tex = s_ldImages [1];
      CX_ASSERT (tex);
      
      float tw = (float) tex->width;
      float th = (float) tex->height;
      float tx = 0.0f + (screenWidth - tw) * 0.5f;
      float ty = 0.0f + (screenHeight - th) * 0.5f;
      
      cx_colour col = *cx_colour_white ();
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, tex);
      
      break;
    }
      
    case APP_LOAD_STAGE_3_BEGIN:
    case APP_LOAD_STAGE_3:
    case APP_LOAD_STAGE_3_END:
    {
      static float rot = 0.0f;
      const float mx = 24.0f;
      const float my = 24.0f;
      
      cx_texture *tex = s_ldImages [2];
      CX_ASSERT (tex);
      
      float tw = (float) tex->width;
      float th = (float) tex->height;
      float tx = (screenWidth - tw - mx);
      float ty = (screenHeight - th - my);
      
      cx_colour col = *cx_colour_white ();
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, rot, &col, tex);
      
      rot += deltaTime * 6.0f;
      rot = fmodf (rot, 360.0f);
      
      break;
    }
      
    default:
    {
      break;
    }
  }
  
  cx_gdi_enable_z_write (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_camera (float deltaTime)
{
  float aspectRatio = cx_gdi_get_aspect_ratio ();
  CX_REFERENCE_UNUSED_VARIABLE (aspectRatio);
  
  const float MaxSpeed = 60.0f;
  CX_REFERENCE_UNUSED_VARIABLE (MaxSpeed);
  
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  CX_REFERENCE_UNUSED_VARIABLE (sw);
  CX_REFERENCE_UNUSED_VARIABLE (sh);
  
#if NEW_ROTATION
  
#if 0
  cx_vec2 desiredVel;
  
  float maxspeed = 1.0;
  
  desiredVel.x = (s_touchTargetPos.x - s_touchPos.x) * maxspeed * 0.25f;
  desiredVel.y = (s_touchTargetPos.y - s_touchPos.y) * maxspeed * 0.25f;
  
  s_touchVel.x = desiredVel.x - s_touchVel.x;
  s_touchVel.y = desiredVel.y - s_touchVel.y;
  
  s_touchPos.x = s_touchPos.x + (s_touchVel.x * deltaTime);
  s_touchPos.y = s_touchPos.y + (s_touchVel.y * deltaTime);
  
  s_rotAngle.x += (s_touchVel.x * deltaTime);
  s_rotAngle.y += (s_touchVel.y * deltaTime);
  
  s_rotAngle.x = fmodf (s_rotAngle.x, 360.0f);
  s_rotAngle.y = cx_clamp (s_rotAngle.y, -89.9f, 89.9f);
#endif
  
#if 1
  float rotAddx = s_rotTarget.x - s_rotAngle.x;
  float rotAddy = s_rotTarget.y - s_rotAngle.y;

  //float alpha = cx_smoothstep (0.98f, 1.0f, dotp);
  float zoomFactor = 1.0f - cx_linearstep (CAMERA_MIN_FOV, CAMERA_MAX_FOV, s_camera->fov);
  
#if 0
  zoomFactor += 1.0f; // range (1.0, 2.0)
#endif
  
#if 0
  zoomFactor = cx_sin ((CX_PI * zoomFactor) + CX_HALF_PI); // sine interpolation // range (-1.0, 1.0)
  zoomFactor = (zoomFactor + 1.0f) * 0.5f; // range (0.0, 1.0)
  zoomFactor += 1.0f; // range (1.0, 2.0)
#endif
  
#if 1
  //zoomFactor *= 2.0f;
  zoomFactor += 1.0f;
  zoomFactor = cx_pow (zoomFactor, 3.0f);
  //zoomFactor += 1.0f;
#endif

  
  float rotDecel = 1.0f * zoomFactor * s_rotSpeedExp;
  float rotSpeed = 10.0f;
  
  float rotSpeedx = (fabsf (rotAddx) / rotDecel);
  float rotSpeedy = (fabsf (rotAddy) / rotDecel);
  
  //x ^ 5
  (void) s_rotSpeedExp;
  
#if 0
  float exponent = 1.0f;
  rotSpeedx = cx_pow (rotSpeedx, exponent);
  rotSpeedy = cx_pow (rotSpeedy, exponent);
#endif
  
#if 0
  rotSpeedx = cx_sin ((CX_PI * rotSpeedx) + CX_HALF_PI);
  rotSpeedy = cx_sin ((CX_PI * rotSpeedy) + CX_HALF_PI);
#endif
  
  if (rotSpeedx >= rotSpeed)
  {
    CX_DEBUGLOG_CONSOLE(1, "ORIA! X");
  }
  
  if (rotSpeedy >= rotSpeed)
  {
    CX_DEBUGLOG_CONSOLE(1, "ORIA! Y");
  }

  rotSpeedx = cx_min (rotSpeedx, rotSpeed);
  rotSpeedy = cx_min (rotSpeedy, rotSpeed);
  
  if (rotSpeedx < 0.1f)
  {
    rotSpeedx = 0.0f;
  }

  if (rotSpeedy < 0.1f)
  {
    rotSpeedy = 0.0f;
  }
  
  s_rotAngle.x += (rotAddx * deltaTime * rotSpeedx);
  s_rotAngle.y += (rotAddy * deltaTime * rotSpeedy);
  
  s_rotAngle.x = fmodf (s_rotAngle.x, 360.0f);
  s_rotAngle.y = cx_clamp (s_rotAngle.y, -89.9f, 89.9f);
#endif
  
  float rotx = s_rotAngle.x;
  float roty = s_rotAngle.y;
  
#else // OLD_ROTATION
  
  cx_vec2 tdist;
  
  tdist.x = s_rotTouchEnd.x - s_rotTouchBegin.x;
  tdist.y = s_rotTouchEnd.y - s_rotTouchBegin.y;

#if 1
  float rx = tdist.x * deltaTime;
  float ry = tdist.y * deltaTime;
  
  s_rotTouchBegin.x += rx;
  s_rotTouchBegin.y += ry;
  
  s_rotationAngle.x += rx;
  s_rotationAngle.y += ry;
  
#else
  s_rotTouchBegin.x += (tdist.x * deltaTime);
  s_rotTouchBegin.y += (tdist.y * deltaTime);
  
  s_rotationAngle.x += (tdist.x * deltaTime);
  s_rotationAngle.y += (tdist.y * deltaTime);
#endif
  
  s_rotationAngle.x = fmodf (s_rotationAngle.x, 360.0f);
  s_rotationAngle.y = cx_clamp (s_rotationAngle.y, -89.9f, 89.9f);
  
  float rotx = s_rotationAngle.x;
  float roty = s_rotationAngle.y;
  
#endif // END_ROTATION
  
#if 1
  cx_vec4_set (&s_camera->position, 0.0f, 0.0f, -2.0f, 1.0f);
  cx_vec4_set (&s_camera->target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update camera (view and projetion matrix)

  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  cx_vec4 axis_x = {{1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, -1.0f, 0.0f, 0.0f}};
  
  camera_rotate_around_point (s_camera, &center, cx_rad (roty), &axis_x);
  camera_rotate_around_point (s_camera, &center, cx_rad (rotx), &axis_y);
  
  // get projection matrix
  cx_mat4x4 projmatrix;
  camera_get_projection_matrix (s_camera, &projmatrix);
  
  // get view matrix
  cx_mat4x4 viewmatrix;
  camera_get_view_matrix (s_camera, &viewmatrix);
  
  // compute modelviewprojection matrix
  cx_mat4x4 mvpMatrix;
  cx_mat4x4_mul (&mvpMatrix, &projmatrix, &viewmatrix);
  
  //camera_update (s_camera, deltaTime);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_P, &projmatrix);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MV, &viewmatrix);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
#else
  
  cx_mat4x4 proj;
  cx_mat4x4_perspective (&proj, cx_rad (65.0f), aspectRatio, CAMERA_PROJECTION_PERSPECTIVE_NEAR, CAMERA_PROJECTION_PERSPECTIVE_FAR);
  
  cx_mat4x4 translation;
  cx_mat4x4_translation (&translation, 0.0f, 0.0f, -5.0f);
  
  cx_mat4x4 rotation;
  cx_mat4x4 rotx, roty;
  cx_mat4x4_rotation_axis_x (&rotx, cx_rad (s_rotationAngleX));
  cx_mat4x4_rotation_axis_y (&roty, cx_rad (s_rotationAngleY));
  cx_mat4x4_mul (&rotation, &rotx, &roty);

  cx_mat4x4 transformation;
  cx_mat4x4_mul (&transformation, &translation, &rotation);
  
  cx_mat4x4 mvpMatrix;
  cx_mat4x4_mul (&mvpMatrix, &proj, &transformation);
  
  cx_gdi_set_transform (CX_GDI_TRANSFORM_P, &proj);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MV, &transformation);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
#endif
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_earth (void)
{
  // update 2d render info: font opacity
  
  float opacity = app_get_zoom_opacity ();
  
  float screenWidth = cx_gdi_get_screen_width ();
  float screenHeight = cx_gdi_get_screen_height ();
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (s_camera, &proj);
  camera_get_view_matrix (s_camera, &view);
  
  // for each point, get 2d point
  float depth, scale;
  cx_vec2 screen;
  
  float zfar = CAMERA_PROJECTION_ORTHOGRAPHIC_FAR;
  float znear = CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR;
  
  int i, c;
  
  cx_vec4 look;
  cx_vec4_sub (&look, &s_camera->position, &s_camera->target); // use inverse direction vector for dot product calcuation
  cx_vec4_normalize (&look);
  
  for (i = 0, c = s_earth->data->count; i < c; ++i)
  {
    const cx_vec4 *pos = &s_earth->data->location [i];
    const cx_vec4 *nor = &s_earth->data->normal [i];
    
    cx_util_world_space_to_screen_space (screenWidth, screenHeight, &proj, &view, pos, &screen, &depth, &scale);
    
    float clipz = (2.0f * depth) - 1.0f;
    float z = ((clipz + (zfar + znear) / (zfar - znear)) * (zfar - znear)) / -2.0f;
    
    s_render2dInfo.renderPos [i].x = screen.x;
    s_render2dInfo.renderPos [i].y = screen.y;
    s_render2dInfo.renderPos [i].z = z;
    s_render2dInfo.renderPos [i].w = scale * 0.7f;
    
    float dotp = cx_max (0.0f, cx_vec4_dot (nor, &look));
    float alphaText = cx_smoothstep (0.95f, 1.0f, dotp);
    float alphaPoint = cx_smoothstep (0.80f, 1.0f, dotp);
    
    s_render2dInfo.opacity [i].x = opacity * alphaText;
    s_render2dInfo.opacity [i].y = opacity * alphaPoint;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds (void)
{
  app_update_feeds_news ();
  app_update_feeds_twitter ();
  app_update_feeds_weather ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_news (void)
{
  if (app_get_city_index_valid (s_selectedCity))
  {
    feed_news_t *feed = &s_newsFeeds [s_selectedCity];
    CX_ASSERT (feed);
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_SUCCESS:
      {
        ui_ctrlr_set_news_feed (feed);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        util_activity_indicator_set_active (false);
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_SERVER_ERROR);
        util_activity_indicator_set_active (false);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;

        break;
      }
        
      case FEED_REQ_STATUS_ERROR:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_CONNECTION_ERROR);
        util_activity_indicator_set_active (false);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        break;
      }
        
      case FEED_REQ_STATUS_IN_PROGRESS:
      {
        break;
      }
        
      default:
      {
        //CX_FATAL_ERROR ("FEED_REQ_STATUS_INVALID");
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_twitter (void)
{
  if (app_get_city_index_valid (s_selectedCity))
  {
    feed_twitter_t *feed = &s_twitterFeeds [s_selectedCity];
    CX_ASSERT (feed);
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_SUCCESS:
      {
        ui_ctrlr_set_twitter_feed (feed);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        util_activity_indicator_set_active (false);
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_SERVER_ERROR);
        util_activity_indicator_set_active (false);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        break;
      }
        
      case FEED_REQ_STATUS_ERROR:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_CONNECTION_ERROR);
        util_activity_indicator_set_active (false);
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        break;
      }
        
      case FEED_REQ_STATUS_IN_PROGRESS:
      {
        break;
      }
        
      default:
      {
        //CX_FATAL_ERROR ("FEED_REQ_STATUS_INVALID");
        break;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_weather (void)
{
  // go through each weather feed and trigger request
  
  if (s_refreshWeather)
  {    
    s_refreshWeather = false;
    
    // if not already updating
    
    if (!app_get_city_index_valid (s_currentWeatherCity))
    {
      s_currentWeatherCity = 0;
      util_activity_indicator_set_active (true);
    }
  }
  
#if 0
  static int success = 0;
#endif
  
  if (app_get_city_index_valid (s_currentWeatherCity)) // is updating
  {
    feed_weather_t *feed = &s_weatherFeeds [s_currentWeatherCity];
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_INVALID:
      {
        const char *wId = s_earth->data->weatherId [s_currentWeatherCity];
        feeds_weather_search (feed, wId);
        break;
      }
        
      case FEED_REQ_STATUS_SUCCESS:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        s_currentWeatherCity++;
#if 0
        success++;
#endif
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        s_currentWeatherCity++;
        break;
      }
        
      case FEED_REQ_STATUS_ERROR:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        s_currentWeatherCity++;
        util_status_bar_set_msg (STATUS_BAR_MSG_CONNECTION_ERROR);
        break;
      }
        
      case FEED_REQ_STATUS_IN_PROGRESS:
      default:
      {
        break;
      }
    }
    
    if (s_currentWeatherCity >= s_earth->data->count)
    {
      util_activity_indicator_set_active (false);
#if 0
      if (!success)
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_CONNECTION_ERROR);
      }
      
      success = 0;
#endif
    }
  }
#if 0
  else // is not updating
  {
    if (s_refreshWeather)
    {
      s_currentWeatherCity = 0;
      s_refreshWeather = false;
      util_activity_indicator_set_active (true);
    }
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_weather_refresh (void)
{
  s_refreshWeather = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_load (void)
{
  cx_gdi_unbind_all_buffers ();
  
  float screenWidth = cx_gdi_get_screen_width ();
  float screenHeight = cx_gdi_get_screen_height ();
  
  cx_mat4x4 proj, view;
  
  cx_mat4x4_ortho (&proj, 0.0f, screenWidth, 0.0f, screenHeight, CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR, CAMERA_PROJECTION_ORTHOGRAPHIC_FAR); 
  cx_mat4x4_identity (&view);
  
  cx_gdi_set_transform (CX_GDI_TRANSFORM_P, &proj);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MV, &view);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MVP, &proj);
  
  app_load_stage_update ();
  app_load_stage_render ();

  app_render_2d_screen_fade ();
  app_render_2d_logo ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_3d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_gdi_unbind_all_buffers ();
  
  //////////////
  // render
  //////////////
  
  app_render_3d_earth ();
  
  //////////////
  // end
  //////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_3d_earth (void)
{
  cx_date date;
  
  cx_time_set_date (&date, CX_TIME_ZONE_UTC);
  
  earth_render (s_earth, &date, &s_camera->position);

#if DEBUG_SHOW_TEMPERATURE
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  // draw points
  float opacity = app_get_zoom_opacity ();
  
  cx_colour col = *cx_colour_white ();
  col.a = opacity;
  
  // gather points
#if 1
  int displayCount = 0;
  cx_vec4 loc [256];
  
  CX_ASSERT (s_earth->data->count < 256);
  
  memset (loc, 0, sizeof (loc));
  
  for (int i = 0, c = s_earth->data->count; i < c; ++i)
  {
    bool display = settings_get_city_display (i);
    
    if (display && (s_selectedCity != i))
    {
      cx_vec4 *pos = &s_earth->data->location [i];
      
      loc [displayCount] = *pos;
      loc [displayCount++].a = s_render2dInfo.opacity [i].y;
    }
  }
  
  cx_draw_points (displayCount, loc, &col, glowtex);
  
#else
  cx_draw_points (s_earth->data->count, s_earth->data->location, &col, glowtex);
#endif
  
  if (app_get_city_index_valid (s_selectedCity))
  {
    cx_colour col = *cx_colour_yellow ();
    col.a = opacity;
    
    // pulse colour
    
    float time = (float) cx_system_time_get_total_time () * 10.0f;
    float t = (cx_sin (time) + 1.0f) * 0.5f;
    
    cx_vec4_mul (&col, t, &col);
    
    cx_vec4 pos = s_earth->data->location [s_selectedCity];
    pos.a = s_render2dInfo.opacity [s_selectedCity].y;
    cx_draw_points (1, &pos, &col, glowtex);
  }
  
  cx_gdi_enable_z_write (true);
#endif
  
#if 0
  static cx_line *normalLines = NULL;
  static cx_line *tangentLines = NULL;
  static cx_line *bitangentLines = NULL;
  
  cx_vertex_data *vertexData = s_earth->visual->mesh [0]->vertexData;
  
  if (!normalLines)
  {
    normalLines = (cx_line *) cx_malloc (sizeof (cx_line) * vertexData->numVertices);
    
    for (int i = 0; i < vertexData->numVertices; ++i)
    {
      cx_vec4 pos, nor, add;
      
      pos = vertexData->vertices [i].position;
      nor = vertexData->vertices [i].normal;
      cx_vec4_normalize (&nor);
      
      cx_vec4_mul (&nor, 0.1f, &nor);
      cx_vec4_add (&add, &pos, &nor);
      
      normalLines [i].start = pos;
      normalLines [i].end = add;
    }
  }
  
  if (!tangentLines)
  {
    tangentLines = (cx_line *) cx_malloc (sizeof (cx_line) * vertexData->numVertices);
    
    for (int i = 0; i < vertexData->numVertices; ++i)
    {
      cx_vec4 pos, tan, add;
      
      pos = vertexData->vertices [i].position;
      tan = vertexData->vertices [i].tangent;
      
      cx_vec4_mul (&tan, 0.1f, &tan);
      cx_vec4_add (&add, &pos, &tan);
      
      tangentLines [i].start = pos;
      tangentLines [i].end = add;
    }
  }
  
  if (!bitangentLines)
  {
    bitangentLines = (cx_line *) cx_malloc (sizeof (cx_line) * vertexData->numVertices);
    
    for (int i = 0; i < vertexData->numVertices; ++i)
    {
      cx_vec4 pos, tan, add;
      
      pos = vertexData->vertices [i].position;
      tan = vertexData->vertices [i].bitangent;
      
      cx_vec4_mul (&tan, 0.1f, &tan);
      cx_vec4_add (&add, &pos, &tan);
      
      bitangentLines [i].start = pos;
      bitangentLines [i].end = add;
    }
  }
  
  if (normalLines)
  {
    cx_draw_lines (vertexData->numVertices, normalLines, cx_colour_orange (), 1.0f);
  }
  
  if (tangentLines)
  {
    cx_draw_lines (vertexData->numVertices, tangentLines, cx_colour_blue (), 1.0f);
  }
  
  if (bitangentLines)
  {
    cx_draw_lines (vertexData->numVertices, bitangentLines, cx_colour_red (), 1.0f);
  }
#endif
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_gdi_unbind_all_buffers ();
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  // set 2d mvp matrix
  float screenWidth = cx_gdi_get_screen_width ();
  float screenHeight = cx_gdi_get_screen_height ();
  
  cx_mat4x4 proj, view;
  
  cx_mat4x4_ortho (&proj, 0.0f, screenWidth, 0.0f, screenHeight, CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR, CAMERA_PROJECTION_ORTHOGRAPHIC_FAR); 
  cx_mat4x4_identity (&view);
  
  cx_gdi_set_transform (CX_GDI_TRANSFORM_P, &proj);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MV, &view);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MVP, &proj);
  
  util_render_fps ();
  
  util_status_bar_render ();
  
  //////////////
  // render
  //////////////
  
  // render local clock 
  app_render_2d_local_clock ();
  
  // render earth icons
  app_render_2d_earth ();
  
  // render feeds
  app_render_2d_feeds ();
  
  // render browser
  browser_render (&s_browserRect, 1.0f);
  
  // render fade
  app_render_2d_screen_fade ();
  
  // intro logo
  app_render_2d_logo ();
  
  //////////////
  // end
  //////////////
  
  cx_gdi_enable_z_write (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_local_clock (void)
{
  cx_date date;
  cx_time_set_date (&date, CX_TIME_ZONE_LOCAL);

  int wday = date.calendar.tm_wday;
  int mday = date.calendar.tm_mday;
  int mon  = date.calendar.tm_mon;
  int hour = date.calendar.tm_hour;
  int min  = date.calendar.tm_min;
  
  CX_ASSERT ((wday >= 0) && (wday < 7));
  CX_ASSERT ((mon >= 0) && (mon < 12));
  
  static const char *wdayStr [7] = 
  {
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday",
  };
  
  static const char *monStr [12] =
  {
    "January",
    "February",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
  };
  
  char timeStr [32], dateStr [32];
  
  int displayFmt = settings_get_clock_display_format ();
  
  if (displayFmt == SETTINGS_CLOCK_DISPLAY_FORMAT_24HR)
  {
    cx_sprintf (timeStr, 32, "%02d:%02d", hour, min);
    cx_sprintf (dateStr, 32, "%s %d %s", wdayStr [wday], mday, monStr [mon]);
  }
  else 
  {
    const char *suffix = (hour > 11) ? "pm" : "am";
    hour = ((hour == 0) || (hour == 12)) ? 12 : hour % 12;
    
    cx_sprintf (timeStr, 32, "%d:%02d%s", hour, min, suffix);
    cx_sprintf (dateStr, 32, "%s %d %s", wdayStr [wday], mday, monStr [mon]);
  }
  
  const cx_font *font = util_get_font (FONT_SIZE_16);
  CX_ASSERT (font);
  
  float sw = cx_gdi_get_screen_width ();
  float tw = cx_font_get_text_width (font, timeStr);
  float dw = cx_font_get_text_width (font, dateStr);

  float tx = sw - tw - 12.0f;
  float ty = 2.0f;
  
  float dx = tx - dw - 6.0f;
  float dy = 2.0f;
  
  cx_font_render (font, dateStr, dx, dy, 0.0f, 0, cx_colour_grey ());
  cx_font_render (font, timeStr, tx, ty, 0.0f, 0, cx_colour_white ());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_feeds (void)
{
  ui_ctrlr_render ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_screen_fade (void)
{
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  util_screen_fade_render (deltaTime);
  
  cx_gdi_enable_z_write (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_logo (void)
{
  if (s_renderLogo && s_logoTex)
  {  
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    float opacity = 1.0f;
    
    if (util_anim_update (&s_logoFade, deltaTime))
    {  
      opacity = 1.0f - s_logoFade.t;
    }
    
    float screenWidth = cx_gdi_get_screen_width ();
    float screenHeight = cx_gdi_get_screen_height ();
    
    float tw = (float) s_logoTex->width;
    float th = (float) s_logoTex->height;
    float tx = 0.0f + (screenWidth - tw) * 0.5f;
    float ty = 0.0f + (screenHeight - th) * 0.5f;
    
    cx_colour col = *cx_colour_white ();
    col.a *= opacity;
    cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, s_logoTex);
    
    if (opacity <= CX_EPSILON)
    {
      // free texture
      cx_texture_destroy (s_logoTex);
      s_logoTex = NULL;
      s_renderLogo = false;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_logo_fade_out (void *data)
{
  util_anim_start (&s_logoFade, ANIM_TYPE_LINEAR, 1.0f, NULL, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_earth (void)
{
#if 1
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_enable_z_write (true);
  
  const char unit [2] = {'C', 'F'};
  int unitType = settings_get_temperature_unit ();
  char tempUnit [3] = { 176, unit [unitType], 0 };
  
  const cx_font *font = util_get_font (FONT_SIZE_14);
  const cx_font *font2 = util_get_font (FONT_SIZE_12);
  
  char text2 [64];
  
  for (int i = 0, c = s_earth->data->count; i < c; ++i)
  {
    if (settings_get_city_display (i))
    {
      const char *text = s_earth->data->names [i];
      const feed_weather_t *feed = &s_weatherFeeds [i];
      const cx_vec4 *pos = &s_render2dInfo.renderPos [i];
      float opacityText = s_render2dInfo.opacity [i].x;
      
      cx_colour colour = (i == s_selectedCity) ? *cx_colour_yellow () : *cx_colour_white ();
      colour.a = opacityText;
  
#if DEBUG_SHOW_TEMPERATURE
      if (feeds_weather_data_valid (feed))
      {
        // (icon width / 2) + margin offset 
        cx_font_render (font, text, pos->x + 24.0f + 18.0f, pos->y - 16.0f, pos->z, 0, &colour);
        
        // temperature
        
        int tempValue = 0;
        
        if (unitType == SETTINGS_TEMPERATURE_UNIT_C)
        {
          tempValue = feed->celsius;
        }
        else
        {
          tempValue = (cx_roundupInt ((float) feed->celsius * 1.8f)) + 32;
        }
        
        char temp [32];
        cx_sprintf (temp, 32, "%d", tempValue);
        cx_strcat (temp, 32, tempUnit);
        
        // time
        
        int hour = feed->timeInfo.hour;
        int min = feed->timeInfo.min;
        
        char time [32];
        int displayFmt = settings_get_clock_display_format ();
        
        if (displayFmt == SETTINGS_CLOCK_DISPLAY_FORMAT_24HR)
        {
          cx_sprintf (time, 32, "%02d:%02d", hour, min);
        }
        else 
        {
          const char *suffix = (hour > 11) ? "pm" : "am";
          hour = ((hour == 0) || (hour == 12)) ? 12 : hour % 12;
          cx_sprintf (time, 32, "%d:%02d%s", hour, min, suffix);  
        }
        
        cx_sprintf (text2, 64, "%s / %s", temp, time);
        
        cx_font_render (font2, text2, pos->x + 24.0f + 18.0f, pos->y - 4.0f, pos->z, 0, &colour);
        
        // render weather icon
        feeds_weather_render (feed, pos->x + 24.0f, pos->y, pos->z, opacityText);
      }
      else
      {
        // (icon width / 2) + margin offset 
        cx_font_render (font, text, pos->x + 12.0f, pos->y - 10.0f, pos->z, 0, &colour);
      }
#else  
      //cx_font_set_scale (s_render2dInfo.font, pos->w, pos->w);
      cx_font_render (font, text, pos->x, pos->y, pos->z, CX_FONT_ALIGNMENT_CENTRE_X, &colour);
      
      // render weather icon
      feeds_weather_render (feed, pos->x, pos->y - 6.0f, pos->z, opacityText);
#endif
    }
  }
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_enable_z_write (false);
  
#else
  float fov = s_camera->fov;
  float opacity = 1.0f - cx_smoothstep (55.0f, 56.0f, fov);
  
  float screenWidth = cx_gdi_get_screen_width ();
  float screenHeight = cx_gdi_get_screen_height ();
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (s_camera, &proj);
  camera_get_view_matrix (s_camera, &view);
  
  // for each point, get 2d point
  float depth, scale;
  cx_vec2 screen;
  
  float zfar = CAMERA_PROJECTION_ORTHOGRAPHIC_FAR;
  float znear = CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR;
  
  int i, c;
  
  cx_vec4 look;
  cx_vec4_sub (&look, &s_camera->position, &s_camera->target); // use inverse direction vector for dot product calcuation
  cx_vec4_normalize (&look);
  
  for (i = 0, c = s_earth->data->count; i < c; ++i)
  {
    const char *text = s_earth->data->names [i];
    const cx_vec4 *pos = &s_earth->data->location [i];
    const cx_vec4 *nor = &s_earth->data->normal [i];
    
    float dotp = cx_max (0.0f, cx_vec4_dot (nor, &look));
    float alpha = cx_smoothstep (0.95f, 1.0f, dotp);
    
    cx_colour colour = (i == s_selectedCity) ? *cx_colour_yellow () : *cx_colour_white ();
    colour.a = opacity * alpha;
    
    cx_util_world_space_to_screen_space (screenWidth, screenHeight, &proj, &view, pos, &screen, &depth, &scale);
    
    float clipz = (2.0f * depth) - 1.0f;
    float z = ((clipz + (zfar + znear) / (zfar - znear)) * (zfar - znear)) / -2.0f;
    
    cx_font_set_scale (s_font, scale, scale);
    cx_font_render (s_font, text, screen.x, screen.y, z, CX_FONT_ALIGNMENT_CENTRE_X, &colour);
    
    screen.y -= 6.0f;
    
    // render weather icon
    feed_weather_t *feed = &s_weatherFeeds [i];
    feeds_weather_render (feed, screen.x, screen.y, z, (opacity * alpha));
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_handle_touch_event (const input_touch_event *event)
{
  if (ui_ctrlr_settings_get_active ())
  {
    ui_ctrlr_settings_set_active (false);
  }
  else if (audio_music_picker_active ())
  {
  }
  else if (ui_ctrlr_handle_input (event))
  {
  }
  else
  {
    switch (event->type)
    {
      case INPUT_TOUCH_TYPE_BEGIN: { app_input_touch_began (event->point.x, event->point.y); break; }
      case INPUT_TOUCH_TYPE_END:   { app_input_touch_ended (event->point.x, event->point.y); break; }
      case INPUT_TOUCH_TYPE_MOVE:  { app_input_touch_moved (event->point.x, event->point.y, event->prevpoint.x, event->prevpoint.y); break; }
      default:                     { break; }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_handle_gesture_event (const input_gesture_event *event)
{
  if (ui_ctrlr_settings_get_active ())
  {
    ui_ctrlr_settings_set_active (false);
  }
  else if (audio_music_picker_active ())
  {
  }
  else
  {
    switch (event->type)
    {
      case INPUT_GESTURE_TYPE_PINCH: { app_input_zoom (event->data.pinch.factor); break; }
      default:                       { break; }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_touch_began (float x, float y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  float touchX = x * sw;
  float touchY = y * sh;
  
  if (browser_is_open ())
  {
    if (!browser_handle_input (&s_browserRect, BROWSER_INPUT_TOUCH_BEGIN, touchX, touchY))
    {
      bool closed = browser_close ();
      CX_REFERENCE_UNUSED_VARIABLE (closed);
    }
  }
  else
  {
    int oldSelectedCity = s_selectedCity;
    int newSelectedCity = app_input_touch_earth (touchX, touchY, sw, sh);
    
    if (app_get_city_index_valid (newSelectedCity))
    {
      if (newSelectedCity != oldSelectedCity)
      {
        feed_news_t *oldFeedNews = &s_newsFeeds [oldSelectedCity];
        feed_twitter_t *oldFeedTwitter = &s_twitterFeeds [oldSelectedCity];
        
        if (oldFeedNews->reqStatus == FEED_REQ_STATUS_IN_PROGRESS)
        {
          feeds_news_cancel_search (oldFeedNews);
          util_activity_indicator_set_active (false);
        }
        
        if (oldFeedTwitter->reqStatus == FEED_REQ_STATUS_IN_PROGRESS)
        {
          feeds_twitter_cancel_search (oldFeedTwitter);
          util_activity_indicator_set_active (false);
        }
        
        const char *query = s_earth->data->newsFeeds [newSelectedCity];
        
        feed_news_t *newFeedNews = &s_newsFeeds [newSelectedCity];
        feed_twitter_t *newFeedTwitter = &s_twitterFeeds [newSelectedCity];
        
        if (newFeedTwitter->reqStatus == FEED_REQ_STATUS_INVALID)
        {
          feeds_twitter_search (newFeedTwitter, query);
          util_activity_indicator_set_active (true);
        }
        
        if (newFeedNews->reqStatus == FEED_REQ_STATUS_INVALID)
        {
          feeds_news_search (newFeedNews, query);
          util_activity_indicator_set_active (true);
        }
      }
      
      s_selectedCity = newSelectedCity;
      
#if NEW_ROTATION
      
      float diffx = 0.5f - x;
      float diffy = 0.5f - y;
      
      CX_DEBUGLOG_CONSOLE (1, "diffx = %.4f, diffy = %.4f", diffx, diffy);
      
      float aspectRatio = sw / sh;
      
      float rotAddx = diffx * 90.0f * aspectRatio;
      float rotAddy = diffy * 90.0f;
      
      s_rotTarget = s_rotAngle;
      
      s_rotTarget.x += rotAddx;
      s_rotTarget.y += rotAddy;
      
      s_rotSpeedExp = CAMERA_ROTATION_SPEED_EXPONENT_CITY_SELECT;
      
#else
      float aspectRatio = sw / sh;
      
      s_rotTouchEnd.x = 0.5f * sw * 0.25f * aspectRatio;
      s_rotTouchEnd.y = 0.5f * sh * 0.25f;
      s_rotTouchBegin.x = x * sw * 0.25f;
      s_rotTouchBegin.y = y * sh * 0.25f;
#endif
      
      audio_soundfx_play (AUDIO_SOUNDFX_CLICK0);
    }
    else
    {
#if NEW_ROTATION

      s_rotTarget = s_rotAngle;
      
      CX_DEBUGLOG_CONSOLE (1, "BEGIN-AFEQRTEY4534545RGDFGQ34TQQ4545411");
      CX_DEBUGLOG_CONSOLE (1, "bx = %.4f, by = %.4f", x, y);
      
      s_rotSpeedExp = CAMERA_ROTATION_SPEED_EXPONENT_SWIPE;
      
#else
      s_rotTouchBegin.x = x * sw * 0.25f;
      s_rotTouchBegin.y = y * sh * 0.25f;
      s_rotTouchEnd.x = x * sw * 0.25f;
      s_rotTouchEnd.y = y * sh * 0.25f;
#endif
    }
  }
  
  /*
  s_rotTouchBegin.x = x * sw * 0.25f;
  s_rotTouchBegin.y = y * sh * 0.25f;
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
  */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_touch_moved (float x, float y, float prev_x, float prev_y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
#if NEW_ROTATION
  
  float diffx = x - prev_x;
  float diffy = y - prev_y;
  
  CX_DEBUGLOG_CONSOLE (1, "diffx = %.4f, diffy = %.4f", diffx, diffy);
  
  float aspectRatio = sw / sh;
  
  float rotAddx = diffx * 90.0f * aspectRatio;
  float rotAddy = diffy * 90.0f;
  
#if 1
  //s_rotTarget.x = s_rotAngle.x + rotAddx;
  //s_rotTarget.y = s_rotAngle.y + rotAddy;
  s_rotTarget.x += rotAddx;
  s_rotTarget.y += rotAddy;
#else
  s_rotAngle.x += rotAddx;
  s_rotAngle.y += rotAddy;
  
  s_rotAngle.x = fmodf (s_rotAngle.x, 360.0f);
  s_rotAngle.y = cx_clamp (s_rotAngle.y, -89.9f, 89.9f);
#endif
  
#else
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
#endif

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_touch_ended (float x, float y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  float touchX = x * sw;
  float touchY = y * sh;
  
  if (browser_is_open ())
  {
    browser_handle_input (&s_browserRect, BROWSER_INPUT_TOUCH_END, touchX, touchY);
  }
  
#if NEW_ROTATION
  
  CX_DEBUGLOG_CONSOLE (1, "ex = %.4f, ey = %.4f", x, y);
  CX_DEBUGLOG_CONSOLE (1, "END-AFEQRTEY4534545RGDFGQ34TQQ4545411");
  
#else
  //s_rotTouchEnd.x = x * sw * 0.25f;
  //s_rotTouchEnd.y = y * sh * 0.25f;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_zoom (float factor)
{
  float f = cx_clamp (factor, 0.5f, 1.5f);
  
  f = s_camera->fov + (1.0f - factor);
  
  s_camera->fov = cx_clamp (f, CAMERA_MIN_FOV, CAMERA_MAX_FOV);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int app_input_touch_earth (float screenX, float screenY, float screenWidth, float screenHeight)
{
  // do ray test
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (s_camera, &proj);
  camera_get_view_matrix (s_camera, &view);
  
  cx_vec2 screen = {{ screenX , screenY }};
  
  // get ray
  
  cx_vec4 rayOrigin, rayDir;
  
  cx_util_screen_space_to_world_space (screenWidth, screenHeight, &proj, &view, &screen, &rayDir, 1.0f, true);
  
  rayOrigin = s_camera->position;
  
  cx_vec4_normalize (&rayDir);
  
  // detect if ray and point normal are on a collision course
  // get closest distance between point and distance and check if less than bounding circle 
  
  cx_vec4 normal, position, toPos;
  
  int cityIndex = CITY_INDEX_INVALID;
  
  int i, c;
  for (i = 0, c = s_earth->data->count; i < c; ++i)
  {
    bool display = settings_get_city_display (i);
    
    if (display)
      //if (s_earth->data->display [i])
    {
#if CX_DEBUG
      const char *city = s_earth->data->names [i];
      CX_REFERENCE_UNUSED_VARIABLE (city);
#endif
    
      float opacityPoint = s_render2dInfo.opacity [i].y;
      
      if (opacityPoint > 0.1f)
      {
        normal = s_earth->data->normal [i];
        position = s_earth->data->location [i];
        
        float dotp = cx_vec4_dot (&normal, &rayDir);
        
        if (dotp < 0.0f)
        {
          cx_vec4 intersectpt, tmp;
          
          cx_vec4_sub (&tmp, &position, &rayOrigin);
          
          float t = (cx_vec4_dot (&normal, &tmp)) / dotp;
          
          cx_vec4_mul (&tmp, t, &rayDir);
          
          cx_vec4_add (&intersectpt, &rayOrigin, &tmp);
          
          // get distance (squared)
          
          cx_vec4_sub (&toPos, &position, &intersectpt);
        
          float distSqr = cx_vec4_length_sq (&toPos);
          
          if (distSqr <= (0.025f * 0.025f))
          {
            CX_DEBUGLOG_CONSOLE (1, "%s", city);
            
            cityIndex = i;
            
            break;
          } 
        }
      }
    }
  }

  return cityIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_on_background (void)
{
  // save settings
  settings_data_save ();
  
  util_activity_indicator_set_active (false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_on_foreground (void)
{
  // load settings?
  
  // trigger weather update
  s_refreshWeather = true;
  
  // refresh selected city
  if (app_get_city_index_valid (s_selectedCity))
  {
    const char *query = s_earth->data->newsFeeds [s_selectedCity];    
    feed_news_t *feedNews = &s_newsFeeds [s_selectedCity];
    feed_twitter_t *feedTwitter = &s_twitterFeeds [s_selectedCity];
    
    if (feedTwitter->reqStatus == FEED_REQ_STATUS_INVALID)
    {
      feeds_twitter_search (feedTwitter, query);
      util_activity_indicator_set_active (true);
    }
    
    if (feedNews->reqStatus == FEED_REQ_STATUS_INVALID)
    {
      feeds_news_search (feedNews, query);
      util_activity_indicator_set_active (true);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void)
{
  const int size = 8 * 1024;
  //cx_vec4 vectors [size];
  cx_mat4x4 matrices [size];
  cx_mat4x4 matrix; 
  cx_mat4x4_identity(&matrix);
  
  float mat16 [16];
  
  cx_timer timer;
  cx_time_start_timer (&timer);
  
  for (int i = 0; i < size; ++i)
  {
    float r = rand () / (RAND_MAX + 1.0f);
    memset (mat16, r, sizeof(mat16));
    
    //cx_vec4_set (&vectors [i], r, r * i, r * i * 2.0f, r * i * 0.5f);
    cx_mat4x4_set (&matrices [i], mat16);
  }
  
  for (int i = 0; i < size; ++i)
  {
    //cx_mat4x4_mul_vec4(&vectors [i], &matrix, &vectors [i]);
  }
  
  for (int i = 0; i < size; ++i)
  {
    cx_mat4x4_mul (&matrices [i], &matrix, &matrices [i]);
  }
  
  cx_time_stop_timer (&timer);
  
  printf ("app_test_code: %.3f\n", timer.elapsedTime);
  //CX_DEBUGLOG_CONSOLE (1, "app_test_code: %.3f", timer.elapsedTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
