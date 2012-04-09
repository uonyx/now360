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

#define CX_PI             (3.14159265f)
#define CX_HALF_PI        (CX_PI * 0.5f)
#define CX_EPSILON        (0.0f)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_min (cxf32 x, cxf32 y);
static CX_INLINE cxf32 cx_max (cxf32 x, cxf32 y);
static CX_INLINE cxf32 cx_clamp (cxf32 x, cxf32 min, cxf32 max);
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

static CX_INLINE cxf32 cx_min (cxf32 x, cxf32 y)
{
  return (x < y ? x : y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_max (cxf32 x, cxf32 y)
{
  return (x > y ? x : y);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxf32 cx_clamp (cxf32 x, cxf32 min, cxf32 max)
{
  return (x < min) ? min : (x > max) ? max : x;
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

#endif
