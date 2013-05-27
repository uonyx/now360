//
//  cx_string.c
//
//  Created by Ubaka Onyechi on 01/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "cx_string.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_explode (char **dst, cxu32 dstSize, const char *src, char delimiter)
{
  CX_ASSERT (dst);
  CX_ASSERT (src);
  
  cxu32 count = 0;
  
  const char *start = src;
  const char *found = strchr (start, delimiter);
  
  while (found)
  {
    CX_ASSERT (count < dstSize);
    
    dst [count++] = cx_strdup (start, found - start);
    
    start = found + 1;
    
    found = strchr (start, delimiter);
  }
  
  CX_ASSERT (count < dstSize);
  
  dst [count++] = cx_strdup (start, strlen (start));
  
  return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_str_html_unescape (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);
  
  const char *s = src;
  cxu32 len = 0;
  char ch = 0;
  
  while ((ch = *s))
  {
    if (ch == '&')
    {
      if ((s [1] == 'q') && (s [2] == 'u') && (s [3] == 'o') && (s [4] == 't') && (s [5] == ';')) // &quot
      {
        s += 6;
        ch = '\"';
      }
      else if ((s [1] == 'a') && (s [2] == 'p') && (s [3] == 'o') && (s [4] == 's') && (s [5] == ';')) // &apos
      {
        s += 6;
        ch = '\'';
      }
      else if ((s [1] == 'a') && (s [2] == 'm') && (s [3] == 'p') && (s [4] == ';')) // &amp
      {
        s += 5;
        ch = '&';
      }
      else if ((s [1] == 'g') && (s [2] == 't') && (s [3] == ';')) // &gt
      {
        s += 4;
        ch = '>';
      }
      else if ((s [1] == 'l') && (s [2] == 't') && (s [3] == ';')) // &lt
      {
        s += 4;
        ch = '<';
      }
      else 
      {
        s++;
      }
    }
    else 
    {
      s++;
    }
    
    CX_ASSERT (len < dstSize);
    dst [len++] = ch;
  }
  
  CX_ASSERT (len < dstSize);
  dst [len] = 0;
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_percent_encode (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);
  
  cxu32 len = 0;
  cxu8 c = 0;
  char enc [16];
  const char *s = src;
  
  while ((c = *s))
  {
    // unreserved characters
    if (((c >= '0') && (c <= '9')) ||
        ((c >= 'a') && (c <= 'z')) ||
        ((c >= 'A') && (c <= 'Z')) ||
        ((c == '-') || (c == '.') || (c == '_') || (c == '~')))
    {
      CX_ASSERT (len < dstSize);
      dst [len++] = c;
    }
    else // ascii control characters, reserved characters, unsafe characters
    {
      cxi32 encSize = cx_sprintf (enc, 16, "%%%02x", c);
#if 1
      for (cxi32 i = 0; i < encSize; ++i)
      {
        CX_ASSERT (len < dstSize);
        dst [len++] = enc [i];
      }
#else
      CX_ASSERT ((len + encSize) < dstSize); CX_REFERENCE_UNUSED_VARIABLE (encSize);
      dst [len++] = enc [0];
      dst [len++] = enc [1];
      dst [len++] = enc [2];
#endif
    }
    
    s++;
  }
  
  CX_ASSERT (len < dstSize);
  
  dst [len] = 0;
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
