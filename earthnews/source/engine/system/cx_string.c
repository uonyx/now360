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

#define UTF8_BYTE1 0x80u
#define UTF8_BYTE2 0xc0u
#define UTF8_BYTE3 0xe0u
#define UTF8_BYTE4 0xf0u
#define UTF8_BYTE5 0xf8u
#define UTF8_BYTE6 0xfcu

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
  
  if (*start != '\0')
  {
    dst [count++] = cx_strdup (start, strlen (start));
  }
  
  return count;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_html_unescape (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize > 0);
  
  const char *s = src;
  cxu32 len = 0;
  char ch = 0;
  
  while ((ch = *s))
  {
    CX_ASSERT (len < dstSize);
    
    if (len >= (dstSize - 1))
    {
      break;
    }
    
    if (ch == '&')
    {
      if ((s [1] == 'a') && (s [2] == 'm') && (s [3] == 'p') && (s [4] == ';')) // &amp;
      {
        dst [len++] = '&';
        s += 5;
      }
      else if ((s [1] == 'g') && (s [2] == 't') && (s [3] == ';')) // &gt;
      {
        dst [len++] = '>';
        s += 4;
      }
      else if ((s [1] == 'l') && (s [2] == 't') && (s [3] == ';')) // &lt;
      {
        dst [len++] = '<';
        s += 4;
      }
      else if ((s [1] == 'q') && (s [2] == 'u') && (s [3] == 'o') && (s [4] == 't') && (s [5] == ';')) // &quot;
      {
        dst [len++] = '\"';
        s += 6;
      }
      else if ((s [1] == 'a') && (s [2] == 'p') && (s [3] == 'o') && (s [4] == 's') && (s [5] == ';')) // &apos;
      {
        dst [len++] = '\'';
        s += 6;
      }
      else if ((s [1] == 'd') && (s [2] == 'e') && (s [3] == 'g') && (s [4] == ';')) // &deg;
      {
        dst [len++] = 194;
        dst [len++] = 176;
        s += 5;
      }
      else if ((s [1] == 'y') && (s [2] == 'e') && (s [3] == 'n') && (s [4] == ';')) // &yen;
      {
        dst [len++] = 194;
        dst [len++] = 165;
        s += 5;
      }
      else if ((s [1] == 'e') && (s [2] == 'u') && (s [3] == 'r') && (s [4] == 'o') && (s [5] == ';')) // &euro;
      {
        dst [len++] = 226;
        dst [len++] = 130;
        dst [len++] = 172;
        s += 6;
      }
      else if ((s [1] == 'p') && (s [2] == 'o') && (s [3] == 'u') && (s [4] == 'n') && (s [5] == 'd') && (s [6] == ';')) // &pound;
      {
        dst [len++] = 194;
        dst [len++] = 163;
        s += 7;
      }
      else if ((s [1] == 'l') && (s [2] == 's') && (s [3] == 'q') && (s [4] == 'u') && (s [5] == 'o') && (s [6] == ';')) // &lsquo;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 152;
        s += 7;
      }
      else if ((s [1] == 'r') && (s [2] == 's') && (s [3] == 'q') && (s [4] == 'u') && (s [5] == 'o') && (s [6] == ';')) // &rsquo;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 153;
        s += 7;
      }
      else if ((s [1] == 'l') && (s [2] == 'd') && (s [3] == 'q') && (s [4] == 'u') && (s [5] == 'o') && (s [6] == ';')) // &ldquo;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 156;
        s += 7;
      }
      else if ((s [1] == 'r') && (s [2] == 'd') && (s [3] == 'q') && (s [4] == 'u') && (s [5] == 'o') && (s [6] == ';')) // &rdquo;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 157;
        s += 7;
      }
      else if ((s [1] == 't') && (s [2] == 'r') && (s [3] == 'a') && (s [4] == 'd') && (s [5] == 'e') && (s [6] == ';')) // &trade;
      {
        dst [len++] = 226;
        dst [len++] = 132;
        dst [len++] = 162;
        s += 7;
      }
      else if ((s [1] == 'm') && (s [2] == 'i') && (s [3] == 'n') && (s [4] == 'u') && (s [5] == 's') && (s [6] == ';')) // &minus;
      {
        dst [len++] = 226;
        dst [len++] = 136;
        dst [len++] = 146;
        s += 7;
      }
      else if ((s [1] == 'n') && (s [2] == 'd') && (s [3] == 'a') && (s [4] == 's') && (s [5] == 'h') && (s [6] == ';')) // &ndash;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 147;
        s += 7;
      }
      else if ((s [1] == 'm') && (s [2] == 'd') && (s [3] == 'a') && (s [4] == 's') && (s [5] == 'h') && (s [6] == ';')) // &mdash;
      {
        dst [len++] = 226;
        dst [len++] = 128;
        dst [len++] = 148;
        s += 7;
      }
      else if ((s [1] == 'c') && (s [2] == 'o') && (s [3] == 'p') && (s [4] == 'y') && (s [5] == ';')) // &copy;
      {
        dst [len++] = 194;
        dst [len++] = 169;
        s += 6;
      }
      else if ((s [1] == 'r') && (s [2] == 'e') && (s [3] == 'g') && (s [4] == ';')) // &reg;
      {
        dst [len++] = 194;
        dst [len++] = 174;
        s += 5;
      }
      else if ((s [1] == 's') && (s [2] == 'd') && (s [3] == 'o') && (s [4] == 't') && (s [5] == ';')) // &sdot;
      {
        dst [len++] = 226;
        dst [len++] = 139;
        dst [len++] = 133;
        s += 5;
      }
      else
      {
        dst [len++] = ch;
        s++;
      }
    }
    else 
    {
      dst [len++] = ch;
      s++;
    }
  }
  
  CX_ASSERT (len < dstSize);
  dst [len] = 0;
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_utf8_decode (cxu32 *dst, const cxu8 *src)
{
  CX_ASSERT (dst);
  CX_ASSERT (src);
  
  cxu32 ch = 0;
  cxu32 offset = 0;

  if ((src [0] & UTF8_BYTE6) == UTF8_BYTE6)
  {
    offset = 6;
  }
  else if ((src [0] & UTF8_BYTE5) == UTF8_BYTE5)
  {
    offset = 5;
  }
  else if ((src [0] & UTF8_BYTE4) == UTF8_BYTE4)
  {
    ch =  ((src [0] & 0x07) << 18) |
          ((src [1] & 0x3f) << 12) |
          ((src [2] & 0x3f) << 6)  |
          ((src [3] & 0x3f));
    
    offset = 4;
  }
  else if ((src [0] & UTF8_BYTE3) == UTF8_BYTE3)
  {
    ch =  ((src [0] & 0x0f) << 12) |
          ((src [1] & 0x3f) << 6) |
          ((src [2] & 0x3f));
    
    offset = 3;
  }
  else if ((src [0] & UTF8_BYTE2) == UTF8_BYTE2)
  {
    ch =  ((src [0] & 0x1f) << 6) |
          ((src [1] & 0x3f));
    
    offset = 2;
  }
  else if (src [0] < UTF8_BYTE1)
  {
    ch =  (src [0]);
    offset = 1;
  }
  else
  {
    offset = strlen ((const char *) src);
    CX_ERROR ("Invalid UTF8 character");
  }
  
  *dst = ch;
  
  return offset;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_utf8_to_unicode (cxu32 *dst, cxu32 dstSize, const char *utf8src)
{
  cxu32 dstLen = 0;
  
  cxu32 srcSize = strlen (utf8src); // data length
  
  CX_ASSERT ((srcSize < dstSize) && "dstSize too small");

  const cxu8 *src = (const cxu8 *) utf8src;
  
  cxi32 ss = srcSize;
#if CX_DEBUG
  cxi32 srcCount = 0;
#endif
  while ((ss > 0) && (dstLen < (dstSize - 1)))
  {
    cxu32 ch = 0;
    cxu32 offset = cx_str_utf8_decode (&ch, src);
    
    CX_ASSERT (dstLen < dstSize);
    
    dst [dstLen++] = ch;
    
    src += offset;

    ss -= offset;
#if CX_DEBUG
    srcCount += offset;
#endif
  }
  
  dst [dstLen] = 0;
  
  return dstLen;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_percent_encode (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize > 0);
  
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
      for (cxi32 i = 0; i < encSize; ++i)
      {
        CX_ASSERT (len < dstSize);
        dst [len++] = enc [i];
      }
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
