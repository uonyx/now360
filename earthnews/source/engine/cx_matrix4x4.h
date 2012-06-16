//
//  cx_matrix4x4.h
//
//  Created by Ubaka Onyechi on 05/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_MATRIX4X4_H
#define CX_MATRIX4X4_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_math.h"
#include "cx_vector4.h"
#include "cx_matrix3x3.h"
#include "cx_string.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CX_SIMD_NEON
#define CX_SIMD_MAT4_DECL   float32x4x4_t _q128x4;
#else
#define CX_SIMD_MAT4_DECL
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_MATRIX4X4_INDEX(ROW, COL)     ((COL) + (4 * (ROW)))

union cx_mat4x4
{
  cxf32 f16 [16];
  CX_SIMD_MAT4_DECL
};

typedef union cx_mat4x4 cx_mat4x4;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_identity (cx_mat4x4 *m);
static CX_INLINE void cx_mat4x4_zero (cx_mat4x4 *m);
static CX_INLINE void cx_mat4x4_set (cx_mat4x4 *m, cxf32 f16 [16]);
static CX_INLINE void cx_mat4x4_set_mat3x3 (cx_mat4x4 *m44, const cx_mat3x3 *m33);
static CX_INLINE void cx_mat4x4_get_mat3x3 (cx_mat3x3 *m33, const cx_mat4x4 *m44);
static CX_INLINE void cx_mat4x4_transpose (cx_mat4x4 * CX_RESTRICT t, const cx_mat4x4 * CX_RESTRICT m);
static CX_INLINE cxf32 cx_mat4x4_inverse (cx_mat4x4 * CX_RESTRICT i, const cx_mat4x4 * CX_RESTRICT m);

static CX_INLINE void cx_mat4x4_set_column (cx_mat4x4 *m, cxi32 index, const cx_vec4 *col);
static CX_INLINE void cx_mat4x4_get_column (const cx_mat4x4 *m, cxi32 index, cx_vec4 *col);
static CX_INLINE void cx_mat4x4_set_row (cx_mat4x4 *m, cxi32 index, const cx_vec4 *row);
static CX_INLINE void cx_mat4x4_get_row (const cx_mat4x4 *m, cxi32 index, cx_vec4 *row);

static CX_INLINE void cx_mat4x4_add (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1);
static CX_INLINE void cx_mat4x4_sub (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1);
static CX_INLINE void cx_mat4x4_mul (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1);
static CX_INLINE void cx_mat4x4_mul_scalar (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m, cxf32 scalar);
static CX_INLINE void cx_mat4x4_mul_vec4 (cx_vec4 * CX_RESTRICT v_out, const cx_mat4x4 * CX_RESTRICT m, const cx_vec4 * CX_RESTRICT v);

static CX_INLINE void cx_mat4x4_scale (cx_mat4x4 *m, cxf32 x, cxf32 y, cxf32 z);
static CX_INLINE void cx_mat4x4_translation (cx_mat4x4 *m, cxf32 x, cxf32 y, cxf32 z);
static CX_INLINE void cx_mat4x4_rotation (cx_mat4x4 *m, cxf32 rad, cxf32 x, cxf32 y, cxf32 z);
static CX_INLINE void cx_mat4x4_rotation_axis_x (cx_mat4x4 *m, cxf32 rad);
static CX_INLINE void cx_mat4x4_rotation_axis_y (cx_mat4x4 *m, cxf32 rad);
static CX_INLINE void cx_mat4x4_rotation_axis_z (cx_mat4x4 *m, cxf32 rad);

static CX_INLINE void cx_mat4x4_perspective (cx_mat4x4 *m, cxf32 fov, cxf32 aspectRatio, cxf32 near, cxf32 far);
static CX_INLINE void cx_mat4x4_ortho (cx_mat4x4 *m, cxf32 left, cxf32 right, cxf32 top, cxf32 bottom, cxf32 near, cxf32 far);
static CX_INLINE bool cx_mat4x4_validate (const cx_mat4x4 *m);
static CX_INLINE void cx_mat4x4_string (char *destBuffer, cxu32 destbufferSize, const cx_mat4x4 *m);

