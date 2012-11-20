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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_PROJECTION_PERSPECTIVE_NEAR   (0.1f)
#define CAMERA_PROJECTION_PERSPECTIVE_FAR    (100.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_NEAR  (-1.0f)
#define CAMERA_PROJECTION_ORTHOGRAPHIC_FAR   (1.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_VIEW_ROTATION_OLD 0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static earth_t *s_earth = NULL;
static cx_font *s_font = NULL;
static camera_t *s_camera = NULL;
static browser_rect_t s_browserRect;

#if DEBUG_VIEW_ROTATION_OLD
static float s_rotationAngleX = 0.0f;
static float s_rotationSpeedX = 0.0f;
static float s_rotationAccelX = 0.0f;
static float s_rotationAngleY = 0.0f;
static float s_rotationSpeedY = 0.0f;
static float s_rotationAccelY = 0.0f;
#else
static cx_vec2 s_rotTouchBegin;
static cx_vec2 s_rotTouchEnd;
static cx_vec2 s_rotationSpeed;
static cx_vec2 s_rotationAngle;
#endif

static int s_selectedCity = -1;
static twitter_feed_t *s_twitterFeeds = NULL;
static news_feed_t *s_newsFeeds = NULL;
static weather_feed_t *s_weatherFeeds = NULL;

void app_input_touch_update (float deltaTime);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int g_currentWeatherCity = -1;
void app_get_weather (void);

void app_test_code (void);
void app_render_2d (void);
void app_render_3d (void);

