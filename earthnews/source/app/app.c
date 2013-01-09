//
//  app.c
//  earthnews
//
//  Created by Ubaka Onyechi on 02/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "app.h"
#include "camera.h"
#include "render.h"
#include "feeds.h"
#include "earth.h"
#include "browser.h"
#include "worker.h"
#include "ui_ctrlr.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_PROJECTION_PERSPECTIVE_NEAR   (0.1f)
#define CAMERA_PROJECTION_PERSPECTIVE_FAR    (100.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR  (-1.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_FAR   (1.0f)

#define CAMERA_MIN_FOV (50.0f)
#define CAMERA_MAX_FOV (80.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum 
{
  COMMS_FEEDS_PENDING,
} comms_status_t;

typedef struct render2d
{
  cx_font *font;
  cx_vec4 *renderPos;
  float *opacity;
} render2d_t;

static earth_t *s_earth = NULL;
static cx_font *s_font = NULL;
static camera_t *s_camera = NULL;
static browser_rect_t s_browserRect;
static render2d_t s_render2dInfo;

static cx_vec2 s_rotTouchBegin;
static cx_vec2 s_rotTouchEnd;
static cx_vec2 s_rotationSpeed;
static cx_vec2 s_rotationAngle;

static feed_twitter_t *s_twitterFeeds = NULL;
static feed_news_t *s_newsFeeds = NULL;
static feed_weather_t *s_weatherFeeds = NULL;

static int s_selectedCity = -1;
static int s_currentWeatherCity = -1;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_handle_touch_event (const input_touch_event *event);
static void app_input_handle_gesture_event (const input_gesture_event *event);
static void app_input_touch_began (float x, float y);
static void app_input_touch_moved (float x, float y, float px, float py);
static void app_input_touch_ended (float x, float y);
static void app_input_zoom (float factor);

static bool app_input_touch_earth (float screenX, float screenY, float screenWidth, float screenHeight);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_camera (float deltaTime);
static void app_update_earth (void);
static void app_update_feeds (void);
static void app_update_feeds_weather (void);

static void app_render_3d (void);
static void app_render_3d_earth (void);

