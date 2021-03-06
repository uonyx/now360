//
//  app.c
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#include "app.h"
#include "util.h"
#include "camera.h"
#include "feeds.h"
#include "earth.h"
#include "worker.h"
#include "ui_ctrlr.h"
#include "audio.h"
#include "settings.h"
#include "webview.h"
#include "metrics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_PROJECTION_PERSPECTIVE_NEAR    (0.1f)
#define CAMERA_PROJECTION_PERSPECTIVE_FAR     (100.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR   (-1.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_FAR    (1.0f)
#define CAMERA_MIN_FOV                        (40.0f)
#define CAMERA_MAX_FOV                        (75.0f)
#define CAMERA_START_FOV                      (50.0f)
#define CLOCK_UPDATE_INTERVAL_SECONDS         (30.0f)
#define CITY_INDEX_INVALID                    (-1)

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

static camera_t       *g_camera = NULL;
static render2d_t      g_render2dInfo;

#define NEW_ROTATION 1

#if NEW_ROTATION
#define CAMERA_ROTATION_SPEED_EXPONENT_SWIPE (1.0f)
#define CAMERA_ROTATION_SPEED_EXPONENT_CITY_SELECT (4.0f)

static cx_vec2  g_rotAngle;
static cx_vec2  g_rotTarget;
static cxf32    g_rotSpeedExp = 1.0f;
#else
static cx_vec2  g_rotTouchBegin;
static cx_vec2  g_rotTouchEnd;
static cx_vec2  g_rotationSpeed;
static cx_vec2  g_rotationAngle;
#endif

static bool               g_isRetina = false;
static cxi64              g_prevTimeInBg = 0;
static cxi64              g_bgTime = 0;

static feed_twitter_t    *g_feedsTwitter = NULL;
static feed_news_t       *g_feedsNews = NULL;
static feed_weather_t    *g_feedsWeather = NULL;

static int                g_selectedCity = CITY_INDEX_INVALID;
static int                g_weatherUpdateCity = CITY_INDEX_INVALID;
static bool               g_weatherRefresh = false;
static cxi64              g_weatherUpdateTime = 0;

static app_state_t        g_appState = APP_STATE_INVALID;
static app_load_stage_t   g_loadStage = APP_LOAD_STAGE_1_BEGIN;
static cx_thread         *g_ldThread = NULL;
static cx_thread_monitor  g_ldThreadMonitor;
static bool               g_ldThreadInProgress = false;
static cx_texture        *g_ldImages [2];

static anim_t             g_logoFade;
static cx_texture        *g_logoTex = NULL;
static cx_texture        *g_glowTex = NULL;

static cx_date            g_dateUTC;
static cx_date            g_dateLocal;
static float              g_dateUpdateTimer = 0.0f;

static float              g_feedsOSDZoomOpacity = 1.0f;
static float              g_feedsOSDZoomTargetOpacity = 1.0f;
static float              g_feedsNewsOSDOpacity = 1.0f;
static float              g_feedsNewsOSDTargetOpacity = 1.0f;
static float              g_feedsTwitterOSDOpacity = 1.0f;
static float              g_feedsTwitterOSDTargetOpacity = 1.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int app_get_clock_str (const cx_date *date, int offset, char *dst, int dstSize);

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

