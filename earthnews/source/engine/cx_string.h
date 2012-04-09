//
//  cx_string.h
//
//  Created by Ubaka Onyechi on 21/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_STRING_H
#define CX_STRING_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE int cx_sprintf (char *dest, cxi32 destBufferSize, const char *format, ...);
static CX_INLINE char *cx_strcpy (char *dest, cxi32 destBufferSize, const char *src);
static CX_INLINE char *cx_strdup (const char *str);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strcpy (char *dest, cxi32 destBufferSize, const char *src)
{
  //CX_ASSERT (strlen(src) < destBufferSize);
  //return strcpy (dest, src);
  
  char *d = strncpy (dest, src, destBufferSize);
  dest [destBufferSize - 1] = 0;
  return d;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strdup (const char *str)
{
  char *d = cx_malloc (strlen (str) + 1);
  
  CX_ASSERT (d);
  
  return strcpy (d, str);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE int cx_sprintf (char *dest, cxi32 destBufferSize, const char *format, ...)
{
  int c;
  va_list args;
  
  va_start (args, format);
  c = vsnprintf(dest, destBufferSize, format, args);
  va_end (args);
  
  return c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
