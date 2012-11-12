//
//  camera.c
//  earthnews
//
//  Created by Ubaka Onyechi on 02/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "camera.h"
// #include "input.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CAMERA_PERSPECTIVE_NEAR       (0.1f)
#define CAMERA_PERSPECTIVE_FAR        (100.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_input_update (camera_t *camera, float deltaTime);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

camera_t *camera_create (float aspectRatio, float fov)
{
  camera_t *camera = cx_malloc (sizeof (camera_t));
  
  camera->aspectRatio = aspectRatio;
  camera->fov = fov;
  
  cx_vec4_set (&camera->position, 0.0f, 0.0f, 0.0f, 1.0f);
  cx_vec4_set (&camera->target, 0.0f, 0.0f, -1.0f, 1.0f);
  cx_vec4_set (&camera->up, 0.0f, 1.0f, 0.0f, 0.0f);
  
  return camera;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_destroy (camera_t *camera)
{
  cx_free (camera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_rotate_view (camera_t *camera, float angle, const cx_vec4 *axis)
{
  CX_ASSERT (camera);
  CX_ASSERT (axis);

  // get rotation matrix
  cx_mat4x4 rotation;
  cx_mat4x4_rotation (&rotation, angle, axis->x, axis->y, axis->z);
  
  // get camera's forward vector (to camera's target)
  cx_vec4 forward;
  cx_vec4_sub (&forward, &camera->target, &camera->position);
  
  // transform forward vector
  cx_vec4 newForward;
  cx_mat4x4_mul_vec4 (&newForward, &rotation, &forward);
  
  // update camera's target
  cx_vec4_add (&camera->target, &camera->position, &newForward);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_rotate_around_point (camera_t *camera, const cx_vec4 * CX_RESTRICT point, float angle, const cx_vec4 * CX_RESTRICT axis)
{
  CX_ASSERT (camera);
  CX_ASSERT (point);
  CX_ASSERT (axis);
  
  // get rotation matrix
  cx_mat4x4 rotation;
  cx_mat4x4_rotation (&rotation, angle, axis->x, axis->y, axis->z);
  
  // get point's forward vector (to camera's position)
  cx_vec4 forward;
  cx_vec4_sub (&forward, &camera->position, point);
  
  // transform forward vector
  cx_vec4 newForward;
  cx_mat4x4_mul_vec4 (&newForward, &rotation, &forward);
  
  // update camera's position
  cx_vec4_add (&camera->position, point, &newForward);
  
  camera->target = *point;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_get_view_matrix (camera_t *camera, cx_mat4x4 *matrix)
{
  CX_ASSERT (camera);
  CX_ASSERT (matrix);
  
  cx_util_look_at (matrix, &camera->position, &camera->target, &camera->up);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void camera_get_projection_matrix (camera_t *camera, cx_mat4x4 *matrix)
{
  CX_ASSERT (camera);
  CX_ASSERT (matrix);
  
  CX_ASSERT (cx_validatef (camera->fov));
  CX_ASSERT (cx_validatef (camera->aspectRatio));
  
  cx_mat4x4_perspective (matrix, cx_rad (camera->fov), camera->aspectRatio, CAMERA_PERSPECTIVE_NEAR, CAMERA_PERSPECTIVE_FAR);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
void camera_look_at (camera_t *camera, const cx_vec4 *eye, const cx_vec4 *target, const cx_vec4 *updir)
{
  CX_ASSERT (camera);
  CX_ASSERT (eye);
  CX_ASSERT (target);
  CX_ASSERT (updir);
  
  CX_ASSERT (eye->w == 1.0f);
  CX_ASSERT (target->w == 1.0f);
  CX_ASSERT (updir->w == 0.0f);
  
  //
  // build rotation matrix (orthogonal)
  //
  
  cx_vec4 r, f, u;
  
  cx_vec4_sub (&f, target, eye);
  cx_vec4_normalize (&f);
  cx_vec4_cross (&r, &f, updir);
  cx_vec4_normalize (&r);
  cx_vec4_cross (&u, &r, &f);
  
  //
  // set view to world matrix
  //
  
  // inverse (transpose) of rotation matrix
  
  cx_vec4 e = *eye;
  cx_mat4x4 * CX_RESTRICT view = &camera->view;
  
  cx_vec4_negate (&f); // opengl (negate z);
  
  cx_mat4x4_set_row (view, 0, &r);
  cx_mat4x4_set_row (view, 1, &u);
  cx_mat4x4_set_row (view, 2, &f);
  
  cx_vec4 i;
  cx_vec4_set (&i, 0.0f, 0.0f, 0.0f, 1.0f);
  cx_mat4x4_set_row (view, 3, &i);

  // inverse of translation (eye)

  //cx_vec4_negate (&e); e.w = 1.0f;
  //cx_mat4x4_set_column (view, 3, &e);
  
  float ex = cx_vec4_dot (&r, &e);
  float ey = cx_vec4_dot (&u, &e);
  float ez = cx_vec4_dot (&f, &e);
  
  view->f16 [12] = -ex;
  view->f16 [13] = -ey;
  view->f16 [14] = -ez;
  view->f16 [15] = 1.0f;
}
*/

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