void app_earth_hit_test (float screenX, float screenY, float screenWidth, float screenHeight);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_reset (float width, float height)
{
  s_camera->aspectRatio = width / height;
  
  cx_gdi_set_screen_dimensions ((int) width, (int) height);
  
  render_screen_reset (width, height);
  
  s_browserRect.width  = 778.0f;
  s_browserRect.height = 620.0f;
  s_browserRect.posX   = 0.0f + ((width - s_browserRect.width) * 0.5f);
  s_browserRect.posY   = 0.0f + ((height - s_browserRect.height) * 0.5f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


void app_init (void *rootViewController, float width, float height)
{ 
  CX_ASSERT (rootViewController);
  
  //
  // engine
  //
  
  cx_engine_init_params params;
  memset (&params, 0, sizeof (params));
  params.screenWidth = (int) width;
  params.screenHeight = (int) height;
  
  cx_engine_init (CX_ENGINE_INIT_ALL, &params);
  
  //
  // ui
  //
  
  render_init (width, height);
  
  //
  // browser
  //

  browser_init (rootViewController);
  
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
  
  s_earth = earth_create ("data/earth_data.json", 1.0f, 64, 32);

  //
  // feeds
  //
  
  feeds_init ();
  
  int cityCount = s_earth->data->count;
  
  s_twitterFeeds = cx_malloc (sizeof (twitter_feed_t) * cityCount);
  memset (s_twitterFeeds, 0, sizeof (twitter_feed_t) * cityCount);
  
  s_newsFeeds = cx_malloc (sizeof (news_feed_t) * cityCount);
  memset (s_newsFeeds, 0, sizeof (news_feed_t) * cityCount);
  
  s_weatherFeeds = cx_malloc (sizeof (weather_feed_t) * cityCount);
  memset (s_weatherFeeds, 0, sizeof (weather_feed_t) * cityCount);
  
  //
  // font 
  //
  
  //s_font = cx_font_create ("data/fonts/courier_new.ttf", 36);
  s_font = cx_font_create ("data/fonts/verdana.ttf", 28);
  
  //
  // other
  //
#if !DEBUG_VIEW_ROTATION_OLD
  memset (&s_rotTouchBegin, 0, sizeof (s_rotTouchBegin));
  memset (&s_rotTouchEnd, 0, sizeof (s_rotTouchEnd));
  memset (&s_rotationSpeed, 0, sizeof (s_rotationSpeed));
  memset (&s_rotationAngle, 0, sizeof (s_rotationAngle));
#endif
  
  const char *w = s_earth->data->weatherId [0];
  feeds_weather_search (&s_weatherFeeds [0], w);
  g_currentWeatherCity = 0;
  
  //
  // test code
  //
  
#if 0
  cx_file file0, file1, file2;
  cx_file_load (&file0, "data/rss.txt");
  cx_file_load (&file1, "data/twitter.txt");
  cx_file_load (&file2, "data/weather2.txt");
  
  news_feed_t rssFeed;
  memset (&rssFeed, 0, sizeof (news_feed_t));
  feeds_news_parse (&rssFeed, file0.data, file0.size);
  
  twitter_feed_t tweets;
  memset (&tweets, 0, sizeof (twitter_feed_t));
  feeds_twitter_parse (&tweets, file1.data, file1.size);
  
  weather_feed_t weather;
  memset(&weather, 0, sizeof (weather_feed_t));
  feeds_weather_parse (&weather, file2.data, file2.size);
  
  news_feed_item_t *rssItem = rssFeed.items;
  while (rssItem)
  {
    CX_DEBUGLOG_CONSOLE(1, "====rss=====");
    CX_DEBUGLOG_CONSOLE(1, rssItem->date);
    CX_DEBUGLOG_CONSOLE(1, rssItem->link);
    CX_DEBUGLOG_CONSOLE(1, rssItem->title);
    rssItem = rssItem->next;
  }
  
  twitter_tweet_t *twItem = tweets.items;
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
  
  cx_engine_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_update (void)
{
  cx_system_time_update ();
  
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  deltaTime = cx_min (deltaTime, (1.0f / 60.0f));
  
  //CX_DEBUGLOG_CONSOLE (1, "%.2f", deltaTime);
  
  app_view_update (deltaTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_view_update (float deltaTime)
{
  float aspectRatio = cx_gdi_get_aspect_ratio ();
  CX_REFERENCE_UNUSED_VARIABLE (aspectRatio);
  
  app_input_touch_update (deltaTime);
  
#if 1
  cx_vec4_set (&s_camera->position, 0.0f, 0.0f, -2.0f, 1.0f);
  cx_vec4_set (&s_camera->target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update camera (view and projetion matrix)

  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  cx_vec4 axis_x = {{1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, -1.0f, 0.0f, 0.0f}};
  
#if DEBUG_VIEW_ROTATION_OLD
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngleX), &axis_x);
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngleY), &axis_y);
#else
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngle.y), &axis_x);
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngle.x), &axis_y);
#endif
  
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
  
  app_get_weather ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_earth_hit_test (float screenX, float screenY, float screenWidth, float screenHeight)
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
    const char *city = s_earth->data->names [i];
    
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
  
  if (cityIndex > -1)
  {
    const char *n = s_earth->data->names [cityIndex];
    //const char *w = s_earth->data->weatherId [cityIndex];
    
    feeds_twitter_search (&s_twitterFeeds [cityIndex], n);
    feeds_news_search (&s_newsFeeds [cityIndex], n);
    //feeds_weather_search (&s_weatherFeeds [cityIndex], w);
    
    s_selectedCity = cityIndex;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_get_weather (void)
{
  if ((g_currentWeatherCity != -1) && (g_currentWeatherCity < s_earth->data->count))
  {
    if (s_weatherFeeds [g_currentWeatherCity].dataReady)
    {
      ++g_currentWeatherCity;
      
      if (g_currentWeatherCity < s_earth->data->count)
      {
        const char *w = s_earth->data->weatherId [g_currentWeatherCity];
        feeds_weather_search (&s_weatherFeeds [g_currentWeatherCity], w);
      }
      else
      {
        g_currentWeatherCity = -1;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_began (float x, float y)
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
    app_earth_hit_test (touchX, touchY, sw, sh);
    
    if (s_selectedCity > -1)
    {
      CX_ASSERT (s_selectedCity < s_earth->data->count);
      
      news_feed_item_t item;      
      news_feed_t *feed = &s_newsFeeds [s_selectedCity];
      
      if (feeds_news_get_item (&item, feed, touchX, touchY))
      {
        bool opened = browser_open (item.link, item.title);
        CX_REFERENCE_UNUSED_VARIABLE (opened);
      }
    }
  }
  
#if !DEBUG_VIEW_ROTATION_OLD
  s_rotTouchBegin.x = x * sw * 0.25f;
  s_rotTouchBegin.y = y * sh * 0.25f;
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_moved (float x, float y, float prev_x, float prev_y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
#if DEBUG_VIEW_ROTATION_OLD
  
  float dx = x - prev_x;
  float dy = y - prev_y;
  
  float accelerationFactor = 30.0f;
  
  s_rotationAccelX = dy * sh * accelerationFactor;
  s_rotationAccelY = dx * sw * accelerationFactor;
  
#else
  
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
  
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_ended (float x, float y)
{
  float sw = cx_gdi_get_screen_width ();
  float sh = cx_gdi_get_screen_height ();
  
  float touchX = x * sw;
  float touchY = y * sh;
  
  if (browser_is_open ())
  {
    browser_handle_input (&s_browserRect, BROWSER_INPUT_TOUCH_END, touchX, touchY);
  }
  
#if DEBUG_VIEW_ROTATION_OLD
  
  s_rotationAccelX = 0.0f;
  s_rotationAccelY = 0.0f;
  
#else
  
  s_rotTouchEnd.x = x * sw * 0.25f;
  s_rotTouchEnd.y = y * sh * 0.25f;
  
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_update (float deltaTime)
{
#if DEBUG_VIEW_ROTATION_OLD 
  
  const float maxRotationSpeed = 60.0f;
  const float decelerationFactor = 0.9f;
  
  s_rotationSpeedX += (s_rotationAccelX * deltaTime);
  s_rotationSpeedY += (s_rotationAccelY * deltaTime);
  
  s_rotationSpeedX = cx_clamp (s_rotationSpeedX, -maxRotationSpeed, maxRotationSpeed);
  s_rotationSpeedY = cx_clamp (s_rotationSpeedY, -maxRotationSpeed, maxRotationSpeed);
  
  if (cx_is_zero (s_rotationAccelX))
  {
    s_rotationSpeedX *= decelerationFactor;
  }
  
  if (cx_is_zero (s_rotationAccelY))
  {
    s_rotationSpeedY *= decelerationFactor;
  }
  
  s_rotationAngleX += (s_rotationSpeedX * deltaTime);
  s_rotationAngleY += (s_rotationSpeedY * deltaTime);
  s_rotationAngleX = cx_clamp (s_rotationAngleX, -89.9f, 89.9f);
  
#else
  
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
  
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_zoom (float factor)
{
  float f = 1.0f - factor;
  
  s_camera->fov = cx_clamp (s_camera->fov + f, 30.0f, 90.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_earth_city_text (void)
{
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
  
  for (i = 0, c = s_earth->data->count; i < c; ++i)
  {
    const char *text = s_earth->data->names [i];
    const cx_vec4 *pos = &s_earth->data->location [i];
    const cx_colour *colour = (i == s_selectedCity) ? cx_colour_green () : cx_colour_yellow ();
    
    cx_util_world_space_to_screen_space (screenWidth, screenHeight, &proj, &view, pos, &screen, &depth, &scale);

    float clipz = (2.0f * depth) - 1.0f;
    float z = ((clipz + (zfar + znear) / (zfar - znear)) * (zfar - znear)) / -2.0f;
    
    cx_font_set_scale (s_font, scale, scale);
    cx_font_render (s_font, text, screen.x, screen.y, z, CX_FONT_ALIGNMENT_CENTRE_X, colour);
    
    screen.y -= 6.0f;
    
    // render weather icon
    weather_feed_t *feed = &s_weatherFeeds [i];
    feeds_weather_render (feed, screen.x, screen.y, z);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_earth (void)
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
  
  cx_vertex_data *vertexData = &s_earth->visual->mesh->vertexData;
  
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

void app_render_2d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_gdi_unbind_all_buffers ();
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_buffer (false);
  
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
  
#if 1
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_buffer (true);
  app_render_earth_city_text ();
  cx_gdi_enable_z_buffer (false);
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_BLEND);
#else
  app_render_earth_city_text ();
#endif
  
  // render feeds
  
  if (s_selectedCity > -1)
  {
    CX_ASSERT (s_selectedCity < s_earth->data->count);
    
    twitter_feed_t *twitterFeed = &s_twitterFeeds [s_selectedCity];
    news_feed_t *newsFeed       = &s_newsFeeds [s_selectedCity];
    
    feeds_twitter_render (twitterFeed);
    feeds_news_render (newsFeed);
  }
  
  // render browser
  
  browser_render (&s_browserRect, 1.0f);
  
  //////////////
  // end
  //////////////
  
  cx_gdi_enable_z_buffer (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render_3d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_gdi_unbind_all_buffers ();
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_enable_z_buffer (true);
  
  //////////////
  // render
  //////////////

  app_render_earth ();
    
  //////////////
  // end
  //////////////
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
