//
//  app.c
//  earthnews
//
//  Created by Ubaka Onyechi on 02/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "app.h"
#include "camera.h"
#include "ui.h"
#include "http.h"
#include "feeds.h"
#include "earth.h"
#include "webview.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEFAULT_PERSPECTIVE_PROJECTION_NEAR   (0.1f)
#define DEFAULT_PERSPECTIVE_PROJECTION_FAR    (100.0f)
#define DEFAULT_ORTHOGRAPHIC_PROJECTION_NEAR  (-1.0f)
#define DEFAULT_ORTHOGRAPHIC_PROJECTION_FAR   (1.0f)

static earth_t *s_earth = NULL;
static cx_font *s_font = NULL;
static camera_t *s_camera = NULL;

static float s_rotAccel = 0.0f;
static float s_rotSpeed = 0.0f;
static float s_rotAngle = 0.0f;

static float s_rotationAngleX = 0.0f;
static float s_rotationSpeedX = 0.0f;
static float s_rotationAccelX = 0.0f; //20.0f;

static float s_rotationAngleY = 0.0f;
static float s_rotationSpeedY = 0.0f;
static float s_rotationAccelY = 0.0f; //20.0f;

#define VIEW_ROTATION_MAX_SPEED       45.0f
#define VIEW_ROTATION_BRAKING_FACTOR  3.0f

static int g_selectedCity = -1;

twitter_feed_t *s_twitterFeeds = NULL;
news_feed_t *s_rssFeeds = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void);
void app_render_2d (void);
void app_render_3d (void);

