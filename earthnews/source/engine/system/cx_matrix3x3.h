//
//  cx_matrix3x3.h
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#ifndef CX_MATRIX3X3_H
#define CX_MATRIX3X3_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_math.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

union cx_mat3x3
{
  cxf32 f9 [9];
};

typedef union cx_mat3x3 cx_mat3x3;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat3x3_identity (cx_mat3x3 *m);
static CX_INLINE void cx_mat3x3_zero (cx_mat3x3 *m);
static CX_INLINE void cx_mat3x3_set (cx_mat3x3 *m, cxf32 f9 [9]);
static CX_INLINE void cx_mat3x3_transpose (cx_mat3x3 * CX_RESTRICT t, const cx_mat3x3 * CX_RESTRICT m);
static CX_INLINE cxf32 cx_mat3x3_determinant (const cx_mat3x3 *m);
static CX_INLINE cxf32 cx_mat3x3_inverse (cx_mat3x3 * CX_RESTRICT i, const cx_mat3x3 * CX_RESTRICT m);
static CX_INLINE bool cx_mat3x3_validate (const cx_mat3x3 *m);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat3x3_identity (cx_mat3x3 *m)
{
  CX_ASSERT (m);
  
  // column 1
  m->f9 [0] = 1.0f;
  m->f9 [1] = 0.0f;
  m->f9 [2] = 0.0f;
  // column 2
  m->f9 [3] = 0.0f;
  m->f9 [4] = 1.0f;
  m->f9 [5] = 0.0f;
  // column 3
  m->f9 [6] = 0.0f;
  m->f9 [7] = 0.0f;
  m->f9 [8] = 1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat3x3_zero (cx_mat3x3 *m)
{
  CX_ASSERT (m);
  
  m->f9 [0] = 0.0f;
  m->f9 [1] = 0.0f;
  m->f9 [2] = 0.0f;
  m->f9 [3] = 0.0f;
  m->f9 [4] = 0.0f;
  m->f9 [5] = 0.0f;
  m->f9 [6] = 0.0f;
  m->f9 [7] = 0.0f;
  m->f9 [8] = 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat3x3_set (cx_mat3x3 * CX_RESTRICT m, cxf32 f9 [9])
{
  CX_ASSERT (m);
  
  m->f9 [0] = f9 [0];
  m->f9 [1] = f9 [1];
  m->f9 [2] = f9 [2];
  m->f9 [3] = f9 [3];
  m->f9 [4] = f9 [4];
  m->f9 [5] = f9 [5];
  m->f9 [6] = f9 [6];
  m->f9 [7] = f9 [7];
  m->f9 [8] = f9 [8];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_mat3x3_transpose (cx_mat3x3 * CX_RESTRICT t, const cx_mat3x3 * CX_RESTRICT m)
{
  CX_ASSERT (m);
  CX_ASSERT (t);
  
  cxf32 t0 = m->f9 [0];
  cxf32 t1 = m->f9 [3];
  cxf32 t2 = m->f9 [6];
  cxf32 t3 = m->f9 [1];
  cxf32 t4 = m->f9 [4];
  cxf32 t5 = m->f9 [7];
  cxf32 t6 = m->f9 [2];
  cxf32 t7 = m->f9 [5];
  cxf32 t8 = m->f9 [8];
  
  t->f9 [0] = t0;
  t->f9 [1] = t1;
  t->f9 [2] = t2;
  t->f9 [3] = t3;
  t->f9 [4] = t4;
  t->f9 [5] = t5;
  t->f9 [6] = t6;
  t->f9 [7] = t7;
  t->f9 [8] = t8;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_mat3x3_determinant (const cx_mat3x3 *m)
{
  CX_ASSERT (m);
  
  cxf32 m0 = m->f9 [0];
  cxf32 m1 = m->f9 [1];
  cxf32 m2 = m->f9 [2];
  cxf32 m3 = m->f9 [3];
  cxf32 m4 = m->f9 [4];
  cxf32 m5 = m->f9 [5];
  cxf32 m6 = m->f9 [6];
  cxf32 m7 = m->f9 [7];
  cxf32 m8 = m->f9 [8];
  
  cxf32 cofactor0 = (m4 * m8) - (m5 * m7);
  cxf32 cofactor3 = (m2 * m7) - (m1 * m8);
  cxf32 cofactor6 = (m1 * m5) - (m2 * m4);
  
  cxf32 det = (m0 * cofactor0) + (m3 * cofactor3) + (m6 * cofactor6);
  
  return det;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_mat3x3_inverse (cx_mat3x3 * CX_RESTRICT i, const cx_mat3x3 * CX_RESTRICT m)
{
  CX_ASSERT (i);
  CX_ASSERT (m);
  
  cxf32 m0 = m->f9 [0];
  cxf32 m1 = m->f9 [1];
  cxf32 m2 = m->f9 [2];
  cxf32 m3 = m->f9 [3];
  cxf32 m4 = m->f9 [4];
  cxf32 m5 = m->f9 [5];
  cxf32 m6 = m->f9 [6];
  cxf32 m7 = m->f9 [7];
  cxf32 m8 = m->f9 [8];
  
  cxf32 cofactor0 = (m4 * m8) - (m5 * m7);
  cxf32 cofactor3 = (m2 * m7) - (m1 * m8);
  cxf32 cofactor6 = (m1 * m5) - (m2 * m4);
  
  cxf32 det = (m0 * cofactor0) + (m3 * cofactor3) + (m6 * cofactor6);
  
  // inverse = 1/det * adjoint
  // adjoint = (transpose of matrix of cofactors
  
  if (!cx_is_zero (det))
  {
    cxf32 invDet = 1.0f / det;
    
    i->f9 [0] = invDet * cofactor0;
    i->f9 [1] = invDet * cofactor3;
    i->f9 [2] = invDet * cofactor6;
    i->f9 [3] = invDet * ((m5 * m6) - (m3 * m8)); // cofactor1
    i->f9 [4] = invDet * ((m0 * m8) - (m2 * m6)); // cofactor4
    i->f9 [5] = invDet * ((m2 * m3) - (m0 * m5)); // cofactor7
    i->f9 [6] = invDet * ((m3 * m7) - (m4 * m6)); // cofactor2
    i->f9 [7] = invDet * ((m1 * m6) - (m0 * m7)); // cofactor5
    i->f9 [8] = invDet * ((m0 * m4) - (m1 * m3)); // cofactor8
  }
  
  return det;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_mat3x3_validate (const cx_mat3x3 *m)
{
  CX_ASSERT (m);
  
  for (cxu8 i = 0; i < 9; ++i)
  {
    if (!cx_validatef (m->f9 [i]))
    {
      return false;
    }
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
