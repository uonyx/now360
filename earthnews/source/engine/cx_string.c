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

#define UTF8_MASK_BITS                0x3F
#define UTF8_MASK_BYTE                0x80
#define UTF8_MASK_2BYTES              0xC0
#define UTF8_MASK_3BYTES              0xE0
#define UTF8_MASK_4BYTES              0xF0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_str_char_to_utf8_char (unsigned char *dst, cxu32 dstSize, cxu32 src)
{
  CX_ASSERT (dst);
  CX_ASSERT (dstSize >= 4);
  
  cxi32 i;
  cxi32 len = 0;
  unsigned char *d = dst;
  unsigned char dest_value;
  
  if (src <= 0x0000007F)
  {
    *d = (unsigned char)src;
    
    len = 1;
  }
  else if (src <= 0x000007FF)
  {
    dest_value = UTF8_MASK_2BYTES;
    
    i = (src >> 6) & 0x1F;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = src & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    len = 2;
  }
  else if (src <= 0x0000FFFF)
  {
    dest_value = UTF8_MASK_3BYTES;
    
    i = (src >> 12) & 0x0F;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = (src >> 6) & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = src & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    len = 3;
  }
  else
  {
    CX_ASSERT (src <= 0x0010FFFF);
    
    dest_value = UTF8_MASK_4BYTES;
    
    i = (src >> 18) & 0x07;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = (src >> 12) & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = (src >> 6) & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    d ++;
    
    dest_value = UTF8_MASK_BYTE;
    
    i = src & UTF8_MASK_BITS;
    
    dest_value |= (unsigned char)i;
    
    *d = dest_value;
    
    len = 4;
  }
  
  return len;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