void app_earth_hit_test (float x, float y);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_reset (int width, int height)
{
  cx_graphics_set_screen_dimensions (width, height);
  s_camera->aspectRatio = (float) width / (float) height;
  
  ui_screen_reset ((float)width, (float) height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_init (int width, int height)
{ 
  //
  // engine
  //
  
  cx_engine_init_params params;
  memset (&params, 0, sizeof (params));
  params.screenWidth = width;
  params.screenHeight = height;
  
  cx_engine_init (CX_ENGINE_INIT_ALL, &params);
  
  //
  // http
  //
  
  http_init ();
  
  //
  // ui
  //
  
  ui_init ((float) width, (float) height);
  
  //
  // webview
  //
  
  web_view_init ((float) width, (float) height);
  
  //
  // camera
  //
  
  s_camera = camera_create ((float) width / (float) height, 65.0f);
  
  //
  // earth
  //
  
  s_earth = earth_create ("data/earth_data.json", 1.0f, 64, 32);

  //
  // feeds
  //
  
  s_twitterFeeds = cx_malloc (sizeof (twitter_feed_t) * s_earth->data->count);
  memset (s_twitterFeeds, 0, sizeof (twitter_feed_t) * s_earth->data->count);
  
  s_rssFeeds = cx_malloc (sizeof (news_feed_t) * s_earth->data->count);
  memset (s_rssFeeds, 0, sizeof (news_feed_t) * s_earth->data->count);
  
  //
  // font 
  //
  
  s_font = cx_font_create ("data/fonts/courier_new.ttf", 36);
  
  //
  // test code
  //
  
#if 1
  cx_file file0, file1, file2;
  cx_file_load (&file0, "data/rss.txt");
  cx_file_load (&file1, "data/twitter.txt");
  cx_file_load (&file2, "data/weather.txt");
  
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
  
  CX_DEBUG_BREAK_ABLE;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_deinit (void)
{
  ui_deinit ();
  
  cx_engine_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_update (void)
{
  cx_system_time_update ();
  
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  app_view_update (deltaTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_view_update (float deltaTime)
{
  float aspectRatio = cx_graphics_get_aspect_ratio ();
  CX_REFERENCE_UNUSED_VARIABLE (aspectRatio);
  
  const float maxRotationSpeed = 40.0f;
  const float decelerationFactor = 0.80f;
  
  s_rotationSpeedX += (s_rotationAccelX * deltaTime);
  s_rotationSpeedY += (s_rotationAccelY * deltaTime);
  
  s_rotationSpeedX = cx_clamp (s_rotationSpeedX, -maxRotationSpeed, maxRotationSpeed);
  s_rotationSpeedY = cx_clamp (s_rotationSpeedY, -maxRotationSpeed, maxRotationSpeed);
  
  if (s_rotationAccelX == 0.0f)
  {
    s_rotationSpeedX *= decelerationFactor;
  }
  
  if (s_rotationAccelY == 0.0f)
  {
    s_rotationSpeedY *= decelerationFactor;
  }
  
  s_rotationAngleX += (s_rotationSpeedX * deltaTime);
  s_rotationAngleY += (s_rotationSpeedY * deltaTime);
  s_rotationAngleX = cx_clamp (s_rotationAngleX, -89.9f, 89.9f);
  
  if (s_rotAccel == 0.0f)
  {
    s_rotSpeed *= decelerationFactor;
  }
  
  s_rotSpeed += (s_rotAccel * deltaTime);
  s_rotAngle += (s_rotSpeed * deltaTime);
  //s_rotAngle = cx_clamp(s_rotAngle, -90.0f, 90.0f);
  
#if 1
  cx_vec4_set (&s_camera->position, 0.0f, 0.0f, 3.0f, 1.0f);
  cx_vec4_set (&s_camera->target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update camera (view and projetion matrix)

  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  
  cx_vec4 axis_x = {{-1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, -1.0f, 0.0f, 0.0f}};
  
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngleX), &axis_x);
  camera_rotate_around_point (s_camera, &center, cx_rad (s_rotationAngleY), &axis_y);
  
  // get projection matrix
  cx_mat4x4 projmatrix;
  camera_get_projection_matrix (s_camera, &projmatrix);
  
  // get view matrix
  cx_mat4x4 viewmatrix;
  camera_get_view_matrix (s_camera, &viewmatrix);
  
  // compute modelviewprojection matrix
  cx_mat4x4 mvpMatrix;
  cx_mat4x4_mul (&mvpMatrix, &projmatrix, &viewmatrix);
  
  camera_update (s_camera, deltaTime);
  
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_P, &projmatrix);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MV, &viewmatrix);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvpMatrix);
  
#else
  cx_mat4x4 proj;
  cx_mat4x4_perspective (&proj, cx_rad (65.0f), aspectRatio, DEFAULT_PERSPECTIVE_PROJECTION_NEAR, DEFAULT_PERSPECTIVE_PROJECTION_FAR);
  
  cx_mat4x4 translation;
  cx_mat4x4_translation (&translation, 0.0f, 0.0f, -5.0f);
  
  cx_mat4x4 rotation;
  //float angle = (float) cx_system_time_get_total_time () * 6.0f;
  //cx_mat4x4_rotation (&rotation, cx_rad (angle), 0.0f, 1.0f, 0.0f);
  //cx_mat4x4_rotation_axis_y (&rotation, cx_rad (angle));
  //cx_mat4x4_mul (&s_mvpMatrix, &proj, &rotation);
  
  cx_mat4x4 rotx, roty;
  cx_mat4x4_rotation_axis_x (&rotx, cx_rad (s_rotationAngleX));
  cx_mat4x4_rotation_axis_y (&roty, cx_rad (s_rotationAngleY));
  cx_mat4x4_mul (&rotation, &rotx, &roty);

  cx_mat4x4 transformation;
  cx_mat4x4_mul (&transformation, &translation, &rotation);
  
  cx_mat3x3_identity (&s_normalMatrix);
  
  cx_mat4x4 mvpMatrix;
  cx_mat4x4_mul (&mvpMatrix, &proj, &transformation);
  
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_P, &proj);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MV, &transformation);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvpMatrix);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_began (float x, float y)
{
  float screenX = x * cx_graphics_get_screen_width ();
  float screenY = y * cx_graphics_get_screen_height ();
  
  bool hideWebView = true;
  
  if (g_selectedCity > -1)
  {
    const char *url = ui_rss_hit_test (&s_rssFeeds [g_selectedCity], screenX, screenY);
    if (url)
    {
      web_view_launch_url (url);
      hideWebView = false;
      
      CX_DEBUGLOG_CONSOLE (1, "PING! [x = %.2f, y = %.2f", screenX, screenY);
    }
  }
  
  if (hideWebView)
  {
    if (web_view_is_shown ())
    {
      web_view_hide (true);
    }
  }
  
  app_earth_hit_test (x, y);
}

