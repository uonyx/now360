//
//  cx_utility.c
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "cx_utility.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


bool cx_util_is_power_of_2 (cxu32 n)
{
  /*
  int c = 0;
  
  if ((n & 1) == 0) // if even 
  {
    for (c = 0; n; n >>= 1)
    {
      c += (n & 1);
    }
  }
  
  return (c == 1);
  */
  
  return ((n & (n - 1)) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_world_space_to_screen_space (float width, float height, const cx_mat4x4 *proj, const cx_mat4x4 *view, 
                                          const cx_vec4 *world, cx_vec2 *screen, float *zScale)
{
  // transforms pos from world space to screen space
  
  CX_ASSERT (proj);
  CX_ASSERT (view);
  CX_ASSERT (screen);
  CX_ASSERT (world);
  CX_ASSERT (world->w == 1.0f);
  
  // transform to eye space
  cx_vec4 eye;
  cx_mat4x4_mul_vec4 (&eye, view, world);
  
  // transform to clip space
  cx_vec4 clip;
  cx_mat4x4_mul_vec4 (&clip, proj, &eye);
  
  // get screen coords
  cx_vec4_mul (&clip, (1.0f / clip.w), &clip);
  
  CX_ASSERT (cx_vec4_validate (&clip));
  
  screen->x = (clip.x + 1.0f) * 0.5f * width;
  screen->y = (-clip.y + 1.0f) * 0.5f * height; // y is inverted in screen coordinates
  
  if (zScale)
  {
#if 1
    float w = 1.0f;
    cx_vec4 eye0 = {{ (w * 0.5f), (w * 0.5f), eye.z, eye.w }};
    
    cx_vec4 proj0;
    cx_mat4x4_mul_vec4 (&proj0, proj, &eye0);
    
    float zs = proj0.x / proj0.w;
#else
    cx_vec4 eye0, proj0, proj1;
    
    cx_mat4x4_mul_vec4 (&eye0, view, world); // to eye space
    cx_mat4x4_mul_vec4 (&proj0, proj, &eye0); // to clip space
    
    eye0.x += 1.0f; // move eye
    
    cx_mat4x4_mul_vec4 (&proj1, proj, &eye0); // to clip space
    
    float zs = (proj1.x / proj1.w) - (proj0.x / proj0.w);
#endif
    
    *zScale = zs;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_screen_space_to_world_space (float width, float height, const cx_mat4x4 *proj, const cx_mat4x4 *view, 
                                           const cx_vec2 *screen, cx_vec4 *world, float clipz, bool ray)
{
  // to clip space
  cx_vec4 clip;
  clip.x = ((screen->x / (0.5f * width)) - 1.0f);
  clip.y = (-(screen->y / (0.5f * height)) + 1.0f);
  clip.z = (2.0f * clipz) - 1.0f;
  clip.w = 1.0f;
  
  CX_ASSERT (cx_vec4_validate (&clip));
  
  // to view (eye) space
  float a = proj->f16 [0]; 
  float b = proj->f16 [5]; 
  float c = proj->f16 [10];
  float d = proj->f16 [14];
  float e = proj->f16 [11];
  
  cx_vec4 eye;
  eye.x = clip.x / a;
  eye.y = clip.y / b;
  eye.z = clip.w / e;
  eye.w = (clip.z / d) - ((c * clip.w) / (d * e));
  
  CX_ASSERT (cx_vec4_validate (&eye));
  
  cx_vec4_mul (&eye, (1.0f / eye.w), &eye);
  
  CX_ASSERT (cx_vec4_validate (&eye));
  
  eye.w = ray ? 0.0f : eye.w;
  
  // to world space
  cx_mat3x3 rot;
  cx_mat4x4_get_mat3x3 (&rot, view);
  cx_mat3x3_transpose (&rot, &rot);

  cx_vec4 trans;
  cx_mat4x4_get_column (view, 3, &trans);
  cx_vec4_negate (&trans); trans.w = 1.0f;

  cx_mat4x4 viewInv;
  cx_mat4x4_zero (&viewInv);
  cx_mat4x4_set_mat3x3 (&viewInv, &rot);
  cx_mat4x4_set_column (&viewInv, 3, &trans);
  
  cx_mat4x4_mul_vec4 (world, &viewInv, &eye);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_look_at (cx_mat4x4 *m, const cx_vec4 * CX_RESTRICT eye, const cx_vec4 * CX_RESTRICT target, 
                      const cx_vec4 * CX_RESTRICT updir)
{
  CX_ASSERT (m);
  CX_ASSERT (eye);
  CX_ASSERT (target);
  CX_ASSERT (updir);
  
  CX_ASSERT (eye->w == 1.0f);
  CX_ASSERT (target->w == 1.0f);
  CX_ASSERT (updir->w == 0.0f);
  
  //////////////////////////////////
  // build rotation matrix
  //////////////////////////////////
  
  cx_vec4 side, up, forward;
  cx_vec4 i = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
  
  cx_vec4_sub (&forward, target, eye);
  cx_vec4_normalize (&forward);
  cx_vec4_cross (&side, &forward, updir);
  cx_vec4_normalize (&side);
  cx_vec4_cross (&up, &side, &forward);
  
  cx_vec4_negate (&forward); // opengl negate z;
  
  // build view to world matrix by transforming rotation and translation
  // inverting (transpose) rotation matrix
  cx_mat4x4_set_row (m, 0, &side);
  cx_mat4x4_set_row (m, 1, &up);
  cx_mat4x4_set_row (m, 2, &forward);
  cx_mat4x4_set_row (m, 3, &i);
  
  // transform translation 
  float ex = cx_vec4_dot (&side, eye);
  float ey = cx_vec4_dot (&up, eye);
  float ez = cx_vec4_dot (&forward, eye);
  cx_vec4 e = {{ -ex, -ey, -ez, 1.0f }};
  cx_mat4x4_set_column (m, 3, &e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
