//
//  cx_math.h
//
//  Created by Ubaka Onyechi on 21/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_MATH_H
#define CX_MATH_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_defines.h"
#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SIMD_ENABLED   1

#if CX_SIMD_ENABLED
#ifdef __ARM_NEON__
#define CX_SIMD_NEON
#endif
#endif

#ifdef CX_SIMD_NEON
#include <arm_neon.h>
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_PI             (3.14159265358979f)
#define CX_HALF_PI        (CX_PI * 0.5f)
#define CX_EPSILON        (1.0e-6f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define cx_min(A,B)	((A) < (B) ? (A) : (B))
#define cx_max(A,B)	((A) > (B) ? (A) : (B))
#define cx_clamp(X,MIN,MAX) (((X) < (MIN)) ? (MIN) : ((X) > (MAX)) ? (MAX) : (X))

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_is_zero (cxf32 x);
static CX_INLINE bool cx_validatef (cxf32 x);
static CX_INLINE cxf32 cx_linearstep (cxf32 edge0, cxf32 edge1, cxf32 x);
static CX_INLINE cxf32 cx_smoothstep (cxf32 edge0, cxf32 edge1, cxf32 x);
static CX_INLINE cxf32 cx_lerp (cxf32 from, cxf32 to, cxf32 t);
static CX_INLINE cxf32 cx_sqrt (cxf32 x);
static CX_INLINE cxf32 cx_inv_sqrt (cxf32 x);
static CX_INLINE cxf32 cx_pow (cxf32 base, cxf32 exponent);
static CX_INLINE cxf32 cx_deg (cxf32 radians);
static CX_INLINE cxf32 cx_rad (cxf32 degree);
static CX_INLINE cxf32 cx_sin (cxf32 x);
static CX_INLINE cxf32 cx_cos (cxf32 x);
static CX_INLINE cxf32 cx_tan (cxf32 x);
static CX_INLINE cxf32 cx_asin (cxf32 x);
static CX_INLINE cxf32 cx_acos (cxf32 x);
static CX_INLINE cxf32 cx_atan (cxf32 x);
static CX_INLINE cxf32 cx_atan2 (cxf32 y, cxf32 x);
static CX_INLINE cxf32 cx_sign (cxf32 x);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_linearstep (cxf32 edge0, cxf32 edge1, cxf32 x)
{
  cxf32 t = (x - edge0) / (edge1 - edge0);
  
  return cx_clamp (t, 0.0f, 1.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_smoothstep (cxf32 edge0, cxf32 edge1, cxf32 x)
{
  cxf32 t = cx_linearstep (edge0, edge1, x);
  
  // hermite interpolation
  return t * t * (3.0f - (2.0f * t));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_lerp (cxf32 start, cxf32 end, cxf32 t)
{
  return (start + (end - start) * t);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_sqrt (cxf32 x)
{
  return sqrtf (x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_inv_sqrt (cxf32 x)
{
  return (1.0f / cx_sqrt (x));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_pow (cxf32 base, cxf32 exponent)
{
  return powf (base, exponent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_deg (cxf32 radians)
{
  return (radians * (180.0f / CX_PI));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_rad (cxf32 degree)
{
  return (degree * (CX_PI / 180.0f));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_sin (cxf32 x)
{
  return sinf (x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_cos (cxf32 x)
{
  return cosf (x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_tan (cxf32 x)
{
  return tanf (x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_asin (cxf32 x)
{
  if (x > -1.0f) // to prevent "invalid" float-point exception
  {
    return (x < 1.0f) ? asinf (x) : CX_HALF_PI;
  }
  
  return -CX_HALF_PI;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_acos (cxf32 x)
{
  if (x > -1.0f) // to prevent "invalid" float-point exception
  {
    return (x < 1.0f) ? acosf (x) : 0.0f;
  }
  
  return CX_PI;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_atan (cxf32 x)
{
  return atanf (x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_atan2 (cxf32 y, cxf32 x)
{
  return atan2f (y, x);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_sign (cxf32 x)
{
  return (x >= 0.0f) ? 1.0f : -1.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_is_zero (cxf32 x)
{
  return fabsf (x) <= CX_EPSILON;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE bool cx_validatef (cxf32 x)
{
  union number
  {
    cxu32 i;
    cxf32 f;
  } n;
  
  n.f = x;
  
  cxu32 exponent = n.i & 0x7f800000;
  cxu32 mantissa = n.i & 0x007fffff;
  
  // check for infinity
  if ((exponent == 0x7f800000) && (mantissa == 0))
  {
    //CX_LOG_CONSOLE (1, "invalid float: inf");
    return false;
  }
  
  // check for qnan
  if ((exponent == 0x7f800000) && (mantissa == 0x400000))
  {
    //CX_LOG_CONSOLE (1, "invalid float: nan");
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
