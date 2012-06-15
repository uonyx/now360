//
//  cx_vector4.h
//
//  Created by Ubaka Onyechi on 01/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_VECTOR4_H
#define CX_VECTOR4_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_math.h"
#include "cx_string.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CX_SIMD_NEON
#define CX_SIMD_VEC4_DECL   float32x4_t _q128;
#else
#define CX_SIMD_VEC4_DECL
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

union cx_vec2
{
  struct { cxf32 x, y; };
  cxf32 f2 [2];
};

union cx_vec4
{
  struct { cxf32 r, g, b, a; };
  struct { cxf32 x, y, z, w; };
  cxf32 f4 [4];
  CX_SIMD_VEC4_DECL
} CX_ALIGN(16);

typedef union cx_vec2 cx_vec2;
typedef union cx_vec4 cx_vec4;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_zero (cx_vec4 *v_out);
static CX_INLINE void cx_vec4_negate (cx_vec4 *v_out);
static CX_INLINE void cx_vec4_set (cx_vec4 * CX_RESTRICT v_out, cxf32 x, cxf32 y, cxf32 z, cxf32 w);
static CX_INLINE void cx_vec4_mul (cx_vec4 * CX_RESTRICT v_out, cxf32 s, const cx_vec4 * CX_RESTRICT v);
static CX_INLINE void cx_vec4_add (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1);
static CX_INLINE void cx_vec4_sub (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1);
static CX_INLINE void cx_vec4_cross (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1);
static CX_INLINE cxf32 cx_vec4_dot (const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1);
static CX_INLINE void cx_vec4_normalize (cx_vec4 *v);
static CX_INLINE cxf32 cx_vec4_length (const cx_vec4 * CX_RESTRICT v);
static CX_INLINE cxf32 cx_vec4_lengthSqr (const cx_vec4 * CX_RESTRICT v);
static CX_INLINE bool cx_vec4_validate (const cx_vec4 *v);
static CX_INLINE void cx_vec4_string (const cx_vec4 *v, char *destBuffer, cxu32 destbufferSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_zero (cx_vec4 *v_out)
{
  CX_ASSERT (cx_vec4_validate (v_out));
  
#ifdef CX_SIMD_NEON
  v_out->_q128 = vdupq_n_f32 (0.0f);
#else
  v_out->x = 0.0f;
  v_out->y = 0.0f;
  v_out->z = 0.0f;
  v_out->w = 0.0f;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_negate (cx_vec4 *v_out)
{
  CX_ASSERT (cx_vec4_validate (v_out));
  
#ifdef CX_SIMD_NEON
  v_out->_q128 = vnegq_f32 (v_out->_q128);
#else
  v_out->x = -v_out->x;
  v_out->y = -v_out->y;
  v_out->z = -v_out->z;
  v_out->w = -v_out->w;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_set (cx_vec4 * CX_RESTRICT v_out, cxf32 x, cxf32 y, cxf32 z, cxf32 w)
{
  CX_ASSERT (cx_validatef (x));
  CX_ASSERT (cx_validatef (y));
  CX_ASSERT (cx_validatef (z));
  CX_ASSERT (cx_validatef (w));
  
#ifdef CX_SIMD_NEON
  cxf32 a[4] CX_ALIGN(16) = { x, y, z, w };
  v_out->_q128 = vld1q_f32 (a);
#else
  v_out->x = x;
  v_out->y = y;
  v_out->z = z;
  v_out->w = w;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_mul (cx_vec4 * CX_RESTRICT v_out, cxf32 s, const cx_vec4 * CX_RESTRICT v)
{
  CX_ASSERT (cx_vec4_validate (v));
  CX_ASSERT (cx_validatef (s));
  
#ifdef CX_SIMD_NEON
  v_out->_q128 = vmulq_n_f32 (v->_q128, s);
#else
  v_out->x = v->x * s;
  v_out->y = v->y * s;
  v_out->z = v->z * s;
  v_out->w = v->w * s;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_add (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1)
{
  CX_ASSERT (cx_vec4_validate (v0));
  CX_ASSERT (cx_vec4_validate (v1));
  
#ifdef CX_SIMD_NEON
  v_out->_q128 = vaddq_f32 (v0->_q128, v1->_q128);
#else
  v_out->x = v0->x + v1->x;
  v_out->y = v0->y + v1->y;
  v_out->z = v0->z + v1->z;
  v_out->w = v0->w + v1->w;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_sub (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1)
{
  CX_ASSERT (cx_vec4_validate (v0));
  CX_ASSERT (cx_vec4_validate (v1));
  
#ifdef CX_SIMD_NEON
  v_out->_q128 = vsubq_f32 (v0->_q128, v1->_q128);
#else
  v_out->x = v0->x - v1->x;
  v_out->y = v0->y - v1->y;
  v_out->z = v0->z - v1->z;
  v_out->w = v0->w - v1->w;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_normalize (cx_vec4 *v)
{
  cxf32 len = cx_vec4_length (v);
  
  CX_ASSERT (cx_validatef (len));
  CX_ASSERT (len > CX_EPSILON);
  
  cx_vec4_mul (v, (1.0f / len), v);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_vec4_length (const cx_vec4 * CX_RESTRICT v)
{
  CX_ASSERT (cx_vec4_validate (v));
  
  cxf32 lsqr = cx_vec4_lengthSqr (v);
  
  cxf32 l = cx_sqrt (lsqr);
  
  return l;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_vec4_lengthSqr (const cx_vec4 * CX_RESTRICT v)
{
  CX_ASSERT (cx_vec4_validate (v));
  
#ifdef CX_SIMD_NEON
  float32x4_t v128 = vmulq_f32 (v->_q128, v->_q128);
  float32x2_t v64a = vget_low_f32 (v128);
  float32x2_t v64b = vget_high_f32 (v128);
  v64a = vadd_f32 (v64a, v64b);
  return vget_lane_f32 (v64a, 0) + vget_lane_f32 (v64a, 1);
#else
  return (v->x * v->x) + (v->y * v->y) + (v->z * v->z) + (v->w * v->w);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_cross (cx_vec4 * CX_RESTRICT v_out, const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1)
{
  CX_ASSERT (v_out);
  CX_ASSERT (cx_vec4_validate (v0));
  CX_ASSERT (cx_vec4_validate (v1));
  
  v_out->x = (v0->y * v1->z) - (v0->z * v1->y);
  v_out->y = (v0->z * v1->x) - (v0->x * v1->z);
  v_out->z = (v0->x * v1->y) - (v0->y * v1->x);
  v_out->w = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_vec4_dot (const cx_vec4 * CX_RESTRICT v0, const cx_vec4 * CX_RESTRICT v1)
{
  CX_ASSERT (cx_vec4_validate (v0));
  CX_ASSERT (cx_vec4_validate (v1));
  
#ifdef CX_SIMD_NEON
  float32x4_t v128 = vmulq_f32 (v0->_q128, v1->_q128);
  float32x2_t v64a = vget_low_f32 (v128);
  float32x2_t v64b = vget_high_f32 (v128);
  v64a = vadd_f32 (v64a, v64b);
  return vget_lane_f32 (v64a, 0) + vget_lane_f32 (v64a, 1);
  
#else
  
  return (v0->x * v1->x) + (v0->y * v1->y) + (v0->z * v1->z) + (v0->w * v1->w);
  
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_vec4_validate (const cx_vec4 *v)
{
  CX_ASSERT (v);
  
  bool valid;
  
  valid = cx_validatef (v->x);
  valid &= cx_validatef (v->y);
  valid &= cx_validatef (v->z);
  valid &= cx_validatef (v->w);
  
  return valid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_vec4_string (const cx_vec4 *v, char *destBuffer, cxu32 destbufferSize)
{
  CX_ASSERT (cx_vec4_validate (v));
  
  cx_sprintf (destBuffer, destbufferSize, "{ %.2f, %.2f, %.2f, %.2f }", v->x, v->y, v->z, v->w );
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