void app_earth_hit_test (float x, float y)
{
  // do ray test
  float screenWidth = cx_graphics_get_screen_width ();
  float screenHeight = cx_graphics_get_screen_height ();
 
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (s_camera, &proj);
  camera_get_view_matrix (s_camera, &view);
  
  cx_vec2 screen = {{ x * screenWidth, y * screenHeight}};
  
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
    
    feeds_twitter_search (&s_twitterFeeds [cityIndex], n);
    feeds_news_search (&s_rssFeeds [cityIndex], n);
    
    g_selectedCity = cityIndex;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_moved (float x, float y, float prev_x, float prev_y)
{
  float dx = x - prev_x;
  float dy = y - prev_y;
  
  float w = cx_graphics_get_screen_width ();
  float h = cx_graphics_get_screen_height ();
  
  s_rotationAccelX = dy * 10.0f * w;
  s_rotationAccelY = dx * 10.0f * h;
  
  float length = cx_sqrt ((dx * dx) + (dy * dy));
  
  if (length > 0.0f)
  {
    s_rotAccel = length * (10.0f * 1024.0f);
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "app_input_touch_moved: length [%.3f] <= 0", length);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_ended (float x, float y)
{
  s_rotationAccelX = 0.0f;
  s_rotationAccelY = 0.0f;
  s_rotAccel = 0.0f;
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
  float screenWidth = cx_graphics_get_screen_width ();
  float screenHeight = cx_graphics_get_screen_height ();
  
  cx_mat4x4 view, proj;
  
  camera_get_projection_matrix (s_camera, &proj);
  camera_get_view_matrix (s_camera, &view);
  
  // for each point, get 2d point
  float scale;
  cx_vec2 screen;
  
  int i, c = s_earth->data->count;
  for (i = 0; i < c; ++i)
  {
    const char *text = s_earth->data->names [i];
    const cx_vec4 *pos = &s_earth->data->location [i];
    const cx_colour *colour = (i == g_selectedCity) ? cx_colour_green () : cx_colour_yellow ();
    
    cx_util_world_space_to_screen_space (screenWidth, screenHeight, &proj, &view, pos, &screen, &scale);

    cx_font_set_scale (s_font, scale, scale);
    cx_font_render (s_font, text, screen.x, screen.y, CX_FONT_ALIGNMENT_CENTRE_X, colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void app_render_earth (void)
{
  cx_mat4x4 mvpMatrix;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvpMatrix);
  
  cx_mat3x3 normalMatrix;
  cx_mat3x3_identity (&normalMatrix);
  
  // get mesh
  cx_mesh *mesh = s_earth->visual->mesh;
  
  // use shader
  cx_shader_use (mesh->shader);
  
  // set uniforms
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvpMatrix.f16);
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_N, CX_SHADER_DATATYPE_MATRIX3X3, normalMatrix.f9);
  
  cx_mesh_render (mesh);
  cx_draw_points_colour (s_earth->data->count, s_earth->data->location, cx_colour_red ());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render_2d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_graphics_unbind_all_buffers ();
  cx_graphics_set_renderstate (CX_GRAPHICS_RENDER_STATE_BLEND);
  cx_graphics_set_blend_mode (CX_GRAPHICS_BLEND_MODE_SRC_ALPHA, CX_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_graphics_enable_z_buffer (false);
  
  // set 2d mvp matrix
  float screenWidth = cx_graphics_get_screen_width ();
  float screenHeight = cx_graphics_get_screen_height ();
  
  cx_mat4x4 orthoProjMatrix;
  cx_mat4x4 orthoViewMatrix;
  
  cx_mat4x4_ortho (&orthoProjMatrix, 0.0f, screenWidth, 0.0f, screenHeight, DEFAULT_ORTHOGRAPHIC_PROJECTION_NEAR, DEFAULT_ORTHOGRAPHIC_PROJECTION_FAR); 
  cx_mat4x4_identity (&orthoViewMatrix);
  
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_P, &orthoProjMatrix);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MV, &orthoViewMatrix);
  cx_graphics_set_transform (CX_GRAPHICS_TRANSFORM_MVP, &orthoProjMatrix);
  
  //////////////
  // render
  //////////////
  
  app_render_earth_city_text ();
  
  ui_render();
  
  if (g_selectedCity > -1)
  {
    ui_render_twitter_feed (&s_twitterFeeds [g_selectedCity]);
    ui_render_rss_feed (&s_rssFeeds [g_selectedCity]);
  }
  
  //////////////
  // end
  //////////////
  
  cx_graphics_enable_z_buffer (true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render_3d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_graphics_unbind_all_buffers ();
  cx_graphics_set_renderstate (CX_GRAPHICS_RENDER_STATE_CULL | CX_GRAPHICS_RENDER_STATE_DEPTH_TEST);
  cx_graphics_enable_z_buffer (true);
  
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
  cx_graphics_clear (cx_colour_black ());
  
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
  cx_timer_start (&timer);
  
  for (int i = 0; i < size; ++i)
  {
    float r = rand () / (RAND_MAX + 1.0f);
    memset (mat16, r, sizeof(mat16));
    
    //cx_vec4_set(&vectors [i], r, r * i, r * i * 2.0f, r * i * 0.5f);
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
  
  cx_timer_stop (&timer);
  
  printf ("app_test_code: %.3f\n", timer.elapsedTime);
  //CX_DEBUGLOG_CONSOLE (1, "app_test_code: %.3f", timer.elapsedTime);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
