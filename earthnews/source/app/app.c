//
//  app.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 02/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include "app.h"
#include "camera.h"

#include "../engine/cx_vector4.h"
#include "../engine/cx_matrix4x4.h"
#include "../engine/cx_time.h"
#include "../engine/cx_font.h"
#include "../engine/cx_draw.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


#define DEFAULT_PERSPECTIVE_PROJECTION_NEAR   (0.1f)
#define DEFAULT_PERSPECTIVE_PROJECTION_FAR    (100.0f)
#define DEFAULT_ORTHOGRAPHIC_PROJECTION_NEAR  (-1.0f)
#define DEFAULT_ORTHOGRAPHIC_PROJECTION_FAR   (1.0f)

static cx_shader *s_shader = NULL;
static cx_shader *s_fontShader = NULL;
static cx_mesh *s_mesh = NULL;
static cx_texture *s_texture = NULL;
static cx_texture *s_quadTexture = NULL;
static cx_material *s_material = NULL;

static camera_t s_camera;
static cx_mat4x4 s_mvpMatrix;
static cx_mat3x3 s_normalMatrix;

static cx_font *s_font;

static cx_vec2 s_rotaxis;
static float s_rotAccel = 0.0f;
static float s_rotSpeed = 0.0f;
static float s_rotAngle = 0.0f;

static float s_rotationAngleX = 0.0f;
static float s_rotationSpeedX = 0.0f;
static float s_rotationAccelX = 0.0f; //20.0f;

static float s_rotationAngleY = 0.0f;
static float s_rotationSpeedY = 0.0f;
static float s_rotationAccelY = 0.0f; //20.0f;