// - determinant? projection (0) or reflection (<0)
// - isOrthogonal? each column must be a unit vec, each column vector must be mutually perpendicular

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_identity (cx_mat4x4 *m)
{
  CX_ASSERT (m);
  
  // column 1
  m->f16 [0] = 1.0f;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  // column 2
  m->f16 [4] = 0.0f;
  m->f16 [5] = 1.0f;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  // column 3
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = 1.0f;
  m->f16 [11] = 0.0f;
  // column 4
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_zero (cx_mat4x4 *m)
{
  CX_ASSERT (m);
  
#ifdef CX_SIMD_NEON
  m->_q128x4.val [0] = vdupq_n_f32 (0.0f);
  m->_q128x4.val [1] = vdupq_n_f32 (0.0f);
  m->_q128x4.val [2] = vdupq_n_f32 (0.0f);
  m->_q128x4.val [3] = vdupq_n_f32 (0.0f);
#else
  m->f16 [0] = 0.0f;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  m->f16 [4] = 0.0f;
  m->f16 [5] = 0.0f;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = 0.0f;
  m->f16 [11] = 0.0f;
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 0.0f;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_set (cx_mat4x4 *m, cxf32 f16 [16])
{
  CX_ASSERT (m);
  
#ifdef CX_SIMD_NEON
  m->_q128x4.val [0] = vld1q_f32 (f16);
  m->_q128x4.val [1] = vld1q_f32 (f16 + 4);
  m->_q128x4.val [2] = vld1q_f32 (f16 + 8);
  m->_q128x4.val [3] = vld1q_f32 (f16 + 12);
#else
  m->f16 [0] = f16 [0];
  m->f16 [1] = f16 [1];
  m->f16 [2] = f16 [2];
  m->f16 [3] = f16 [3];
  m->f16 [4] = f16 [4];
  m->f16 [5] = f16 [5];
  m->f16 [6] = f16 [6];
  m->f16 [7] = f16 [7];
  m->f16 [8] = f16 [8];
  m->f16 [9] = f16 [9];
  m->f16 [10] = f16 [10];
  m->f16 [11] = f16 [11];
  m->f16 [12] = f16 [12];
  m->f16 [13] = f16 [13];
  m->f16 [14] = f16 [14];
  m->f16 [15] = f16 [15];
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_transpose (cx_mat4x4 * CX_RESTRICT t, const cx_mat4x4 * CX_RESTRICT m)
{
  CX_ASSERT (m);
  CX_ASSERT (t);
  
#ifdef CX_SIMD_NEON
  t->_q128x4 = vld4q_f32 (m->f16);
#else
  cxf32 m0  = m->f16 [0];
  cxf32 m1  = m->f16 [1];
  cxf32 m2  = m->f16 [2];
  cxf32 m3  = m->f16 [3];
  cxf32 m4  = m->f16 [4];
  cxf32 m5  = m->f16 [5];
  cxf32 m6  = m->f16 [6];
  cxf32 m7  = m->f16 [7];
  cxf32 m8  = m->f16 [8];
  cxf32 m9  = m->f16 [9];
  cxf32 m10 = m->f16 [10];
  cxf32 m11 = m->f16 [11];
  cxf32 m12 = m->f16 [12];
  cxf32 m13 = m->f16 [13];
  cxf32 m14 = m->f16 [14];
  cxf32 m15 = m->f16 [15];
  
  t->f16 [0] = m0;
  t->f16 [1] = m4;
  t->f16 [2] = m8;
  t->f16 [3] = m12;
  t->f16 [4] = m1;
  t->f16 [5] = m5;
  t->f16 [6] = m9;
  t->f16 [7] = m13;
  t->f16 [8] = m2;
  t->f16 [9] = m6;
  t->f16 [10] = m10;
  t->f16 [11] = m14;
  t->f16 [12] = m3;
  t->f16 [13] = m7;
  t->f16 [14] = m11;
  t->f16 [15] = m15;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_set_mat3x3 (cx_mat4x4 *m44, const cx_mat3x3 *m33)
{
  CX_ASSERT (m33);
  CX_ASSERT (m44);
  
  m44->f16 [0] = m33->f9 [0];
  m44->f16 [1] = m33->f9 [1];
  m44->f16 [2] = m33->f9 [2];
  m44->f16 [4] = m33->f9 [3];
  m44->f16 [5] = m33->f9 [4];
  m44->f16 [6] = m33->f9 [5];
  m44->f16 [8] = m33->f9 [6];
  m44->f16 [9] = m33->f9 [7];
  m44->f16 [10] = m33->f9 [8];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_get_mat3x3 (cx_mat3x3 *m33, const cx_mat4x4 *m44)
{
  CX_ASSERT (m33);
  CX_ASSERT (m44);
  
  m33->f9 [0] = m44->f16 [0];
  m33->f9 [1] = m44->f16 [1];
  m33->f9 [2] = m44->f16 [2];
  m33->f9 [3] = m44->f16 [4];
  m33->f9 [4] = m44->f16 [5];
  m33->f9 [5] = m44->f16 [6];
  m33->f9 [6] = m44->f16 [8];
  m33->f9 [7] = m44->f16 [9];
  m33->f9 [8] = m44->f16 [10];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_set_column (cx_mat4x4 *m, cxi32 index, const cx_vec4 *col)
{
  CX_ASSERT (m);
  CX_ASSERT (col);
  CX_ASSERT ((index >= 0) && (index <= 3));
  
#ifdef CX_SIMD_NEON
  m->_q128x4.val [index] = col->_q128;
#else
  int i = index * 4;
  m->f16 [i + 0] = col->x;
  m->f16 [i + 1] = col->y;
  m->f16 [i + 2] = col->z;
  m->f16 [i + 3] = col->w;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_get_column (const cx_mat4x4 *m, cxi32 index, cx_vec4 *col)
{
  CX_ASSERT (m);
  CX_ASSERT (col);
  CX_ASSERT ((index >= 0) && (index <= 3));
  
#ifdef CX_SIMD_NEON
  col->_q128 = m->_q128x4.val [index];
#else
  int i = index * 4;
  col->x = m->f16 [i + 0];
  col->y = m->f16 [i + 1];
  col->z = m->f16 [i + 2];
  col->w = m->f16 [i + 3];
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_set_row (cx_mat4x4 *m, cxi32 index, const cx_vec4 *row)
{
  CX_ASSERT (m);
  CX_ASSERT (row);
  CX_ASSERT ((index >= 0) && (index <= 3));
  
  m->f16 [index + 0] = row->x;
  m->f16 [index + 4] = row->y;
  m->f16 [index + 8] = row->z;
  m->f16 [index + 12] = row->w;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_get_row (const cx_mat4x4 *m, cxi32 index, cx_vec4 *row)
{
  CX_ASSERT (m);
  CX_ASSERT (row);
  CX_ASSERT ((index >= 0) && (index <= 3));

  row->x = m->f16 [index + 0];
  row->y = m->f16 [index + 4];
  row->z = m->f16 [index + 8];
  row->w = m->f16 [index + 12];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_add (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1)
{
  CX_ASSERT (m_out);
  CX_ASSERT (m0);
  CX_ASSERT (m1);
  
#ifdef CX_SIMD_NEON
  m_out->_q128x4.val [0] = vaddq_f32 (m0->_q128x4.val [0], m1->_q128x4.val [0]);
  m_out->_q128x4.val [1] = vaddq_f32 (m0->_q128x4.val [1], m1->_q128x4.val [1]);
  m_out->_q128x4.val [2] = vaddq_f32 (m0->_q128x4.val [2], m1->_q128x4.val [2]);
  m_out->_q128x4.val [3] = vaddq_f32 (m0->_q128x4.val [3], m1->_q128x4.val [3]);
#else
  cxf32 m00 = m0->f16 [0];
  cxf32 m01 = m0->f16 [1];
  cxf32 m02 = m0->f16 [2];
  cxf32 m03 = m0->f16 [3];
  cxf32 m04 = m0->f16 [4];
  cxf32 m05 = m0->f16 [5];
  cxf32 m06 = m0->f16 [6];
  cxf32 m07 = m0->f16 [7];
  cxf32 m08 = m0->f16 [8];
  cxf32 m09 = m0->f16 [9];
  cxf32 m010 = m0->f16 [10];
  cxf32 m011 = m0->f16 [11];
  cxf32 m012 = m0->f16 [12];
  cxf32 m013 = m0->f16 [13];
  cxf32 m014 = m0->f16 [14];
  cxf32 m015 = m0->f16 [15];
  
  cxf32 m10 = m1->f16 [0];
  cxf32 m11 = m1->f16 [1];
  cxf32 m12 = m1->f16 [2];
  cxf32 m13 = m1->f16 [3];
  cxf32 m14 = m1->f16 [4];
  cxf32 m15 = m1->f16 [5];
  cxf32 m16 = m1->f16 [6];
  cxf32 m17 = m1->f16 [7];
  cxf32 m18 = m1->f16 [8];
  cxf32 m19 = m1->f16 [9];
  cxf32 m110 = m1->f16 [10];
  cxf32 m111 = m1->f16 [11];
  cxf32 m112 = m1->f16 [12];
  cxf32 m113 = m1->f16 [13];
  cxf32 m114 = m1->f16 [14];
  cxf32 m115 = m1->f16 [15];
  
  m_out->f16 [0]  = m00 + m10;
  m_out->f16 [1]  = m01 + m11;
  m_out->f16 [2]  = m02 + m12;
  m_out->f16 [3]  = m03 + m13;
  m_out->f16 [4]  = m04 + m14;
  m_out->f16 [5]  = m05 + m15;
  m_out->f16 [6]  = m06 + m16;
  m_out->f16 [7]  = m07 + m17;
  m_out->f16 [8]  = m08 + m18;
  m_out->f16 [9]  = m09 + m19;
  m_out->f16 [10] = m010 + m110;
  m_out->f16 [11] = m011 + m111;
  m_out->f16 [12] = m012 + m112;
  m_out->f16 [13] = m013 + m113;
  m_out->f16 [14] = m014 + m114;
  m_out->f16 [15] = m015 + m115;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_sub (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1)
{
  CX_ASSERT (m_out);
  CX_ASSERT (m0);
  CX_ASSERT (m1);
  
#ifdef CX_SIMD_NEON
  m_out->_q128x4.val [0] = vsubq_f32 (m0->_q128x4.val [0], m1->_q128x4.val [0]);
  m_out->_q128x4.val [1] = vsubq_f32 (m0->_q128x4.val [1], m1->_q128x4.val [1]);
  m_out->_q128x4.val [2] = vsubq_f32 (m0->_q128x4.val [2], m1->_q128x4.val [2]);
  m_out->_q128x4.val [3] = vsubq_f32 (m0->_q128x4.val [3], m1->_q128x4.val [3]);
#else
  cxf32 m00 = m0->f16 [0];
  cxf32 m01 = m0->f16 [1];
  cxf32 m02 = m0->f16 [2];
  cxf32 m03 = m0->f16 [3];
  cxf32 m04 = m0->f16 [4];
  cxf32 m05 = m0->f16 [5];
  cxf32 m06 = m0->f16 [6];
  cxf32 m07 = m0->f16 [7];
  cxf32 m08 = m0->f16 [8];
  cxf32 m09 = m0->f16 [9];
  cxf32 m010 = m0->f16 [10];
  cxf32 m011 = m0->f16 [11];
  cxf32 m012 = m0->f16 [12];
  cxf32 m013 = m0->f16 [13];
  cxf32 m014 = m0->f16 [14];
  cxf32 m015 = m0->f16 [15];
  
  cxf32 m10 = m1->f16 [0];
  cxf32 m11 = m1->f16 [1];
  cxf32 m12 = m1->f16 [2];
  cxf32 m13 = m1->f16 [3];
  cxf32 m14 = m1->f16 [4];
  cxf32 m15 = m1->f16 [5];
  cxf32 m16 = m1->f16 [6];
  cxf32 m17 = m1->f16 [7];
  cxf32 m18 = m1->f16 [8];
  cxf32 m19 = m1->f16 [9];
  cxf32 m110 = m1->f16 [10];
  cxf32 m111 = m1->f16 [11];
  cxf32 m112 = m1->f16 [12];
  cxf32 m113 = m1->f16 [13];
  cxf32 m114 = m1->f16 [14];
  cxf32 m115 = m1->f16 [15];

  m_out->f16 [0]  = m00 - m10;
  m_out->f16 [1]  = m01 - m11;
  m_out->f16 [2]  = m02 - m12;
  m_out->f16 [3]  = m03 - m13;
  m_out->f16 [4]  = m04 - m14;
  m_out->f16 [5]  = m05 - m15;
  m_out->f16 [6]  = m06 - m16;
  m_out->f16 [7]  = m07 - m17;
  m_out->f16 [8]  = m08 - m18;
  m_out->f16 [9]  = m09 - m19;
  m_out->f16 [10] = m010 - m110;
  m_out->f16 [11] = m011 - m111;
  m_out->f16 [12] = m012 - m112;
  m_out->f16 [13] = m013 - m113;
  m_out->f16 [14] = m014 - m114;
  m_out->f16 [15] = m015 - m115;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_mul (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m0, const cx_mat4x4 * CX_RESTRICT m1)
{
  CX_ASSERT (m_out);
  CX_ASSERT (m0);
  CX_ASSERT (m1);
  
#ifdef CX_SIMD_NEON
  
#if 0
  m_out->_q128x4.val [0] = vmulq_n_f32 (m0->_q128x4.val [0], vgetq_lane_f32 (m1->_q128x4.val [0], 0));
  m_out->_q128x4.val [1] = vmulq_n_f32 (m0->_q128x4.val [0], vgetq_lane_f32 (m1->_q128x4.val [1], 0));
  m_out->_q128x4.val [2] = vmulq_n_f32 (m0->_q128x4.val [0], vgetq_lane_f32 (m1->_q128x4.val [2], 0));
  m_out->_q128x4.val [3] = vmulq_n_f32 (m0->_q128x4.val [0], vgetq_lane_f32 (m1->_q128x4.val [3], 0));
  
  m_out->_q128x4.val [0] = vmlaq_n_f32 (m_out->_q128x4.val [0], m0->_q128x4.val [1], vgetq_lane_f32 (m1->_q128x4.val [0], 1));
  m_out->_q128x4.val [1] = vmlaq_n_f32 (m_out->_q128x4.val [1], m0->_q128x4.val [1], vgetq_lane_f32 (m1->_q128x4.val [1], 1));
  m_out->_q128x4.val [2] = vmlaq_n_f32 (m_out->_q128x4.val [2], m0->_q128x4.val [1], vgetq_lane_f32 (m1->_q128x4.val [2], 1));
  m_out->_q128x4.val [3] = vmlaq_n_f32 (m_out->_q128x4.val [3], m0->_q128x4.val [1], vgetq_lane_f32 (m1->_q128x4.val [3], 1));
  
  m_out->_q128x4.val [0] = vmlaq_n_f32 (m_out->_q128x4.val [0], m0->_q128x4.val [2], vgetq_lane_f32 (m1->_q128x4.val [0], 2));
  m_out->_q128x4.val [1] = vmlaq_n_f32 (m_out->_q128x4.val [1], m0->_q128x4.val [2], vgetq_lane_f32 (m1->_q128x4.val [1], 2));
  m_out->_q128x4.val [2] = vmlaq_n_f32 (m_out->_q128x4.val [2], m0->_q128x4.val [2], vgetq_lane_f32 (m1->_q128x4.val [2], 2));
  m_out->_q128x4.val [3] = vmlaq_n_f32 (m_out->_q128x4.val [3], m0->_q128x4.val [2], vgetq_lane_f32 (m1->_q128x4.val [3], 2));
  
  m_out->_q128x4.val [0] = vmlaq_n_f32 (m_out->_q128x4.val [0], m0->_q128x4.val [3], vgetq_lane_f32 (m1->_q128x4.val [0], 3));
  m_out->_q128x4.val [1] = vmlaq_n_f32 (m_out->_q128x4.val [1], m0->_q128x4.val [3], vgetq_lane_f32 (m1->_q128x4.val [1], 3));
  m_out->_q128x4.val [2] = vmlaq_n_f32 (m_out->_q128x4.val [2], m0->_q128x4.val [3], vgetq_lane_f32 (m1->_q128x4.val [2], 3));
  m_out->_q128x4.val [3] = vmlaq_n_f32 (m_out->_q128x4.val [3], m0->_q128x4.val [3], vgetq_lane_f32 (m1->_q128x4.val [3], 3));
#else
  cxf32 m10 = m1->f16 [0];
  cxf32 m11 = m1->f16 [1];
  cxf32 m12 = m1->f16 [2];
  cxf32 m13 = m1->f16 [3];
  cxf32 m14 = m1->f16 [4];
  cxf32 m15 = m1->f16 [5];
  cxf32 m16 = m1->f16 [6];
  cxf32 m17 = m1->f16 [7];
  cxf32 m18 = m1->f16 [8];
  cxf32 m19 = m1->f16 [9];
  cxf32 m110 = m1->f16 [10];
  cxf32 m111 = m1->f16 [11];
  cxf32 m112 = m1->f16 [12];
  cxf32 m113 = m1->f16 [13];
  cxf32 m114 = m1->f16 [14];
  cxf32 m115 = m1->f16 [15];
  
  float32x4_t _q128m00 = m0->_q128x4.val [0];
  float32x4_t _q128m01 = m0->_q128x4.val [1];
  float32x4_t _q128m02 = m0->_q128x4.val [2];
  float32x4_t _q128m03 = m0->_q128x4.val [3];
  
  float32x4_t _q128x40, _q128x41, _q128x42, _q128x43;
  
  _q128x40 = vmulq_n_f32 (_q128m00, m10);
  _q128x41 = vmulq_n_f32 (_q128m00, m14);
  _q128x42 = vmulq_n_f32 (_q128m00, m18);
  _q128x43 = vmulq_n_f32 (_q128m00, m112);
  
  _q128x40 = vmlaq_n_f32 (_q128x40, _q128m01, m11);
  _q128x41 = vmlaq_n_f32 (_q128x41, _q128m01, m15);
  _q128x42 = vmlaq_n_f32 (_q128x42, _q128m01, m19);
  _q128x43 = vmlaq_n_f32 (_q128x43, _q128m01, m113);
  
  _q128x40 = vmlaq_n_f32 (_q128x40, _q128m02, m12);
  _q128x41 = vmlaq_n_f32 (_q128x41, _q128m02, m16);
  _q128x42 = vmlaq_n_f32 (_q128x42, _q128m02, m110);
  _q128x43 = vmlaq_n_f32 (_q128x43, _q128m02, m114);
  
  _q128x40 = vmlaq_n_f32 (_q128x40, _q128m03, m13);
  _q128x41 = vmlaq_n_f32 (_q128x41, _q128m03, m17);
  _q128x42 = vmlaq_n_f32 (_q128x42, _q128m03, m111);
  _q128x43 = vmlaq_n_f32 (_q128x43, _q128m03, m115);
  
  m_out->_q128x4.val [0] = _q128x40;
  m_out->_q128x4.val [1] = _q128x41;
  m_out->_q128x4.val [2] = _q128x42;
  m_out->_q128x4.val [3] = _q128x43;
#endif
  
#else
  /* consume inputs - helps prevent against aliasing */
  
  cxf32 m00 = m0->f16 [0];
  cxf32 m01 = m0->f16 [1];
  cxf32 m02 = m0->f16 [2];
  cxf32 m03 = m0->f16 [3];
  cxf32 m04 = m0->f16 [4];
  cxf32 m05 = m0->f16 [5];
  cxf32 m06 = m0->f16 [6];
  cxf32 m07 = m0->f16 [7];
  cxf32 m08 = m0->f16 [8];
  cxf32 m09 = m0->f16 [9];
  cxf32 m010 = m0->f16 [10];
  cxf32 m011 = m0->f16 [11];
  cxf32 m012 = m0->f16 [12];
  cxf32 m013 = m0->f16 [13];
  cxf32 m014 = m0->f16 [14];
  cxf32 m015 = m0->f16 [15];
  
  cxf32 m10 = m1->f16 [0];
  cxf32 m11 = m1->f16 [1];
  cxf32 m12 = m1->f16 [2];
  cxf32 m13 = m1->f16 [3];
  cxf32 m14 = m1->f16 [4];
  cxf32 m15 = m1->f16 [5];
  cxf32 m16 = m1->f16 [6];
  cxf32 m17 = m1->f16 [7];
  cxf32 m18 = m1->f16 [8];
  cxf32 m19 = m1->f16 [9];
  cxf32 m110 = m1->f16 [10];
  cxf32 m111 = m1->f16 [11];
  cxf32 m112 = m1->f16 [12];
  cxf32 m113 = m1->f16 [13];
  cxf32 m114 = m1->f16 [14];
  cxf32 m115 = m1->f16 [15];
  
  /* do multiplication */
  
  m_out->f16 [0]  = (m00 * m10) + (m04 * m11) + (m08 * m12) + (m012 * m13);
  m_out->f16 [4]  = (m00 * m14) + (m04 * m15) + (m08 * m16) + (m012 * m17);
  m_out->f16 [8]  = (m00 * m18) + (m04 * m19) + (m08 * m110) + (m012 * m111);
  m_out->f16 [12] = (m00 * m112) + (m04 * m113) + (m08 * m114) + (m012 * m115);
  
  m_out->f16 [1]  = (m01 * m10) + (m05 * m11) + (m09 * m12) + (m013 * m13);
  m_out->f16 [5]  = (m01 * m14) + (m05 * m15) + (m09 * m16) + (m013 * m17);
  m_out->f16 [9]  = (m01 * m18) + (m05 * m19) + (m09 * m110) + (m013 * m111);
  m_out->f16 [13] = (m01 * m112) + (m05 * m113) + (m09 * m114) + (m013 * m115);
  
  m_out->f16 [2]  = (m02 * m10) + (m06 * m11) + (m010 * m12) + (m014 * m13);
  m_out->f16 [6]  = (m02 * m14) + (m06 * m15) + (m010 * m16) + (m014 * m17);
  m_out->f16 [10] = (m02 * m18) + (m06 * m19) + (m010 * m110) + (m014 * m111);
  m_out->f16 [14] = (m02 * m112) + (m06 * m113) + (m010 * m114) + (m014 * m115);
  
  m_out->f16 [3]  = (m03 * m10) + (m07 * m11) + (m011 * m12) + (m015 * m13);
  m_out->f16 [7]  = (m03 * m14) + (m07 * m15) + (m011 * m16) + (m015 * m17);
  m_out->f16 [11] = (m03 * m18) + (m07 * m19) + (m011 * m110) + (m015 * m111);
  m_out->f16 [15] = (m03 * m112) + (m07 * m113) + (m011 * m114) + (m015 * m115);
  
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_mul_scalar (cx_mat4x4 * CX_RESTRICT m_out, const cx_mat4x4 * CX_RESTRICT m, cxf32 scalar)
{
  CX_ASSERT (m_out);
  CX_ASSERT (m);
  
#ifdef CX_SIMD_NEON
  m_out->_q128x4.val [0] = vmulq_n_f32 (m->_q128x4.val [0], scalar);
  m_out->_q128x4.val [1] = vmulq_n_f32 (m->_q128x4.val [1], scalar);
  m_out->_q128x4.val [2] = vmulq_n_f32 (m->_q128x4.val [2], scalar);
  m_out->_q128x4.val [3] = vmulq_n_f32 (m->_q128x4.val [3], scalar);
#else
  m_out->f16 [0]  = m->f16 [0] * scalar;
  m_out->f16 [1]  = m->f16 [1] * scalar;
  m_out->f16 [2]  = m->f16 [2] * scalar;
  m_out->f16 [3]  = m->f16 [3] * scalar;
  m_out->f16 [4]  = m->f16 [4] * scalar;
  m_out->f16 [5]  = m->f16 [5] * scalar;
  m_out->f16 [6]  = m->f16 [6] * scalar;
  m_out->f16 [7]  = m->f16 [7] * scalar;
  m_out->f16 [8]  = m->f16 [8] * scalar;
  m_out->f16 [9]  = m->f16 [9] * scalar;
  m_out->f16 [10] = m->f16 [10] * scalar;
  m_out->f16 [11] = m->f16 [11] * scalar;
  m_out->f16 [12] = m->f16 [12] * scalar;
  m_out->f16 [13] = m->f16 [13] * scalar;
  m_out->f16 [14] = m->f16 [14] * scalar;
  m_out->f16 [15] = m->f16 [15] * scalar;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_mul_vec4 (cx_vec4 * CX_RESTRICT v_out, const cx_mat4x4 * CX_RESTRICT m, const cx_vec4 * CX_RESTRICT v)
{
  CX_ASSERT (v_out);
  CX_ASSERT (m);
  CX_ASSERT (v);
  
  cxf32 vx = v->x;
  cxf32 vy = v->y;
  cxf32 vz = v->z;
  cxf32 vw = v->w;

#ifdef CX_SIMD_NEON
  float32x4_t m4x40, m4x41, m4x42, m4x43;
  
  m4x40 = vmulq_n_f32 (m->_q128x4.val [0], vx);
  m4x41 = vmulq_n_f32 (m->_q128x4.val [1], vy);
  m4x42 = vmulq_n_f32 (m->_q128x4.val [2], vz);
  m4x43 = vmulq_n_f32 (m->_q128x4.val [3], vw);

  v_out->_q128 = vaddq_f32 (vaddq_f32 (m4x40, m4x41), vaddq_f32 (m4x42, m4x43));
#else
  cxf32 m00 = m->f16 [0];
  cxf32 m01 = m->f16 [1];
  cxf32 m02 = m->f16 [2];
  cxf32 m03 = m->f16 [3];
  cxf32 m04 = m->f16 [4];
  cxf32 m05 = m->f16 [5];
  cxf32 m06 = m->f16 [6];
  cxf32 m07 = m->f16 [7];
  cxf32 m08 = m->f16 [8];
  cxf32 m09 = m->f16 [9];
  cxf32 m10 = m->f16 [10];
  cxf32 m11 = m->f16 [11];
  cxf32 m12 = m->f16 [12];
  cxf32 m13 = m->f16 [13];
  cxf32 m14 = m->f16 [14];
  cxf32 m15 = m->f16 [15];
  
  v_out->x = (m00 * vx) + (m04 * vy) + (m08 * vz) + (m12 * vw);
  v_out->y = (m01 * vx) + (m05 * vy) + (m09 * vz) + (m13 * vw);
  v_out->z = (m02 * vx) + (m06 * vy) + (m10 * vz) + (m14 * vw);
  v_out->w = (m03 * vx) + (m07 * vy) + (m11 * vz) + (m15 * vw);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_scale (cx_mat4x4 *m, cxf32 x, cxf32 y, cxf32 z)
{
  CX_ASSERT (m);
  
  cx_mat4x4_identity (m);
  
  m->f16 [0] = x;
  m->f16 [5] = y;
  m->f16 [10] = z;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_translation (cx_mat4x4 *m, cxf32 x, cxf32 y, cxf32 z)
{
  CX_ASSERT (m);
  
  m->f16 [0] = 1.0f;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  m->f16 [4] = 0.0f;
  m->f16 [5] = 1.0f;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = 1.0f;
  m->f16 [11] = 0.0f;
  m->f16 [12] = x;
  m->f16 [13] = y;
  m->f16 [14] = z;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_rotation (cx_mat4x4 *m, cxf32 rad, cxf32 x, cxf32 y, cxf32 z)
{
  CX_ASSERT (m);
  
  cxf32 sin = cx_sin (rad);
  cxf32 cos = cx_cos (rad);
  cxf32 onesubcos = 1.0f - cos;
  
  m->f16 [0]  = (x * x * onesubcos) + cos;
  m->f16 [1]  = (x * y * onesubcos) + (z * sin);
  m->f16 [2]  = (x * z * onesubcos) - (y * sin);
  m->f16 [3]  = 0.0f;
  m->f16 [4]  = (x * y * onesubcos) - (z * sin);
  m->f16 [5]  = (y * y * onesubcos) + cos;
  m->f16 [6]  = (y * z * onesubcos) + (x * sin);
  m->f16 [7]  = 0.0f;
  m->f16 [8]  = (x * z * onesubcos) + (y * sin);
  m->f16 [9]  = (y * z * onesubcos) - (x * sin);
  m->f16 [10] = (z * z * onesubcos) + cos;
  m->f16 [11] = 0.0f;
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_rotation_axis_x (cx_mat4x4 *m, cxf32 rad)
{
  CX_ASSERT (m);
  
  cxf32 sin = cx_sin (rad);
  cxf32 cos = cx_cos (rad);
  
  // column 1
  m->f16 [0] = 1.0f;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  // column 2
  m->f16 [4] = 0.0f;
  m->f16 [5] = cos;
  m->f16 [6] = sin;
  m->f16 [7] = 0.0f;
  // column 3
  m->f16 [8] = 0.0f;
  m->f16 [9] = -sin;
  m->f16 [10] = cos;
  m->f16 [11] = 0.0f;
  // column 4
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_rotation_axis_y (cx_mat4x4 *m, cxf32 rad)
{
  CX_ASSERT (m);
  
  cxf32 sin = cx_sin (rad);
  cxf32 cos = cx_cos (rad);
  
  // column 1
  m->f16 [0] = cos;
  m->f16 [1] = 0.0f;
  m->f16 [2] = -sin;
  m->f16 [3] = 0.0f;
  // column 2
  m->f16 [4] = 0.0f;
  m->f16 [5] = 1.0f;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  // column 3
  m->f16 [8] = sin;
  m->f16 [9] = 0.0f;
  m->f16 [10] = cos;
  m->f16 [11] = 0.0f;
  // column 4
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_rotation_axis_z (cx_mat4x4 *m, cxf32 rad)
{
  CX_ASSERT (m);
  
  cxf32 sin = cx_sin (rad);
  cxf32 cos = cx_cos (rad);
  
  // column 1
  m->f16 [0] = cos;
  m->f16 [1] = sin;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  // column 2
  m->f16 [4] = -sin;
  m->f16 [5] = cos;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  // column 3
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = 1.0f;
  m->f16 [11] = 0.0f;
  // column 4
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = 0.0f;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_perspective (cx_mat4x4 *m, cxf32 fov, cxf32 aspectRatio, cxf32 near, cxf32 far)
{
  CX_ASSERT (m);
  
  cxf32 d = 1.0f / cx_tan (fov * 0.5f);
  cxf32 nsf = near - far;
  
  m->f16 [0] = d / aspectRatio;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  
  m->f16 [4] = 0.0f;
  m->f16 [5] = d;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = (near + far) / nsf;
  m->f16 [11] = -1.0f;
  
  m->f16 [12] = 0.0f;
  m->f16 [13] = 0.0f;
  m->f16 [14] = (2.0f * near * far) / nsf;
  m->f16 [15] = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_ortho (cx_mat4x4 *m, cxf32 left, cxf32 right, cxf32 top, cxf32 bottom, cxf32 near, cxf32 far)
{
  CX_ASSERT (m);
  
  cxf32 rsl = right - left;
  cxf32 tsb = top - bottom;
  cxf32 fsn = far - near;
  cxf32 ral = right + left;
  cxf32 tab = top + bottom;
  cxf32 fan = far + near;
  
  m->f16 [0] = 2.0f / rsl;
  m->f16 [1] = 0.0f;
  m->f16 [2] = 0.0f;
  m->f16 [3] = 0.0f;
  
  m->f16 [4] = 0.0f;
  m->f16 [5] = 2.0f / tsb;
  m->f16 [6] = 0.0f;
  m->f16 [7] = 0.0f;
  
  m->f16 [8] = 0.0f;
  m->f16 [9] = 0.0f;
  m->f16 [10] = -2.0f / fsn;
  m->f16 [11] = 0.0f;
  
  m->f16 [12] = -ral / rsl;
  m->f16 [13] = -tab / tsb;
  m->f16 [14] = -fan / fsn;
  m->f16 [15] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_mat4x4_inverse (cx_mat4x4 * CX_RESTRICT i, const cx_mat4x4 * CX_RESTRICT m)
{
  cxf32 m0 = m->f16 [0];
  cxf32 m1 = m->f16 [1];
  cxf32 m2 = m->f16 [2];
  cxf32 m3 = m->f16 [3];
  cxf32 m4 = m->f16 [4];
  cxf32 m5 = m->f16 [5];
  cxf32 m6 = m->f16 [6];
  cxf32 m7 = m->f16 [7];
  cxf32 m8 = m->f16 [8];
  cxf32 m9 = m->f16 [9];
  cxf32 m10 = m->f16 [10];
  cxf32 m11 = m->f16 [11];
  cxf32 m12 = m->f16 [12];
  cxf32 m13 = m->f16 [13];
  cxf32 m14 = m->f16 [14];
  cxf32 m15 = m->f16 [15];
  
  i->f16 [0] =  (m5 * m10 * m15) - (m5 * m11 * m14) - 
                (m9 * m6 * m15) + (m9 * m7 * m14) + 
                (m13 * m6 * m11) - (m13 * m7 * m10);
  i->f16 [4] =  (-m4 * m10 * m15) + (m4 * m11 * m14) + 
                (m8 * m6 * m15) - (m8 * m7 * m14) - 
                (m12 * m6 * m11) + (m12 * m7 * m10);
  i->f16 [8] =  (m4 * m9 * m15) - (m4 * m11 * m13) - 
                (m8 * m5 * m15) + (m8 * m7 * m13) + 
                (m12 * m5 * m11) - (m12 * m7 * m9);
  i->f16 [12] = (-m4 * m9 * m14) + (m4 * m10 * m13) + 
                (m8 * m5 * m14) - (m8 * m6 * m13) - 
                (m12 * m5 * m10) + (m12 * m6 * m9);
  i->f16 [1] =  (-m1 * m10 * m15) + (m1 * m11 * m14) + 
                (m9 * m2 * m15) - (m9 * m3 * m14) - 
                (m13 * m2 * m11) + (m13 * m3 * m10);
  i->f16 [5] =  (m0 * m10 * m15) - (m0 * m11 * m14) -
                (m8 * m2 * m15) + (m8 * m3 * m14) +
                (m12 * m2 * m11) - (m12 * m3 * m10);
  i->f16 [9] =  (-m0 * m9 * m15) + (m0 * m11 * m13) +
                (m8 * m1 * m15) - (m8 * m3 * m13) -
                (m12 * m1 * m11) + (m12 * m3 * m9);
  i->f16 [13] = (m0 * m9 * m14) - (m0 * m10 * m13) -
                (m8 * m1 * m14) + (m8 * m2 * m13) +
                (m12 * m1 * m10) - (m12 * m2 * m9);
  i->f16 [2] =  (m1 * m6 * m15) - (m1 * m7 * m14) -
                (m5 * m2 * m15) + (m5 * m3 * m14) +
                (m13 * m2 * m7) - (m13 * m3 * m6);
  i->f16 [6] =  (-m0 * m6 * m15) + (m0 * m7 * m14) +
                (m4 * m2 * m15) - (m4 * m3 * m14) -
                (m12 * m2 * m7) + (m12 * m3 * m6);
  i->f16 [10] = (m0 * m5 * m15) - (m0 * m7 * m13) -
                (m4 * m1 * m15) + (m4 * m3 * m13) +
                (m12 * m1 * m7) - (m12 * m3 * m5);
  i->f16 [14] = (-m0 * m5 * m14) + (m0 * m6 * m13) +
                (m4 * m1 * m14) - (m4 * m2 * m13) -
                (m12 * m1 * m6) + (m12 * m2 * m5);
  i->f16 [3] =  (-m1 * m6 * m11) + (m1 * m7 * m10) +
                (m4 * m2 * m11) - (m4 * m3 * m10) -
                (m8 * m2 * m7) + (m8 * m3 * m6);
  i->f16 [7] =  (m0 * m6 * m11) - (m0 * m7 * m10) -
                (m4 * m2 * m11) + (m4 * m3 * m10) +
                (m8 * m2 * m7) - (m8 * m3 * m6);
  i->f16 [11] = (-m0 * m5 * m11) + (m0 * m7 * m9) +
                (m4 * m1 * m11) - (m4 * m3 * m9) -
                (m8 * m1 * m7) + (m8 * m3 * m5);
  i->f16 [15] = (m0 * m5 * m10) - (m0 * m6 * m9) -
                (m4 * m1 * m10) + (m4 * m2 * m9) +
                (m8 * m1 * m6) - (m8 * m2 * m5);
  
  cxf32 det = (m0 * i->f16 [0]) + (m1 * i->f16 [4]) + (m2 * i->f16 [8]) + (m3 * i->f16 [12]);
  
  if (det != 0.0f)
  {
    cxf32 invDet = 1.0f / det;
    
    cx_mat4x4_mul_scalar (i, i, invDet);
  }
  
  return det;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_mat4x4_validate (const cx_mat4x4 *m)
{
  bool valid = true;
  
  for (cxu8 i = 0; i < 16; ++i)
  {
    valid &= cx_validatef (m->f16 [i]);
  }
  
  return valid;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat4x4_string (char *destBuffer, cxu32 destbufferSize, const cx_mat4x4 *m)
{
  CX_ASSERT (m);
  
  cx_sprintf (destBuffer, destbufferSize, "{ \n%.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f,\n%.2f, %.2f, %.2f, %.2f\n }", 
                                           m->f16 [0], m->f16 [4], m->f16 [8], m->f16 [12],
                                           m->f16 [1], m->f16 [5], m->f16 [9], m->f16 [13],
                                           m->f16 [2], m->f16 [6], m->f16 [10], m->f16 [14],
                                           m->f16 [3], m->f16 [7], m->f16 [11], m->f16 [15]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