static void app_load_init (void);
static void app_load_deinit (void);
static void app_load_stage_transition (void *userdata);
static void app_load_stage_update (void);
static void app_load_stage_render (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_clocks (void);
static void app_update_camera (void);
static void app_update_earth (void);
static void app_update_osd_opacity (void);
static void app_update_feeds (void);
static void app_update_feeds_news (void);
static void app_update_feeds_twitter (void);
static void app_update_feeds_weather (void);

static void app_render_3d (void);
static void app_render_3d_earth (void);
static void app_render_2d (void);
static void app_render_2d_earth (void);
static void app_render_2d_feeds (void);
static void app_render_2d_local_clock (void);
static void app_render_2d_screen_fade (void);
static void app_render_2d_logo (void);
static void app_render_2d_logo_fade_out (void *data);
static void app_render_2d_logo_fade_end (void *data);
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
  
  cx_time_set_date (&g_dateUTC, CX_TIME_ZONE_UTC);
  cx_time_set_date (&g_dateLocal, CX_TIME_ZONE_LOCAL);

  //
  // util
  //
  
  util_thread_init ();
  
  //
  // earth data
  //
  
  bool success = earth_init ("data/earth.json", &g_dateUTC);
  
  CX_FATAL_ASSERT (success); CX_REF_UNUSED (success);
  
  const char **cityNames = earth_data_get_names ();
  int cityCount = earth_data_get_count ();
  
  settings_set_city_names (cityNames, cityCount);
  
  g_glowTex = cx_texture_create_from_file ("data/images/earth/glowcircle.gb25-16.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  
  //
  // feeds
  //
  
  feeds_init ();
  
  g_feedsTwitter = cx_malloc (sizeof (feed_twitter_t) * cityCount);
  memset (g_feedsTwitter, 0, sizeof (feed_twitter_t) * cityCount);
  
  g_feedsNews = cx_malloc (sizeof (feed_news_t) * cityCount);
  memset (g_feedsNews, 0, sizeof (feed_news_t) * cityCount);
  
  g_feedsWeather = cx_malloc (sizeof (feed_weather_t) * cityCount);
  memset (g_feedsWeather, 0, sizeof (feed_weather_t) * cityCount);

  //
  // 2d render info
  //
  
  g_render2dInfo.renderPos = cx_malloc (sizeof (cx_vec4) * cityCount);
  memset (g_render2dInfo.renderPos, 0, (sizeof (cx_vec4) * cityCount));
  
  g_render2dInfo.opacity = cx_malloc (sizeof (cx_vec2) * cityCount);
  memset (g_render2dInfo.opacity, 0, (sizeof (cx_vec2) * cityCount));
  
  //
  // other
  //
  
#if NEW_ROTATION
  memset (&g_rotAngle, 0, sizeof (g_rotAngle));
  memset (&g_rotTarget, 0, sizeof (g_rotTarget));
#else
  memset (&g_rotTouchBegin, 0, sizeof (g_rotTouchBegin));
  memset (&g_rotTouchEnd, 0, sizeof (g_rotTouchEnd));
  memset (&g_rotationSpeed, 0, sizeof (g_rotationSpeed));
  memset (&g_rotationAngle, 0, sizeof (g_rotationAngle));
#endif
  
  cx_gdi_shared_context_destroy ();
  
  cx_thread_monitor_signal (&g_ldThreadMonitor);
  
  return CX_THREAD_EXIT_STATUS_SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_init (void *rootvc, void *gctx, int width, int height)
{ 
  CX_ASSERT (rootvc);
  
  //
  // engine
  //
  
  cx_engine_init_params params;
  memset (&params, 0, sizeof (params));
  
  params.graphics.screenWidth = width;
  params.graphics.screenHeight = height;
  params.graphics.context = gctx;
  
  params.network.httpCacheMemSizeMb = 2;
  params.network.httpCacheDiskSizeMb = 8;
#if CX_DEBUG
  params.network.httpCacheClear = false;
#else
  params.network.httpCacheClear = true;
#endif
  
  cx_engine_init (CX_ENGINE_INIT_ALL, &params);
  
  g_appState = APP_STATE_INIT;
  
  //
  // metrics
  //
  
  metrics_init ();
  
  metrics_event_log (METRICS_EVENT_LOAD_BEGIN, NULL);
  
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
  // webview
  //
  
  webview_init (rootvc);
  
  //
  // camera
  //
  
  g_camera = camera_create (width / height, CAMERA_START_FOV);
  
  //
  // loading screen
  //
  
  app_load_init ();
  
  //
  // loading thread
  //
  
  device_type_t devType = util_get_device_type ();
  
  g_isRetina = devType > DEVICE_TYPE_IPAD2;
  g_ldThreadInProgress = true;
  
  g_ldThread = cx_thread_create ("init_load", CX_THREAD_TYPE_JOINABLE, app_init_load, NULL);
  
  cx_thread_monitor_init (&g_ldThreadMonitor);
  
  cx_thread_start (g_ldThread);
  
  //
  // metrics
  //
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_deinit (void)
{
  feeds_deinit ();
  
  input_deinit ();
  
  util_deinit ();
  
  metrics_deinit ();
  
  cx_engine_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_view_resize (float width, float height)
{
  g_camera->aspectRatio = width / height;
  
  cx_gdi_set_screen_dimensions ((int) width, (int) height);
  
  ui_ctrlr_screen_resize (width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_update (void)
{
  cx_system_time_update ();

  switch (g_appState) 
  {
    case APP_STATE_INIT: 
    {
      if (cx_thread_monitor_wait_timed (&g_ldThreadMonitor, 0))
      {
        //
        // input
        //
        
        input_init ();
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_BEGIN, app_input_handle_touch_event);
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_END, app_input_handle_touch_event);
        input_register_touch_event_callback (INPUT_TOUCH_TYPE_MOVE, app_input_handle_touch_event);
        input_register_gesture_event_callback (INPUT_GESTURE_TYPE_PINCH, app_input_handle_gesture_event);
        
        metrics_event_log (METRICS_EVENT_LOAD_END, NULL);
        
        // wait for load state
       
        g_ldThreadInProgress = false;
      }
      
      break; 
    }
      
    case APP_STATE_UPDATE:
    {
      input_update ();
      
      app_update_clocks ();
      
      app_update_camera ();
      
      app_update_osd_opacity ();
      
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
  
  switch (g_appState) 
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
#if 0
static float app_get_zoom_opacity (void)
{
  float fov = g_camera->fov;
  
  float opacity = 1.0f - cx_smoothstep (CAMERA_START_FOV, CAMERA_START_FOV + 4.0f, fov);
  
  return opacity;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int app_get_clock_str (const cx_date *date, int offset, char *dst, int dstSize)
{
  CX_ASSERT (date);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize > 0);
  
  int dstLen = 0;
  
#if 0
  int offsetHr = offset / 100;
  int offsetMin = offset - (offsetHr * 100);
#else
  
  int offsetSecs = offset;
  int offsetMin = (offsetSecs / 60) % 60;
  int offsetHr = (offsetSecs / 3600);
  
#endif
  
  int hr = date->calendar.tm_hour;
  int mn = date->calendar.tm_min;
  
  int hour = (hr + offsetHr);
  int min = (mn + offsetMin);
  
  if (min > 60)
  {
    hour = hour + 1;
    min = min % 60;
  }
  else if (min < 0)
  {
    hour = hour - 1;
    min = 60 - min;
  }
  
  hour = (hour < 0) ? (hour + 24) : (hour % 24);
  
#if 1
  int sc = date->calendar.tm_sec;
  int tsecs = (hr * 3600) + (mn * 60) + sc;
  int secs = tsecs + offsetSecs;
  int nmin = (secs / 60) % 60;
  int nhr = (secs / 3600);
  
  nhr = (nhr < 0) ? (nhr + 24) : (nhr % 24);
  
  CX_REF_UNUSED (nmin);
  CX_REF_UNUSED (nhr);
#endif
  

  CX_ASSERT (hour >= 0);
  CX_ASSERT (min >= 0);
  
  int displayFmt = settings_get_clock_display_format ();
  
  if (displayFmt == SETTINGS_CLOCK_DISPLAY_FORMAT_24HR)
  {
    dstLen = cx_sprintf (dst, dstSize, "%02d:%02d", hour, min);
  }
  else
  {
    const char *suffix = (hour > 11) ? "pm" : "am";
    hour = ((hour == 0) || (hour == 12)) ? 12 : hour % 12;
    dstLen = cx_sprintf (dst, dstSize, "%d:%02d%s", hour, min, suffix);
  }
  
  return dstLen;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_init (void)
{
  cx_file_storage_base b = CX_FILE_STORAGE_BASE_RESOURCE;
  
  g_logoTex = cx_texture_create_from_file ("data/images/loading/now360-500px.png", b, false);
  g_ldImages [0] = cx_texture_create_from_file ("data/images/loading/uonyechi.com.png", b, false);
  g_ldImages [1] = cx_texture_create_from_file ("data/images/loading/credits-amble20vx.png", b, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_deinit (void)
{
  cx_texture_destroy (g_logoTex);
  g_logoTex = NULL;
  
  for (int i = 0; i < 2; ++i)
  {
    cx_texture_destroy (g_ldImages [i]);
    g_ldImages [i] = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_transition (void *userdata)
{
  CX_ASSERT (userdata);
  
  app_load_stage_t nextStage = (int) userdata;
  
  g_loadStage = nextStage;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_load_stage_update (void)
{
  const float deltaTime = (float) cx_system_time_get_delta_time ();
  
  switch (g_loadStage) 
  {
    case APP_LOAD_STAGE_1_BEGIN:
    {
      util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 0.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_1);
      break;
    }
      
    case APP_LOAD_STAGE_1:
    {
      static float timer = 1.0f;
      timer -= deltaTime;
      
      if (timer <= 0.0f)
      {
        g_loadStage = APP_LOAD_STAGE_1_END;
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
        g_loadStage = APP_LOAD_STAGE_2_END;
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
      if (util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 1.0f, app_load_stage_transition, (void *) APP_LOAD_STAGE_3))
      {
        util_activity_indicator_set_active (true);
      }
      
      break;
    }
      
    case APP_LOAD_STAGE_3:
    {
      if (!g_ldThreadInProgress)
      {
        g_loadStage = APP_LOAD_STAGE_3_END;
      }
      
      break;
    }
      
    case APP_LOAD_STAGE_3_END:
    {
      if (util_screen_fade_trigger (SCREEN_FADE_TYPE_IN, 1.0f, 3.0f, app_render_2d_logo_fade_out, NULL))
      {
        util_activity_indicator_set_active (false);
        
        g_appState = APP_STATE_UPDATE;
      }
      
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
  const float screenWidth = cx_gdi_get_screen_width ();
  const float screenHeight = cx_gdi_get_screen_height ();
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  switch (g_loadStage) 
  {
    case APP_LOAD_STAGE_1_BEGIN:
    case APP_LOAD_STAGE_1:
    case APP_LOAD_STAGE_1_END:
    {
      cx_texture *tex = g_ldImages [0]; //uonyechi.com
      CX_ASSERT (tex);
      
      float tw = (float) tex->width;
      float th = (float) tex->height;
      float tx = 0.0f + (screenWidth - tw) * 0.5f;
      float ty = 0.0f + (screenHeight - th) * 0.5f;
      
      cx_colour col = *cx_colour_white ();
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, tex);
      
      app_render_2d_screen_fade ();
      
      break;
    }
      
    case APP_LOAD_STAGE_2_BEGIN:
    case APP_LOAD_STAGE_2:
    case APP_LOAD_STAGE_2_END:
    {
      cx_texture *tex = g_ldImages [1]; // credits
      CX_ASSERT (tex);
      
      float tw = (float) tex->width;
      float th = (float) tex->height;
      float tx = 0.0f + (screenWidth - tw) * 0.5f;
      float ty = 0.0f + (screenHeight - th) * 0.5f;
      
      cx_colour col = *cx_colour_white ();
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, tex);
      
      app_render_2d_screen_fade ();
      
      break;
    }
      
    case APP_LOAD_STAGE_3_BEGIN:
    case APP_LOAD_STAGE_3:
    {      
      app_render_2d_logo ();
      
      app_render_2d_screen_fade ();
      
      break;
    }

    case APP_LOAD_STAGE_3_END:
    {
      app_render_2d_screen_fade ();
      
      app_render_2d_logo ();
      
      break;
    }
      
    default:
    {
      app_render_2d_screen_fade ();
      
      break;
    }
  }
  
  cx_gdi_enable_z_write (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_camera (void)
{
  float deltaTime = (float) cx_system_time_get_delta_time ();
  deltaTime = cx_min (deltaTime, (1.0f / 60.0f));
  
  float aspectRatio = cx_gdi_get_aspect_ratio ();
  CX_REF_UNUSED (aspectRatio);
  
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  CX_REF_UNUSED (sw);
  CX_REF_UNUSED (sh);
  
#if NEW_ROTATION

  float rotAddx = g_rotTarget.x - g_rotAngle.x;
  float rotAddy = g_rotTarget.y - g_rotAngle.y;

  float zoomFactor = 1.0f - cx_linearstep (CAMERA_MIN_FOV, CAMERA_MAX_FOV, g_camera->fov);
  zoomFactor += 1.0f;
  zoomFactor = cx_pow (zoomFactor, 3.0f);

  
  float rotDecel = 1.0f * zoomFactor * g_rotSpeedExp;
  float rotSpeed = 10.0f;
  
  float rotSpeedx = (fabsf (rotAddx) / rotDecel);
  float rotSpeedy = (fabsf (rotAddy) / rotDecel);
  
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
  
  g_rotAngle.x += (rotAddx * deltaTime * rotSpeedx);
  g_rotAngle.y += (rotAddy * deltaTime * rotSpeedy);
  
  float rotx = fmodf (g_rotAngle.x, 360.0f);
  float roty = cx_clamp (g_rotAngle.y, -89.9f, 89.9f);
  
#else // OLD_ROTATION
  
  cx_vec2 tdist;
  
  tdist.x = g_rotTouchEnd.x - g_rotTouchBegin.x;
  tdist.y = g_rotTouchEnd.y - g_rotTouchBegin.y;

  float rx = tdist.x * deltaTime;
  float ry = tdist.y * deltaTime;
  
  g_rotTouchBegin.x += rx;
  g_rotTouchBegin.y += ry;
  
  g_rotationAngle.x += rx;
  g_rotationAngle.y += ry;
  
  g_rotationAngle.x = fmodf (g_rotationAngle.x, 360.0f);
  g_rotationAngle.y = cx_clamp (g_rotationAngle.y, -89.9f, 89.9f);
  
  float rotx = g_rotationAngle.x;
  float roty = g_rotationAngle.y;
  
#endif // END_ROTATION
  
  cx_vec4_set (&g_camera->position, 0.0f, 0.0f, -2.0f, 1.0f);
  cx_vec4_set (&g_camera->target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update camera (view and projetion matrix)

  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  cx_vec4 axis_x = {{1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, 1.0f, 0.0f, 0.0f}};
  
  camera_rotate_around_point (g_camera, &center, cx_rad (roty), &axis_x);
  camera_rotate_around_point (g_camera, &center, cx_rad (-rotx), &axis_y);
  
  // get projection matrix
  cx_mat4x4 projmatrix;
  camera_get_projection_matrix (g_camera, &projmatrix);
  
  // get view matrix
  cx_mat4x4 viewmatrix;
  camera_get_view_matrix (g_camera, &viewmatrix);
  
  // compute modelviewprojection matrix
  cx_mat4x4 mvpMatrix;
  cx_mat4x4_mul (&mvpMatrix, &projmatrix, &viewmatrix);
  
  //camera_update (g_camera, deltaTime);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_P, &projmatrix);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MV, &viewmatrix);
  cx_gdi_set_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_clocks (void)
{
  float deltaTime = (float) cx_system_time_get_delta_time ();

  g_dateUpdateTimer -= deltaTime;
  
  if (g_dateUpdateTimer <= 0.0f)
  {
    cx_time_set_date (&g_dateUTC, CX_TIME_ZONE_UTC);
    cx_time_set_date (&g_dateLocal, CX_TIME_ZONE_LOCAL);
    
    g_dateUpdateTimer = CLOCK_UPDATE_INTERVAL_SECONDS;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_earth (void)
{
  // update 2d render info: font opacity
  
  float opacity = g_feedsOSDZoomOpacity;
  
  float screenWidth = cx_gdi_get_screen_width ();
  float screenHeight = cx_gdi_get_screen_height ();
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (g_camera, &proj);
  camera_get_view_matrix (g_camera, &view);
  
  // for each point, get 2d point
  float depth, scale;
  cx_vec2 screen;
  
  float zfar = CAMERA_PROJECTION_ORTHOGRAPHIC_FAR;
  float znear = CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR;
  
  int i, c;
  
  cx_vec4 look;
  cx_vec4_sub (&look, &g_camera->position, &g_camera->target); // use inverse direction vector for dot product calcuation
  cx_vec4_normalize (&look);
  
  for (i = 0, c = earth_data_get_count (); i < c; ++i)
  {
    const cx_vec4 *pos = earth_data_get_position (i);
    const cx_vec4 *nor = earth_data_get_normal (i);
    
    cx_vec4 spos;
    
    if (i == g_selectedCity)
    {
      cx_vec4 snor;
      cx_vec4_mul (&snor, 0.035f, nor);
      cx_vec4_add (&spos, pos, &snor);
    
      pos = &spos;
    }
    
    cx_util_world_space_to_screen_space (screenWidth, screenHeight, &proj, &view, pos, &screen, &depth, &scale);
    
    float clipz = (2.0f * depth) - 1.0f;
    float z = ((clipz + (zfar + znear) / (zfar - znear)) * (zfar - znear)) / -2.0f;
    
    g_render2dInfo.renderPos [i].x = screen.x;
    g_render2dInfo.renderPos [i].y = screen.y;
    g_render2dInfo.renderPos [i].z = z;
    g_render2dInfo.renderPos [i].w = scale * 0.7f;
    
    float dotp = cx_max (0.0f, cx_vec4_dot (nor, &look));
    float alphaText = cx_smoothstep (0.95f, 1.0f, dotp);
    float alphaPoint = cx_smoothstep (0.80f, 1.0f, dotp);
    
    g_render2dInfo.opacity [i].x = (i == g_selectedCity) ? (opacity * alphaPoint) : (opacity * alphaText);
    g_render2dInfo.opacity [i].y = opacity * alphaPoint;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds (void)
{
  // set osd opacity
  
  ui_ctrlr_set_news_feed_opacity (g_feedsOSDZoomOpacity * g_feedsNewsOSDOpacity);
  ui_ctrlr_set_twitter_feed_opacity (g_feedsOSDZoomOpacity * g_feedsTwitterOSDOpacity);
  
  // update feeds
  
  app_update_feeds_news ();
  app_update_feeds_twitter ();
  app_update_feeds_weather ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_osd_opacity (void)
{
  float dt = (float) cx_system_time_get_delta_time ();
  
  // zoom opacity
  
  g_feedsOSDZoomOpacity += ((g_feedsOSDZoomTargetOpacity - g_feedsOSDZoomOpacity) * dt * 7.0f);
  g_feedsOSDZoomOpacity = cx_clamp (g_feedsOSDZoomOpacity, 0.0f, 1.0f);
  
  // news opacity
  
  g_feedsNewsOSDOpacity += ((g_feedsNewsOSDTargetOpacity - g_feedsNewsOSDOpacity) * dt * 10.0f);
  g_feedsNewsOSDOpacity = cx_clamp (g_feedsNewsOSDOpacity, 0.0f, 1.0f);
  
  // twitter opacity
  
  g_feedsTwitterOSDOpacity += ((g_feedsTwitterOSDTargetOpacity - g_feedsTwitterOSDOpacity) * dt * 10.0f);
  g_feedsTwitterOSDOpacity = cx_clamp (g_feedsTwitterOSDOpacity, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_news (void)
{
  if (earth_data_validate_index (g_selectedCity))
  {
    feed_news_t *feed = &g_feedsNews [g_selectedCity];
    CX_ASSERT (feed);
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_SUCCESS:
      {
        util_activity_indicator_set_active (false);
        ui_ctrlr_set_news_feed (feed);
        g_feedsNewsOSDTargetOpacity = 1.0f;
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_NEWS_COMMS_ERROR);
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
        //CX_ERROR ("FEED_REQ_STATUS_INVALID");
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
  if (earth_data_validate_index (g_selectedCity))
  {
    feed_twitter_t *feed = &g_feedsTwitter [g_selectedCity];
    CX_ASSERT (feed);
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_SUCCESS:
      {
        util_activity_indicator_set_active (false);
        ui_ctrlr_set_twitter_feed (feed);
        g_feedsTwitterOSDTargetOpacity = 1.0f;
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        util_status_bar_set_msg (STATUS_BAR_MSG_TWITTER_COMMS_ERROR);
        util_activity_indicator_set_active (false);
        ui_ctrlr_set_twitter_feed (feed);
        g_feedsTwitterOSDTargetOpacity = 1.0f;
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
        //CX_ERROR ("FEED_REQ_STATUS_INVALID");
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
  
  if (g_weatherRefresh)
  {    
    g_weatherRefresh = false;
    
    if (!earth_data_validate_index (g_weatherUpdateCity)) // is not updating (dogdy eh?)
    {
      g_weatherUpdateCity = 0;
      g_weatherUpdateTime = cx_time_get_utc_epoch ();
      util_activity_indicator_set_active (true);
    }
  }
  else
  {
    if (!earth_data_validate_index (g_weatherUpdateCity)) // is not updating
    {
      cxi64 currentTime = cx_time_get_utc_epoch ();
      cxi64 timeElapsed = currentTime - g_weatherUpdateTime;
      cxi64 timeRefreshSeconds = 60 * 5;
      
      if (timeElapsed > timeRefreshSeconds)
      {
        g_weatherRefresh = true;
      }
    }
  }
  
#if 0
  static int success = 0;
#endif
  
  if (earth_data_validate_index (g_weatherUpdateCity)) // is updating
  {
    feed_weather_t *feed = &g_feedsWeather [g_weatherUpdateCity];
    
    const char *wId = earth_data_get_weather (g_weatherUpdateCity);
    
    switch (feed->reqStatus) 
    {
      case FEED_REQ_STATUS_INVALID:
      {
        feeds_weather_search (feed, wId);
        break;
      }
        
      case FEED_REQ_STATUS_SUCCESS:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        g_weatherUpdateCity++;
#if 0
        success++;
#endif
        break;
      }
        
      case FEED_REQ_STATUS_FAILURE:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        g_weatherUpdateCity++;
        util_status_bar_set_msg (STATUS_BAR_MSG_WEATHER_COMMS_ERROR);
        break;
      }
        
      case FEED_REQ_STATUS_ERROR:
      {
        feed->reqStatus = FEED_REQ_STATUS_INVALID;
        g_weatherUpdateCity++;
        util_status_bar_set_msg (STATUS_BAR_MSG_CONNECTION_ERROR);
        break;
      }
        
      case FEED_REQ_STATUS_IN_PROGRESS:
      default:
      {
        break;
      }
    }
    
    int cityCount = earth_data_get_count ();
    
    if (g_weatherUpdateCity >= cityCount)
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
    if (g_weatherRefresh)
    {
      g_weatherUpdateCity = 0;
      g_weatherRefresh = false;
      util_activity_indicator_set_active (true);
    }
  }
#endif
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
  // earth
  earth_render (&g_camera->position, &g_dateUTC);

  // points
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (false);
  
  // draw points
  float opacity = g_feedsOSDZoomOpacity;
  
  // gather points
  int displayCount = 0;
  int cityCount = earth_data_get_count ();
  
  cx_vec4 loc [cityCount];
  cx_colour col [cityCount];
  
  memset (loc, 0, sizeof (loc));
  memset (col, 0, sizeof (col));
  
  for (int i = 0; i < cityCount; ++i)
  {
    bool display = settings_get_city_display (i);
    
    if (display)
    {
      cx_vec4 pos = *earth_data_get_position (i);
      float a = g_render2dInfo.opacity [i].y;
      
      if (i == g_selectedCity)
      {
        cx_colour ycol;
        cx_colour_set (&ycol, 1.0f, 1.0f, 0.2f, opacity);
        
        const float pulseSpeed = 15.0f;
        float time = (float) cx_system_time_get_total_time () * pulseSpeed;
        float t = (cx_sin (time) + 1.0f) * 0.5f;
        ycol.a *= (t * a);
        
        col [displayCount] = ycol;
        
        const cx_vec4 *p = earth_data_get_position (g_selectedCity);
        cx_vec4 n = *earth_data_get_normal (g_selectedCity);
        cx_vec4_mul (&n, 0.035f, &n);
        cx_vec4_add (&pos, p, &n);
      }
      else
      {     
        cx_colour wcol = *cx_colour_white ();
        wcol.a = (opacity * a);
        
        col [displayCount] = wcol;
      }
      
      loc [displayCount++] = pos;
    }
  }
  
  // draw points
  static bool hackReadyDraw = false; // haven't quite figured the bug that has given birth to this hack

  if (hackReadyDraw)
  {
    cx_draw_points (displayCount, loc, col, g_glowTex, g_isRetina ? 3.0f : 2.0f);
  }
  else
  {
    hackReadyDraw = true;
  }

  cx_gdi_enable_z_write (true);
  
  // debug
#if (CX_DEBUG && 0)
  static cx_line *normalLines = NULL;
  static cx_line *tangentLines = NULL;
  static cx_line *bitangentLines = NULL;
  
  cx_vertex_data *vertexData = earth_data_get_mesh_vertex_data (0);
  
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
  
  //////////////
  // render
  //////////////
  
  // render local clock 
  app_render_2d_local_clock ();
  
  // render earth icons
  app_render_2d_earth ();
  
  // render feeds
  app_render_2d_feeds ();
  
  // render fade
  app_render_2d_screen_fade ();
  
  // status
  util_status_bar_render ();
  
  // logo
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
  
  const cx_date *date = &g_dateLocal;
  
  int wday = date->calendar.tm_wday;
  int mday = date->calendar.tm_mday;
  int mon  = date->calendar.tm_mon;
  
  CX_ASSERT ((wday >= 0) && (wday < 7));
  CX_ASSERT ((mon >= 0) && (mon < 12));
  
  app_get_clock_str (date, 0, timeStr, 32);
  cx_sprintf (dateStr, 32, "%s %d %s", wdayStr [wday], mday, monStr [mon]);
  
#if 1
  const cx_font *font = util_get_font (FONT_ID_DEFAULT_16);
  CX_ASSERT (font);
  
  float sw = cx_gdi_get_screen_width ();
  float tw = cx_font_get_text_width (font, timeStr);
  float dw = cx_font_get_text_width (font, dateStr);
  
  float tx = sw - tw - 12.0f;
  float ty = 2.0f;
  
  float dx = tx - dw - 6.0f;
  float dy = 2.0f;
  
  cx_colour grey;
  cx_colour_set (&grey, 0.65f, 0.65f, 0.65f, 1.0f);
  cx_font_render (font, dateStr, dx, dy, 0.0f, 0, &grey);
  cx_font_render (font, timeStr, tx, ty, 0.0f, 0, cx_colour_white ());

#else
  const cx_font *font = util_get_font (FONT_ID_NEWS_18);
  CX_ASSERT (font);
  
  cx_font_set_scale (font, 15.0f/18.0f, 15.0f/18.0f);
  
  float sw = cx_gdi_get_screen_width ();
  float tw = cx_font_get_text_width (font, timeStr);
  float dw = cx_font_get_text_width (font, dateStr);

  float tx = sw - tw - 12.0f;
  float ty = 2.0f;
  
  float dx = tx - dw - 6.0f;
  float dy = 2.0f;
  
  cx_colour grey;
  cx_colour_set (&grey, 0.65f, 0.65f, 0.65f, 1.0f);
  cx_font_render (font, dateStr, dx, dy, 0.0f, 0, &grey);
  cx_font_render (font, timeStr, tx, ty, 0.0f, 0, cx_colour_white ());
  
  cx_font_set_scale (font, 1.0f, 1.0f);
#endif
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
  if (g_logoTex) // deleted after loading is complete
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    float opacity = 1.0f;
    
    if (util_anim_update (&g_logoFade, deltaTime))
    {  
      opacity = 1.0f - g_logoFade.t;
    }
    
    if (g_logoTex) // may be deleted after loading is complete in callback in util_anim_update *nasty*
    {
      float screenWidth = cx_gdi_get_screen_width ();
      float screenHeight = cx_gdi_get_screen_height ();
      
      float tw = (float) g_logoTex->width;
      float th = (float) g_logoTex->height;
      float tx = 0.0f + (screenWidth - tw) * 0.5f;
      float ty = 0.0f + (screenHeight - th) * 0.5f;
      
      cx_colour col = *cx_colour_white ();
      col.a *= opacity;
      cx_draw_quad (tx, ty, (tx + tw), (ty + th), 0.0f, 0.0f, &col, g_logoTex);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_logo_fade_out (void *data)
{
  util_anim_start (&g_logoFade, ANIM_TYPE_LINEAR, 1.0f, app_render_2d_logo_fade_end, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_logo_fade_end (void *data)
{
  app_load_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_DRAW_SELECTED_LAST 0

static void app_render_2d_earth (void)
{
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_enable_z_write (true);
  
  int unitType = settings_get_temperature_unit ();
  char tempUnit [4];
  
  tempUnit [0] = 226; // 0xe2
  tempUnit [1] = 132; // 0x84
  tempUnit [2] = (unitType == SETTINGS_TEMPERATURE_UNIT_C) ? 131 : 137; // 0x83 : 0x89
  tempUnit [3] = 0;
  
  const cx_date *date = &g_dateUTC;
  
  const cx_font *font0 = util_get_font (FONT_ID_DEFAULT_16);
  const cx_font *font1 = util_get_font (FONT_ID_DEFAULT_14);
  const cx_font *font2 = util_get_font (FONT_ID_DEFAULT_12);
  
  char tempStr [32];
  char timeStr [32];
  char text2 [64];
  
  for (int i = 0, c = earth_data_get_count (); i < c; ++i)
  {
#if DEBUG_DRAW_SELECTED_LAST
    if (settings_get_city_display (i) && (i != g_selectedCity))
#else
    if (settings_get_city_display (i))
#endif
    {
      const char *cityStr = earth_data_get_city (i);
      int tzOffset = earth_data_get_tz_offset (i);
      
      const feed_weather_t *feed = &g_feedsWeather [i];
      const cx_vec4 *pos = &g_render2dInfo.renderPos [i];
      float opacityText = g_render2dInfo.opacity [i].x;
      
      app_get_clock_str (date, tzOffset, timeStr, 32);
      
#if DEBUG_DRAW_SELECTED_LAST
      cx_colour colour;
      cx_colour_set (&colour, 0.9f, 0.9f, 0.9f, opacityText * 0.95f);
      
      const cx_font *lfont = font1;
      const cx_font *sfont = font2;
      
      CX_ASSERT (lfont);
      CX_ASSERT (sfont);
#else
      cx_colour colour;
      const cx_font *lfont = NULL;
      const cx_font *sfont = NULL;
      
      if (i == g_selectedCity)
      {
        lfont = font0;
        sfont = font1;
        
        cx_colour_set (&colour, 1.0f, 1.0f, 1.0f, opacityText);
      }
      else
      {
        lfont = font1;
        sfont = font2;
        
        cx_colour_set (&colour, 0.9f, 0.9f, 0.9f, opacityText * 0.95f);
      }
#endif
      
      if (feed->dataReady)
      {
        int tempFarenHeit = (cx_util_roundup_int ((float) feed->celsius * 1.8f)) + 32;
        int tempValue = (unitType == SETTINGS_TEMPERATURE_UNIT_C) ? feed->celsius : tempFarenHeit;
        
        cx_sprintf (tempStr, 32, "%d", tempValue);
        cx_strcat (tempStr, 32, tempUnit);
        cx_sprintf (text2, 64, "%s / %s", tempStr, timeStr);
        
        if (feeds_weather_data_cc_valid (feed))
        {
          //////////////////
          // city 
          //////////////////
          
          // (icon width / 2) + margin offset
          cx_font_render (lfont, cityStr, pos->x + 42.0f, pos->y - 16.0f, pos->z, 0, &colour); // 42.0f = 24.0f + 18.0f
          
          ////////////////////////
          // temperature / time
          ///////////////////////
          
          cx_font_render (sfont, text2, pos->x + 42.0f, pos->y - 4.0f, pos->z, 0, &colour);
          
          //////////////////
          // weather icon
          //////////////////
          
          feeds_weather_render (feed, pos->x + 24.0f, pos->y, pos->z, opacityText);
        }
        else
        {
          //////////////////
          // city
          //////////////////
          
          cx_font_render (lfont, cityStr, pos->x + 12.0f, pos->y - 14.0f, pos->z, 0, &colour);
          
          ////////////////////////
          // temperature / time
          ///////////////////////
          
          cx_font_render (sfont, text2, pos->x + 12.0f, pos->y - 2.0f, pos->z, 0, &colour);
        }
      }
      else
      {
        //////////////////
        // city
        //////////////////
        
        cx_font_render (lfont, cityStr, pos->x + 12.0f, pos->y - 14.0f, pos->z, 0, &colour);
        
        //////////////////
        // time
        //////////////////
        
        cx_font_render (sfont, timeStr, pos->x + 12.0f, pos->y - 2.0f, pos->z, 0, &colour);
      }
    }
  }
  
#if DEBUG_DRAW_SELECTED_LAST
  if (earth_data_validate_index (g_selectedCity))
  {
    int i = g_selectedCity;
    CX_ASSERT (settings_get_city_display (i));
    
    const char *cityStr = earth_data_get_city (i);
    int tzOffset = earth_data_get_tz_offset (i);
    
    const feed_weather_t *feed = &g_feedsWeather [i];
    const cx_vec4 *pos = &g_render2dInfo.renderPos [i];
    float opacityText = g_render2dInfo.opacity [i].x;
    
    app_get_clock_str (date, tzOffset, timeStr, 32);
    
    //cx_colour colour = (i == g_selectedCity) ? *cx_colour_yellow () : *cx_colour_white ();
    //colour.a = opacityText;
    
    cx_colour colour;
    const cx_font *lfont = font0;
    const cx_font *sfont = font1;
    cx_colour_set (&colour, 1.0f, 1.0f, 1.0f, opacityText);
    
    CX_ASSERT (lfont);
    CX_ASSERT (sfont);
    
    if (feed->dataReady)
    {
      int tempFarenHeit = (cx_roundupInt ((float) feed->celsius * 1.8f)) + 32;
      int tempValue = (unitType == SETTINGS_TEMPERATURE_UNIT_C) ? feed->celsius : tempFarenHeit;
      
      cx_sprintf (tempStr, 32, "%d", tempValue);
      cx_strcat (tempStr, 32, tempUnit);
      cx_sprintf (text2, 64, "%s / %s", tempStr, timeStr);
      
      if (feeds_weather_data_cc_valid (feed))
      {
        //////////////////
        // city
        //////////////////
        
        // (icon width / 2) + margin offset
        cx_font_render (lfont, cityStr, pos->x + 24.0f + 18.0f, pos->y - 16.0f, pos->z, 0, &colour);
        
        ////////////////////////
        // temperature / time
        ///////////////////////
        
        cx_font_render (sfont, text2, pos->x + 24.0f + 18.0f, pos->y - 4.0f, pos->z, 0, &colour);
        
        //////////////////
        // weather icon
        //////////////////
        
        feeds_weather_render (feed, pos->x + 24.0f, pos->y, pos->z, opacityText);
      }
      else
      {
        //////////////////
        // city
        //////////////////
        
        cx_font_render (lfont, cityStr, pos->x + 12.0f, pos->y - 14.0f, pos->z, 0, &colour);
        
        ////////////////////////
        // temperature / time
        ///////////////////////
        
        cx_font_render (sfont, text2, pos->x + 12.0f, pos->y - 2.0f, pos->z, 0, &colour);
      }
    }
    else
    {
      //////////////////
      // city
      //////////////////
      
      cx_font_render (lfont, cityStr, pos->x + 12.0f, pos->y - 14.0f, pos->z, 0, &colour);
      
      //////////////////
      // time
      //////////////////
      
      cx_font_render (sfont, timeStr, pos->x + 12.0f, pos->y - 2.0f, pos->z, 0, &colour);
    }
  }
#endif
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_enable_z_write (false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_handle_touch_event (const input_touch_event *event)
{
  if (settings_ui_active ())
  {
    ui_ctrlr_settings_set_active (false);
  }
  else if (webview_active ())
  {
    //webview_hide ();
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

  int oldSelectedCity = g_selectedCity;
  int newSelectedCity = app_input_touch_earth (touchX, touchY, sw, sh);
  
  if (earth_data_validate_index (newSelectedCity))
  {
    if (newSelectedCity != oldSelectedCity)
    {
      if (earth_data_validate_index (oldSelectedCity))
      {
        feed_news_t *oldFeedNews = &g_feedsNews [oldSelectedCity];
        feed_twitter_t *oldFeedTwitter = &g_feedsTwitter [oldSelectedCity];
        
        if (oldFeedNews->reqStatus == FEED_REQ_STATUS_IN_PROGRESS)
        {
          feeds_news_cancel_search (oldFeedNews);
          util_activity_indicator_set_active (false);
        }
        
        if (oldFeedTwitter->reqStatus == FEED_REQ_STATUS_IN_PROGRESS)
        {
          //feeds_twitter_cancel_search (oldFeedTwitter);
          //util_activity_indicator_set_active (false);
        }
      }
      
      const char *query = earth_data_get_feed_query (newSelectedCity);
      
      feed_news_t *newFeedNews = &g_feedsNews [newSelectedCity];
      feed_twitter_t *newFeedTwitter = &g_feedsTwitter [newSelectedCity];
      
      if (newFeedTwitter->reqStatus == FEED_REQ_STATUS_INVALID)
      {
        float lat, lon;
        earth_data_get_terrestrial_coords (newSelectedCity, &lat, &lon);
        bool loc = settings_get_local_tweets_only ();
        
        feeds_twitter_search (newFeedTwitter, query, loc, lat, lon);
        
        util_activity_indicator_set_active (true);
        g_feedsTwitterOSDTargetOpacity = 0.0f;
      }
      
      if (newFeedNews->reqStatus == FEED_REQ_STATUS_INVALID)
      {
        feeds_news_search (newFeedNews, query);
        
        util_activity_indicator_set_active (true);
        g_feedsNewsOSDTargetOpacity = 0.0f;
      }
    }
    
    if (g_selectedCity == CITY_INDEX_INVALID)
    {
      const char *city = earth_data_get_city (newSelectedCity);
      metrics_event_log (METRICS_EVENT_FIRST_CITY, (void *) city);
    }
    
    g_selectedCity = newSelectedCity;
    
#if NEW_ROTATION
      
    float diffx = 0.5f - x;
    float diffy = 0.5f - y;
    
    CX_LOG_CONSOLE (0, "diffx = %.4f, diffy = %.4f", diffx, diffy);
    
    float aspectRatio = sw / sh;
    
    float rotAddx = diffx * 90.0f * aspectRatio;
    float rotAddy = diffy * 90.0f;
    
    g_rotTarget = g_rotAngle;
    
    g_rotTarget.x += rotAddx;
    g_rotTarget.y += rotAddy;
    
    g_rotSpeedExp = CAMERA_ROTATION_SPEED_EXPONENT_CITY_SELECT;
    
#else
    float aspectRatio = sw / sh;
    
    g_rotTouchEnd.x = 0.5f * sw * 0.25f * aspectRatio;
    g_rotTouchEnd.y = 0.5f * sh * 0.25f;
    g_rotTouchBegin.x = x * sw * 0.25f;
    g_rotTouchBegin.y = y * sh * 0.25f;
#endif
      
    audio_soundfx_play (AUDIO_SOUNDFX_CLICK0);
  }
  else
  {
#if NEW_ROTATION

    g_rotTarget = g_rotAngle;
    
    CX_LOG_CONSOLE (0, "BEGIN-AFEQRTEY4534545RGDFGQ34TQQ4545411");
    CX_LOG_CONSOLE (0, "bx = %.4f, by = %.4f", x, y);
    
    g_rotSpeedExp = CAMERA_ROTATION_SPEED_EXPONENT_SWIPE;
    
#else
    g_rotTouchBegin.x = x * sw * 0.25f;
    g_rotTouchBegin.y = y * sh * 0.25f;
    g_rotTouchEnd.x = x * sw * 0.25f;
    g_rotTouchEnd.y = y * sh * 0.25f;
#endif
  }
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
  
  CX_LOG_CONSOLE (0, "diffx = %.4f, diffy = %.4f", diffx, diffy);
  
  float aspectRatio = sw / sh;
  
  float rotAddx = diffx * 90.0f * aspectRatio;
  float rotAddy = diffy * 90.0f;
  
#if 1
  g_rotTarget.x += rotAddx;
  g_rotTarget.y += rotAddy;
#else
  g_rotAngle.x += rotAddx;
  g_rotAngle.y += rotAddy;
  
  g_rotAngle.x = fmodf (g_rotAngle.x, 360.0f);
  g_rotAngle.y = cx_clamp (g_rotAngle.y, -89.9f, 89.9f);
#endif
  
#else
  g_rotTouchEnd.x = x * sw * 0.25f;
  g_rotTouchEnd.y = y * sh * 0.25f;
#endif

}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_touch_ended (float x, float y)
{
#if NEW_ROTATION
  
#else
  //g_rotTouchEnd.x = x * sw * 0.25f;
  //g_rotTouchEnd.y = y * sh * 0.25f;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_zoom (float factor)
{
  //float f = cx_clamp (factor, 0.5f, 1.5f);
  
  float f = g_camera->fov + (1.0f - factor);
  
  g_camera->fov = cx_clamp (f, CAMERA_MIN_FOV, CAMERA_MAX_FOV);
  
  float fovMid = (CAMERA_MAX_FOV - CAMERA_MIN_FOV) * 0.55f; // 0.5f;
  float fovCutoff = CAMERA_MIN_FOV + fovMid;
  g_feedsOSDZoomTargetOpacity = (g_camera->fov <= fovCutoff) ? 1.0f : 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int app_input_touch_earth (float screenX, float screenY, float screenWidth, float screenHeight)
{
  // do ray test
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (g_camera, &proj);
  camera_get_view_matrix (g_camera, &view);
  
  cx_vec2 screen = {{ screenX , screenY }};
  
  // get ray
  
  cx_vec4 rayOrigin, rayDir;
  
  cx_util_screen_space_to_world_space (screenWidth, screenHeight, &proj, &view, &screen, &rayDir, 1.0f, true);
  
  rayOrigin = g_camera->position;
  
  cx_vec4_normalize (&rayDir);
  
  // detect if ray and point normal are on a collision course
  // get closest distance between point and distance and check if less than bounding circle 
  
  cx_vec4 normal, position, toPos;
  
  int cityIndex = CITY_INDEX_INVALID;
  
  int i, c;
  for (i = 0, c = earth_data_get_count (); i < c; ++i)
  {
    bool display = settings_get_city_display (i);
    
    if (display)
    {
#if CX_DEBUG
      const char *city = earth_data_get_city (i);
      CX_REF_UNUSED (city);
#endif
    
      float opacityPoint = g_render2dInfo.opacity [i].y;
      
      if (opacityPoint > 0.1f)
      {
        normal = *earth_data_get_normal (i);
        position = *earth_data_get_position (i);
        
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
        
          float touchRadius = 0.025f;
          float distSqr = cx_vec4_length_sq (&toPos);
          
          if (distSqr <= (touchRadius * touchRadius))
          {
            CX_LOG_CONSOLE (0, "%s", city);
            
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
  if (g_appState == APP_STATE_UPDATE)
  {
#if 0
    if (audio_music_playing ())
    {
      audio_music_pause ();
    }
#endif
    // save settings
    //settings_data_save (); // redundant
    
    util_activity_indicator_set_active (false); // necessary?
  
    metrics_event_log (METRICS_EVENT_APP_BG, NULL);
    
    g_bgTime = cx_time_get_utc_epoch ();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_on_foreground (void)
{
  if (g_appState == APP_STATE_UPDATE)
  {
#if 0
    if (audio_music_paused ())
    {
      audio_music_play ();
    }
#endif
    
    metrics_event_log (METRICS_EVENT_APP_FG, NULL);
    
    cxi64 currentTime = cx_time_get_utc_epoch ();
    cxi64 timeElapsed = (currentTime - g_bgTime) + g_prevTimeInBg;
    cxi64 timeRefreshSeconds = 1 * 30;
    
    if (timeElapsed >= timeRefreshSeconds)
    {
      // refresh selected city
      if (earth_data_validate_index (g_selectedCity))
      {
        const char *query = earth_data_get_feed_query (g_selectedCity);
        feed_news_t *feedNews = &g_feedsNews [g_selectedCity];
        feed_twitter_t *feedTwitter = &g_feedsTwitter [g_selectedCity];
        
        if (feedTwitter->reqStatus == FEED_REQ_STATUS_INVALID)
        {
          float lat, lon;
          earth_data_get_terrestrial_coords (g_selectedCity, &lat, &lon);
          bool loc = settings_get_local_tweets_only ();
          
          feeds_twitter_search (feedTwitter, query, loc, lat, lon);
          util_activity_indicator_set_active (true);
        }
        
        if (feedNews->reqStatus == FEED_REQ_STATUS_INVALID)
        {
          feeds_news_search (feedNews, query);
          util_activity_indicator_set_active (true);
        }
      }
      
      g_prevTimeInBg = 0;
    }
    else
    {
      g_prevTimeInBg = timeElapsed;
    }
    
    // update clocks
    g_dateUpdateTimer = 0.0f;
    
    // trigger weather update
    g_weatherUpdateTime = 0;
    
    // update dst offsets
    earth_data_update_dst_offsets ();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_on_terminate (void)
{
  if (g_appState == APP_STATE_UPDATE)
  {
    settings_data_save ();
    metrics_event_log (METRICS_EVENT_APP_TERM, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_on_memory_warning (void)
{
  CX_LOG_CONSOLE (1, "BONKERS! RECEIVED MEMORY WARNING!!!");
  CX_LOG_CONSOLE (1, "BONKERS! RECEIVED MEMORY WARNING!!!");
  CX_LOG_CONSOLE (1, "BONKERS! RECEIVED MEMORY WARNING!!!");
  
  if (g_appState == APP_STATE_UPDATE)
  {
    util_status_bar_set_msg (STATUS_BAR_MSG_IOS_MEMORY_WARNING);
    
    cx_http_clear_cache ();
    
    metrics_event_log (METRICS_EVENT_APP_MEMORY_WARNING, NULL);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
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
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