#define VIEW_ROTATION_MAX_SPEED      45.0f
#define VIEW_ROTATION_BRAKING_FACTOR 3.0f

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render_2d (void);
void app_render_3d (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


void app_initialise (int width, int height)
{
  (void)s_shader;
  (void)s_mesh;
  (void)s_texture;
  (void)s_material;
  
  cx_time_global_init ();
  cx_graphics_initialise (width, height);
  cx_graphics_set_viewport (width, height);
  
  cx_draw_initialise ();
  
  cx_mat4x4_identity (&s_mvpMatrix);
  cx_mat3x3_identity (&s_normalMatrix);
  
  cx_graphics_print_info ();
  cx_mat4x4_identity (&s_mvpMatrix);

  // shader
  s_shader = cx_shader_create ("mesh", "data/shaders");
  CX_FATAL_ASSERT (s_shader);

  s_fontShader = cx_shader_create ("font", "data/shaders");
  CX_FATAL_ASSERT (s_fontShader);

  // texture  
  s_texture = cx_texture_create_from_file ("data/textures/earthmap1k.png");
  s_quadTexture = cx_texture_create_from_file ("data/textures/avatar.png");

  // material
  s_material = cx_material_create ("material-1"); 
  cx_material_attach_texture (s_material, s_texture, CX_MATERIAL_TEXTURE_AMBIENT);
  cx_material_set_properties (s_material, CX_MATERIAL_PROPERTY_AMBIENT);
  
  // mesh
  s_mesh = cx_mesh_create_sphere (64, 1.0f, s_shader, s_material);
  
  // camera 
  camera_init (&s_camera, 65.0f);
  CX_REFERENCE_UNUSED_VARIABLE (s_camera);
  
  s_font = cx_font_create ("data/fonts/courier_new.ttf", 24.0f, s_fontShader);
  
  s_rotaxis.x = 1.0f;
  s_rotaxis.y = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_deinitialise (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_update (void)
{
  cx_time_global_update ();
  
  float deltaTime = (float) cx_time_global_delta_time ();
  
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
  
#if 0
  cx_vec4_set (&s_camera.position, 0.0f, 0.0f, -4.0f, 1.0f);
  cx_vec4_set (&s_camera.target, 0.0f, 0.0f, 0.0f, 1.0f);
  
  // update view and projetion matrix
  cx_mat4x4 projmatrix;
  cx_mat4x4_perspective (&projmatrix, cx_rad (s_camera.fov), aspectRatio, DEFAULT_PERSPECTIVE_PROJECTION_NEAR, DEFAULT_PERSPECTIVE_PROJECTION_FAR);
  
  // set rotation
  cx_vec4 center = {{0.0f, 0.0f, 0.0f, 1.0f}};
  
  cx_vec4 axis_x = {{1.0f, 0.0f, 0.0f, 0.0f}};
  cx_vec4 axis_y = {{0.0f, 1.0f, 0.0f, 0.0f}};
  
  camera_rotate_around_point (&s_camera, &center, cx_rad (s_rotationAngleX), &axis_x);
  camera_rotate_around_point (&s_camera, &center, cx_rad (s_rotationAngleY), &axis_y);
  
  /*
   cx_vec4 axis = {{s_rotaxis.x, s_rotaxis.y, 0.0f, 0.0f}};
   camera_rotate_around_point (&s_camera, &center, cx_rad (s_rotAngle), &axis);
   */
  
  // get view matrix
  cx_mat4x4 viewmatrix;
  camera_get_view_matrix (&s_camera, &viewmatrix);
  
  // compute modelviewprojection matrix
  cx_mat4x4_mul (&s_mvpMatrix, &projmatrix, &viewmatrix);
  
  cx_mat3x3_identity (&s_normalMatrix); 
  
  camera_update (&s_camera, 0.0f);
  
  
  cx_graphics_set_mvp_matrix (&s_mvpMatrix);
  
#else
  cx_mat4x4 proj;
  cx_mat4x4_perspective (&proj, cx_rad (65.0f), aspectRatio, DEFAULT_PERSPECTIVE_PROJECTION_NEAR, DEFAULT_PERSPECTIVE_PROJECTION_FAR);
  
  cx_mat4x4 translation;
  cx_mat4x4_translation (&translation, 0.0f, 0.0f, -7.0f);
  
  cx_mat4x4 rotation;
  //float angle = (float) cx_time_global_total_time () * 6.0f;
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
  
  cx_mat4x4_mul (&s_mvpMatrix, &proj, &transformation);
  
  cx_graphics_set_mvp_matrix (&s_mvpMatrix);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_began (float x, float y)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_input_touch_moved (float x, float y, float prev_x, float prev_y)
{
  float dx = x - prev_x;
  float dy = y - prev_y;
  
  float w = 768.0f;
  float h = 1024.0f;
  
  s_rotationAccelX = dy * 10.0f * w;
  s_rotationAccelY = dx * 10.0f * h;
  
  float length = cx_sqrt ((dx * dx) + (dy * dy));
  
  if (length > 0.0f)
  {
    s_rotAccel = length * (10.0f * 1024.0f);
    
    dx /= length;
    dy /= length;
    
    float perp_dx = dy;
    float perp_dy = -dx;
    
    s_rotaxis.x = perp_dx;
    s_rotaxis.y = perp_dy;
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
  
  s_camera.fov += f;
  
  s_camera.fov = cx_clamp (s_camera.fov, 30.0f, 90.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_render (void)
{
  //cx_graphics_clear (cx_colour_black ());
  cx_graphics_clear (cx_colour_white ());
  
  app_render_3d ();
  app_render_2d ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// 2d - begin
// - enable 2d shader
// - set render states
// - set mvp matrix
// - set material

void app_render_2d (void)
{
  //////////////
  // begin
  //////////////
  
  cx_graphics_unbind_all_buffers ();
  cx_graphics_activate_renderstate (CX_GRAPHICS_RENDER_STATE_BLEND);
  cx_graphics_set_blend_mode (CX_GRAPHICS_BLEND_MODE_SRC_ALPHA, CX_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_graphics_enable_z_buffer (false);
  
  // set 2d matrix
  float screenWidth = cx_graphics_get_screen_width ();
  float screenHeight = cx_graphics_get_screen_height ();
  cx_mat4x4 orthoProjMatrix;
  cx_mat4x4_ortho (&orthoProjMatrix, 0.0f, screenWidth, 0.0f, screenHeight, DEFAULT_ORTHOGRAPHIC_PROJECTION_NEAR, DEFAULT_ORTHOGRAPHIC_PROJECTION_FAR); 
  
  cx_graphics_set_mvp_matrix (&orthoProjMatrix);
  
  //////////////
  // render
  //////////////
  
  float x1 = 60.0f;
  float y1 = 100.0f;
  float x2 = x1 + 100.0f;
  float y2 = y1 + 100.0f;
  
  cx_draw_quad_colour (x1, y1, x2, y2, cx_colour_blue ());
  
  x1 += 100.0f;
  x2 += 100.0f;
  
  float u1 = 0.0f;
  float v1 = 0.0f;
  float u2 = 1.0f;
  float v2 = 1.0f;
  
  cx_draw_quad_texture (x1, y1, x2, y2, u1, v1, u2, v2, cx_colour_white(), s_quadTexture);
  
  cx_font_render (s_font, "Jack and Jill went up the hill", 4.0f, 36.0f, cx_colour_green ());
  
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
  cx_graphics_activate_renderstate (CX_GRAPHICS_RENDER_STATE_CULL | CX_GRAPHICS_RENDER_STATE_DEPTH_TEST);
  
  
  //////////////
  // render
  //////////////
  
  // use shader
  cx_shader_use (s_mesh->shader);
  
  // set uniforms
  cx_graphics_set_mvp_matrix (&s_mvpMatrix);
  
  cx_shader_write_to_uniform (s_mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, s_mvpMatrix.f16);
  cx_shader_write_to_uniform (s_mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_N, CX_SHADER_DATATYPE_MATRIX3X3, s_normalMatrix.f9);
  
  // mesh render
  cx_mesh_render (s_mesh);
  
  //////////////
  // end
  //////////////
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
