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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


#define DEFAULT_PERSPECTIVE_PROJECTION_NEAR 0.1f
#define DEFAULT_PERSPECTIVE_PROJECTION_FAR  100.0f

static cx_shader *s_shader = NULL;
static cx_mesh *s_mesh = NULL;
static cx_texture *s_texture = NULL;
static cx_material *s_material = NULL;

static camera_t s_camera;
static cx_mat4x4 s_mvpMatrix;
static cx_mat3x3 s_normalMatrix;

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

void app_initialise (int width, int height)
{
  cx_time_global_init ();
  cx_graphics_initialise (width, height);
  
  s_shader = cx_shader_load ("Shader", NULL);
  CX_ASSERT (s_shader);
  
  cx_mat4x4_identity (&s_mvpMatrix);
  cx_mat3x3_identity (&s_normalMatrix);
  
  cx_graphics_print_info ();
  cx_mat4x4_identity (&s_mvpMatrix);
  
  // shader
  s_shader = cx_shader_load ("Shader", NULL);
  CX_FATAL_ASSERT (s_shader != NULL);
  
  // mesh
  s_mesh = cx_mesh_create_sphere (64, 1.5f);
  cx_mesh_gpu_init (s_mesh, s_shader);
  
  // texture  
  s_texture = cx_texture_create ("earthmap1k.png");
  cx_texture_gpu_init (s_texture);
  
  // material
  s_material = cx_material_create ("material-1"); 
  cx_material_attach_texture (s_material, s_texture, CX_MATERIAL_TEXTURE_AMBIENT);
  cx_material_set_properties (s_material, CX_MATERIAL_PROPERTY_AMBIENT);
  
  //cx_graphics_enable_culling (true, 0);
  cx_graphics_enable_depth_test (true);
  
  // camera 
  camera_init (&s_camera, 65.0f);
  CX_REFERENCE_UNUSED_VARIABLE (s_camera);
  
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
  cx_vec4_set (&s_camera.position, 0.0f, 0.0f, -7.0f, 1.0f);
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
  camera_update (&s_camera, 0.0f);
  
  cx_mat3x3_identity (&s_normalMatrix); 
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
  cx_mat4x4_mul (&s_mvpMatrix, &proj, &transformation);
  
  cx_mat3x3_identity (&s_normalMatrix);
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
    CX_OUTPUTLOG_CONSOLE (1, "app_input_touch_moved: length [%.3f] <= 0", length);
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
  cx_graphics_clear (cx_colour_black ());
  
  // use shader
  cx_shader_use (s_shader);
 
  // set uniforms
  cx_shader_write_to_uniform (s_shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, s_mvpMatrix.f16);
  cx_shader_write_to_uniform (s_shader, CX_SHADER_UNIFORM_TRANSFORM_N, CX_SHADER_DATATYPE_MATRIX3X3, s_normalMatrix.f9);
  
  // mesh render
  cx_mesh_render (s_mesh, s_shader);
  
  // material render
  cx_material_render (s_material, s_shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void app_test_code (void)
{
#if CX_DEBUG
  CX_OUTPUTLOG_CONSOLE (1, "app_initialise");
  
  cx_timer timer;
  
  cx_vec4 vec0, vec1, result;
  
  cx_vec4_zero (&vec0);
  cx_vec4_set (&vec1, 1.0f, 2.0f, 3.0f, 0.0f);
  cx_vec4_add (&result, &vec0, &vec1);
  
  cx_vec4 copy = result;
  
  cx_vec4 mult; float scalar = 1.5f;
  cx_vec4_mul (&mult, scalar, &copy);
  
  cx_vec4_sub (&result, &vec0, &vec1);
  
  float dotp = cx_vec4_dot(&result, &copy);
  CX_REFERENCE_UNUSED_VARIABLE (dotp);
  CX_OUTPUTLOG_CONSOLE (1, "Dot product = %f", dotp);
  
  cx_mat4x4 mat0, mat1, mat2;
  
  const int bufSize = 128;
  char logStr [bufSize];
  
  cx_mat4x4_zero(&mat0);
  cx_mat4x4_string(logStr, bufSize, &mat0);
  CX_OUTPUTLOG_CONSOLE(1, "zero matrix : %s", logStr);
  
  cx_mat4x4_identity(&mat0);
  cx_mat4x4_string(logStr, bufSize, &mat0);
  CX_OUTPUTLOG_CONSOLE(1, "identity matrix : %s", logStr);
  
  cxf32 farray1 [16] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
  cx_mat4x4_set(&mat0, farray1);
  cx_mat4x4_string(logStr, bufSize, &mat0);
  CX_OUTPUTLOG_CONSOLE(1, "set matrix : %s", logStr);
  
  cx_mat4x4_transpose(&mat1, &mat0);
  cx_mat4x4_string(logStr, bufSize, &mat1);
  CX_OUTPUTLOG_CONSOLE(1, "transpose matrix : %s", logStr);
  
  cxf32 farray2 [16] = { 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2 };
  cx_mat4x4_set (&mat0, farray2);
  
  cx_mat4x4_add(&mat0, &mat2, &mat1);
  cx_mat4x4_string(logStr, bufSize, &mat2);
  CX_OUTPUTLOG_CONSOLE(1, "add matrix : %s", logStr);
  
  cx_mat4x4_sub(&mat0, &mat2, &mat1);
  cx_mat4x4_string(logStr, bufSize, &mat2);
  CX_OUTPUTLOG_CONSOLE(1, "sub matrix : %s", logStr);
  
  cx_mat4x4_mul_scalar(&mat2, &mat0, 3.0f);
  cx_mat4x4_string(logStr, bufSize, &mat2);
  CX_OUTPUTLOG_CONSOLE(1, "mul scalar matrix : %s", logStr);
  
  cx_vec4_set (&vec0, 1.0f, 2.0f, 3.0f, 0.0f);
  cx_mat4x4_identity(&mat0);
  cx_mat4x4_mul_vec4(&vec1, &mat0, &vec0);
  cx_vec4_string(logStr, bufSize, &vec1);
  CX_OUTPUTLOG_CONSOLE(1, "mul vec vector: %s", logStr);
  
  cx_mat4x4_mul(&mat2, &mat0, &mat1);
  cx_mat4x4_string(logStr, bufSize, &mat0);
  CX_OUTPUTLOG_CONSOLE(1, "mul matrix : %s", logStr);
  
  
  CX_OUTPUTLOG_CONSOLE(1, "matrix multiply test");
  const int testCount = 0;// 8 * 1024 * 1024;
  
  cx_timer_start(&timer);
  
  for (int i = 0; i < testCount; ++i)
  {
    cx_mat4x4_set(&mat1, farray1);
    cx_mat4x4_set (&mat2, farray2);
    
    cx_mat4x4_mul(&mat2, &mat0, &mat1);
    cx_vec4_add (&result, &vec0, &vec1);
  }
  
  cx_timer_stop (&timer);
  
  printf ("Elasped Time = %.3f ms\n", timer.elapsedTime);
  //CX_OUTPUTLOG_CONSOLE(1, "Elasped Time = %.3f ms", timer.elapsedTime);
  
  CX_DEBUG_BREAK_ABLE
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