static void app_render_2d (void);
static void app_render_2d_earth (void);
static void app_render_2d_feeds (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
static void app_create_ui (void);
static void app_update_ui (void);
static void app_render_ui (void);
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_init (void *rootvc, float width, float height)
{ 
  CX_ASSERT (rootvc);
  
  //
  // engine
  //
  
  cx_engine_init_params params;
  memset (&params, 0, sizeof (params));
  params.screenWidth = (int) width;
  params.screenHeight = (int) height;
  
  cx_engine_init (CX_ENGINE_INIT_ALL, &params);
  
  render_init (width, height);
  
  //
  // input
  //
  
  input_init ();
  input_register_touch_event_callback (INPUT_TOUCH_TYPE_BEGIN, app_input_handle_touch_event);
  input_register_touch_event_callback (INPUT_TOUCH_TYPE_END, app_input_handle_touch_event);
  input_register_touch_event_callback (INPUT_TOUCH_TYPE_MOVE, app_input_handle_touch_event);
  input_register_gesture_event_callback (INPUT_GESTURE_TYPE_PINCH, app_input_handle_gesture_event);
  
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
  
  s_camera = camera_create (width / height, 65.0f);
  
  //
  // earth data
  //
  
  s_earth = earth_create ("data/earth_data.json", 1.0f, 128, 64);

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
  s_render2dInfo.opacity = cx_malloc (sizeof (float) * cityCount);
  s_render2dInfo.font = cx_font_create ("data/fonts/verdana.ttf", 14);
  
  //
  // font 
  //
  
  //s_font = cx_font_create ("data/fonts/courier_new.ttf", 36);
  s_font = cx_font_create ("data/fonts/verdana.ttf", 28);
  
  //
  // other
  //
  memset (&s_rotTouchBegin, 0, sizeof (s_rotTouchBegin));
  memset (&s_rotTouchEnd, 0, sizeof (s_rotTouchEnd));
  memset (&s_rotationSpeed, 0, sizeof (s_rotationSpeed));
  memset (&s_rotationAngle, 0, sizeof (s_rotationAngle));

  const char *w = s_earth->data->weatherId [0];
  feeds_weather_search (&s_weatherFeeds [0], w);
  s_currentWeatherCity = 0;
  
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
  
  render_deinit ();
  
  input_deinit ();
  
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
  
  render_screen_reset (width, height);
  
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
  
  input_update ();
  
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  deltaTime = cx_min (deltaTime, (1.0f / 60.0f));
    
  app_update_camera (deltaTime);
  
  app_update_earth ();
  
  app_update_feeds ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render (void)
{
  cx_gdi_clear (cx_colour_black ());
  
  app_render_3d ();
  
  app_render_2d ();
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
  
  cx_vec2 tdist;
  
  tdist.x = s_rotTouchEnd.x - s_rotTouchBegin.x;
  tdist.y = s_rotTouchEnd.y - s_rotTouchBegin.y;
  
#if 0
  cx_vec2 accel;
  
  accel.x = tdist.x - s_rotationSpeed.x;
  accel.y = tdist.y - s_rotationSpeed.x;
  
  s_rotationSpeed.x += (accel.x * deltaTime);
  s_rotationSpeed.y += (accel.y * deltaTime);
  
  s_rotationSpeed.x = cx_clamp (s_rotationSpeed.x, -MaxSpeed, MaxSpeed);
  s_rotationSpeed.y = cx_clamp (s_rotationSpeed.y, -MaxSpeed, MaxSpeed);
  
  s_rotTouchBegin.x += (s_rotationSpeed.x * deltaTime);
  s_rotTouchBegin.y += (s_rotationSpeed.y * deltaTime);
  
  s_rotationAngle.x += (s_rotationSpeed.x * deltaTime);
  s_rotationAngle.y += (s_rotationSpeed.y * deltaTime);
#elif 0
  //tdist.x *= (sw / MaxSpeed);
  //tdist.y *= (sh / MaxSpeed);
  
  tdist.x *= (MaxSpeed / sw);
  tdist.y *= (MaxSpeed / sh);
  
  s_rotationSpeed.x += (tdist.x * deltaTime);
  s_rotationSpeed.y += (tdist.y * deltaTime);
  
  s_rotationSpeed.x = cx_clamp (s_rotationSpeed.x, -MaxSpeed, MaxSpeed);
  s_rotationSpeed.y = cx_clamp (s_rotationSpeed.y, -MaxSpeed, MaxSpeed);
  
  s_rotTouchBegin.x += (s_rotationSpeed.x * deltaTime);
  s_rotTouchBegin.y += (s_rotationSpeed.y * deltaTime);
  
  s_rotationAngle.x += (s_rotationSpeed.x * deltaTime);
  s_rotationAngle.y += (s_rotationSpeed.y * deltaTime);
#else
  CX_DEBUGLOG_CONSOLE (0, "tdist.x: %.3f, tdist.y: %.3f", tdist.x, tdist.y);
  
  s_rotTouchBegin.x += (tdist.x * deltaTime);
  s_rotTouchBegin.y += (tdist.y * deltaTime);
  
  s_rotationAngle.x += (tdist.x * deltaTime);
  s_rotationAngle.y += (tdist.y * deltaTime);
#endif
  
  s_rotationAngle.x = fmodf (s_rotationAngle.x, 360.0f);
  s_rotationAngle.y = cx_clamp (s_rotationAngle.y, -89.9f, 89.9f);
  
#if 1
  cx_vec4_set (&s_camera->position, 0.0f, 0.0f, -2.0f, 1.0f);
  cx_vec4_set (&s_camera->target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update camera (view and projetion matrix)

  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  cx_vec4 axis_x = {{1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, -1.0f, 0.0f, 0.0f}};
  
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngle.y), &axis_x);
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngle.x), &axis_y);
  
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
    float alpha = cx_smoothstep (0.95f, 1.0f, dotp);
    
    s_render2dInfo.opacity [i] = opacity * alpha;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds (void)
{
  app_update_feeds_weather ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_update_feeds_weather (void)
{
  if ((s_currentWeatherCity != -1) && (s_currentWeatherCity < s_earth->data->count))
  {
    if (s_weatherFeeds [s_currentWeatherCity].dataReady)
    {
      ++s_currentWeatherCity;
      
      if (s_currentWeatherCity < s_earth->data->count)
      {
        const char *w = s_earth->data->weatherId [s_currentWeatherCity];
        feeds_weather_search (&s_weatherFeeds [s_currentWeatherCity], w);
      }
      else
      {
        s_currentWeatherCity = -1;
      }
    }
  }
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
  
  // draw points
  //cx_draw_points (s_earth->data->count, s_earth->data->location, cx_colour_red (), NULL);
  
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
  
  render_test ();
  render_fps ();
  
  //////////////
  // render
  //////////////
  
  app_render_2d_earth ();
  
  // render feeds
  
  app_render_2d_feeds ();
  
  // render browser
  
  browser_render (&s_browserRect, 1.0f);
  
  //////////////
  // end
  //////////////
  
  cx_gdi_enable_z_write (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_feeds (void)
{
#if 0
  if (s_selectedCity > -1)
  {
    CX_ASSERT (s_selectedCity < s_earth->data->count);
    
    feed_twitter_t *twitterFeed = &s_twitterFeeds [s_selectedCity];
    
    feeds_twitter_render (twitterFeed);
  }
#endif
  
  if (s_selectedCity > -1)
  {
    CX_ASSERT (s_selectedCity < s_earth->data->count);
    
    feed_news_t *newsFeed = &s_newsFeeds [s_selectedCity];
    
    ui_ctrlr_set_news_feed (newsFeed);
    
    feed_twitter_t *twitterFeed = &s_twitterFeeds [s_selectedCity];
    
    ui_ctrlr_set_twitter_feed (twitterFeed);
  }
  
  ui_ctrlr_render ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_2d_earth (void)
{
#if 1
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_enable_z_write (true);
  
  for (int i = 0, c = s_earth->data->count; i < c; ++i)
  {
    const char *text = s_earth->data->names [i];
    const feed_weather_t *feed = &s_weatherFeeds [i];
    const cx_vec4 *pos = &s_render2dInfo.renderPos [i];
    float opacity = s_render2dInfo.opacity [i];
    
    cx_colour colour = (i == s_selectedCity) ? *cx_colour_yellow () : *cx_colour_white ();
    colour.a = opacity;
    
    //cx_font_set_scale (s_font, pos->w, pos->w);
    
    cx_font_render (s_render2dInfo.font, text, pos->x, pos->y, pos->z, CX_FONT_ALIGNMENT_CENTRE_X, &colour);
    
    // render weather icon
    
    feeds_weather_render (feed, pos->x, pos->y - 6.0f, pos->z, opacity);
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
  if (ui_ctrlr_handle_input (event))
  {
    CX_DEBUGLOG_CONSOLE (1, "UI input handled");
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
  switch (event->type) 
  {
    case INPUT_GESTURE_TYPE_PINCH: { app_input_zoom (event->data.pinch.factor); break; }
    default:                       { break; }
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
    if (app_input_touch_earth (touchX, touchY, sw, sh))
    {
      int index = s_selectedCity;
      CX_ASSERT (index > -1);
      
      const char *n = s_earth->data->names [index];
      
      feeds_twitter_search (&s_twitterFeeds [index], n);
      
      feeds_news_search (&s_newsFeeds [index], n);
    }
  }
  
  s_rotTouchBegin.x = x * sw * 0.25f;
  s_rotTouchBegin.y = y * sh * 0.25f;
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_input_touch_moved (float x, float y, float prev_x, float prev_y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
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
  
  //s_rotTouchEnd.x = x * sw * 0.25f;
  //s_rotTouchEnd.y = y * sh * 0.25f;
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

static bool app_input_touch_earth (float screenX, float screenY, float screenWidth, float screenHeight)
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
  
  int cityIndex = -1;
  
  int i, c;
  for (i = 0, c = s_earth->data->count; i < c; ++i)
  {    
#if CX_DEBUG
    const char *city = s_earth->data->names [i];
#endif
    
    float opacity = s_render2dInfo.opacity [i];
    
    if (opacity > 0.1f)
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
        
        float distSqr = cx_vec4_lengthSqr (&toPos);
        
        if (distSqr <= (0.025f * 0.025f))
        {
          CX_DEBUGLOG_CONSOLE (1, "%s", city);
          
          cityIndex = i;
          
          break;
        } 
      }
    }
  }
  
  if (cityIndex > -1)
  {
    s_selectedCity = cityIndex;
    
    return true;
  }
  
  return false;
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
