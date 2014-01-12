//
//  cx_utility.c
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#include "cx_util.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu16 cx_util_byte_swap_u16 (cxu16 val)
{
  cxu16 swap = (cxu16) ((val >> 8) | (val << 8));
  
  return swap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_util_byte_swap_u32 (cxu32 val)
{
  cxu32 swap = (cxu32) (((val >> 24) & 0x000000ff) |
                        ((val >> 8)  & 0x0000ff00) |
                        ((val << 8)  & 0x00ff0000) |
                        ((val << 24) & 0xff000000));
  return swap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu64 cx_util_byte_swap_u64 (cxu64 val)
{
  cxu64 swap = (cxu64) (((val >> 56) & 0x00000000000000ff) |
                        ((val >> 40) & 0x000000000000ff00) |
                        ((val >> 24) & 0x0000000000ff0000) |
                        ((val >> 8)  & 0x00000000ff000000) |
                        ((val << 8)  & 0x000000ff00000000) |
                        ((val << 24) & 0x0000ff0000000000) |
                        ((val << 40) & 0x00ff000000000000) |
                        ((val << 56) & 0xff00000000000000));
  return swap;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu8* cx_util_byte_swap (cxu8 *bytes, cxu32 size)
{
  CX_ASSERT (bytes);
  CX_ASSERT (size > 0);
  
  cxu8 *b = bytes;
  cxu32 i = 0;
  cxu32 j = size - 1;
  
  while (i < j)
  {
    cxu8 tmp = b [i];
    b [i++] = b [j];
    b [j--] = tmp;
  }
  
  return b;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_util_is_pow2 (cxu32 n)
{
  return ((n & (n - 1)) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_util_roundup_int (cxf32 x)
{
  cxf32 f = x + 0.5f;
  cxi32 i = (cxi32) floor (f);
  
  return i;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_util_roundup_pow2 (cxu32 x)
{
  cxu32 r = x;
  
  --r;
  
  r |= r >> 1;
  r |= r >> 2;
  r |= r >> 4;
  r |= r >> 8;
  r |= r >> 16;
  
  return ++r;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_world_space_to_screen_space (cxf32 width, cxf32 height, const cx_mat4x4 *proj, const cx_mat4x4 *view, 
                                          const cx_vec4 *world, cx_vec2 *screen, cxf32 *depth, cxf32 *zScale)
{
  // transforms pos from world space to screen space
  
  CX_ASSERT (proj);
  CX_ASSERT (view);
  CX_ASSERT (screen);
  CX_ASSERT (depth);
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
  
  // clip.z should be in the range (-1, 1). if so, rescale to the range (0, 1) for depth value.
  
  *depth = (clip.z + 1.0f) * 0.5f;
  CX_REF_UNUSED (depth);
  
#if 0
  CX_LOG_CONSOLE (1, "-----");
  CX_LOG_CONSOLE (1, "clip.z = %.3f", clip.z);
  CX_LOG_CONSOLE (1, "depth  = %.3f", *depth);
#endif
  
  if (zScale) // 2D
  {
#if 1
    cxf32 w = 1.0f;
    cx_vec4 eye0 = {{ (w * 0.5f), (w * 0.5f), eye.z, eye.w }};
    
    cx_vec4 proj0;
    cx_mat4x4_mul_vec4 (&proj0, proj, &eye0);
    
    cxf32 zs = proj0.x / proj0.w;
#else
    cx_vec4 eye0, proj0, proj1;
    
    cx_mat4x4_mul_vec4 (&eye0, view, world); // to eye space
    cx_mat4x4_mul_vec4 (&proj0, proj, &eye0); // to clip space
    
    eye0.x += 1.0f; // move eye
    
    cx_mat4x4_mul_vec4 (&proj1, proj, &eye0); // to clip space
    
    cxf32 zs = (proj1.x / proj1.w) - (proj0.x / proj0.w);
#endif
    
    *zScale = zs;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_screen_space_to_world_space (cxf32 width, cxf32 height, const cx_mat4x4 *proj, const cx_mat4x4 *view, 
                                           const cx_vec2 *screen, cx_vec4 *world, cxf32 depth, bool ray)
{
  // to clip space
  cx_vec4 clip;
  clip.x = ((screen->x / (0.5f * width)) - 1.0f);
  clip.y = (-(screen->y / (0.5f * height)) + 1.0f);
  clip.z = (2.0f * depth) - 1.0f;
  clip.w = 1.0f;
  
  CX_ASSERT (cx_vec4_validate (&clip));
  
  // to view (eye) space
  cxf32 a = proj->f16 [0]; 
  cxf32 b = proj->f16 [5]; 
  cxf32 c = proj->f16 [10];
  cxf32 d = proj->f16 [14];
  cxf32 e = proj->f16 [11];
  
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
  CX_ASSERT (cx_is_zero (updir->w));
  
  // build rotation matrix (camera's transformation matrix)
  
  cx_vec4 side, up, forward;
  cx_vec4 i = {{ 0.0f, 0.0f, 0.0f, 1.0f }};
  
  cx_vec4_sub (&forward, target, eye);
  cx_vec4_normalize (&forward);
  cx_vec4_cross (&side, &forward, updir);
  cx_vec4_normalize (&side);
  cx_vec4_cross (&up, &side, &forward);
  
  cx_vec4_negate (&forward); // opengl negate z;
  
  // build view to world matrix by transforming rotation and translation
  
  // invert (transpose) rotation matrix
  cx_mat4x4_set_row (m, 0, &side);
  cx_mat4x4_set_row (m, 1, &up);
  cx_mat4x4_set_row (m, 2, &forward);
  cx_mat4x4_set_row (m, 3, &i);
  
  // transform translation 
  cxf32 ex = cx_vec4_dot (&side, eye);
  cxf32 ey = cx_vec4_dot (&up, eye);
  cxf32 ez = cx_vec4_dot (&forward, eye);
  cx_vec4 e = {{ -ex, -ey, -ez, 1.0f }};
  cx_mat4x4_set_column (m, 3, &e);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_util_word_filter (char *text, const char **words, cxu32 wordCount, char subchar)
{
  // brute-force search
  
  for (cxu32 i = 0; i < wordCount; ++i)
  {
    const char *word = words [i];
    CX_ASSERT (word);
    
    bool matchWord = false;
    
    if (word [0] == '@')
    {
      word++;
      matchWord = true;
    }
    
    cxu32 wlen = strlen (word);
    
    char *found = strcasestr (text, word);
    
    while (found)
    {
      bool replace = false;
      
      if (matchWord) // match word
      {
        cxu8 fca = (found == text) ? 0 : found [-1];
        cxu8 fcb = found [wlen];
        
        if ((fca <= 32) && (fcb <= 32))
        {
          replace = true;
        }
        else
        {
          found += wlen;
        }
      }
      else
      {
        replace = true; // contain world
      }
      
      if (replace)
      {
        while (wlen--)
        {
          cxu8 fc = *found;
          
          if (fc > 32) // ignore whitespace
          {
            *found++ = subchar;
          }
          else
          {
            found++;
          }
        }
      }
      
      found = strcasestr (found, word);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
