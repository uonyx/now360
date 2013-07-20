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

typedef enum   // http://en.wikipedia.org/wiki/Unicode_block
{
  CX_STR_UNICODE_BLOCK_INVALID = -1,
  CX_STR_UNICODE_BLOCK_LATIN_BASIC,         // 96  (0-127)         (: english, french, spanish, german, vietnamese)
  CX_STR_UNICODE_BLOCK_LATIN_EXTENDED,      // 96  (128-255)       (: french, german, spanish, icelandic, vietnamese)
  CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A,    // 128 (256-383)       (: latin, czech, dutch, polish, turkish)
  CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_B,    // 208 (384-591)       (: africa alphabet, pan-nigerian, americanist, khosian, pinyin, romanian,
  CX_STR_UNICODE_BLOCK_IPA,                 // 96  (592-687)       (: international phonetic alphabet)
  CX_STR_UNICODE_BLOCK_GREEK_COPTIC,        // 144 (880-1023)      (: greek)
  CX_STR_UNICODE_BLOCK_CYRILLIC,            // 256 (1024-1279)     (: russia, serbia, bulgaria, ukraine, belarus, bosnia, etc
  CX_STR_UNICODE_BLOCK_CYRILLIC_SUPPLEMENT, // 48  (1280-1327)     (: abkhaz, komi, mordvin, aleut,
  CX_STR_UNICODE_BLOCK_HEBREW,              // 112 (1424-1535)
  CX_STR_UNICODE_BLOCK_ARABIC,              // 256 (1536-1791)
  CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS,    // 48  (8352-8399)
  CX_STR_UNICODE_BLOCK_CJK_FULL,            // 20940 (19968-40908)
} cx_str_unicode_block;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_str_explode (char **dst, cxu32 dstSize, const char *src, char delimiter);
cxu32 cx_str_percent_encode (char *dst, cxu32 dstSize, const char *src);
cxu32 cx_str_html_unescape (char *dst, cxu32 dstSize, const char *src);
cxu32 cx_str_utf8_to_unicode (cxu32 *dst, cxu32 dstSize, const char *utf8src);
cxu32 cx_str_utf8_decode (cxu32 *dst, const cxu8 *src);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strcat (char *dst, cxu32 dstSize, const char *src);
static CX_INLINE char *cx_strcpy (char *dst, cxu32 dstSize, const char *src);
static CX_INLINE char *cx_strdup (const char *str, cxu32 strLength);
static CX_INLINE cxi32 cx_sprintf (char *dst, cxu32 dstSize, const char *format, ...);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strcpy (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);

  char *d = strncpy (dst, src, dstSize);
  
  dst [dstSize - 1] = 0;
  
  return d;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strcat (char *dst, cxu32 dstSize, const char *src)
{
  CX_ASSERT (src);
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);
  CX_ASSERT ((dstSize - strlen (dst)) > strlen (src));
  
#if 1
  char *d = dst;
  while (*d) { d++; }
  while ((*d++ = *src++)) {}
  return --d;
#else
  return strcat (dst, src);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE char *cx_strdup (const char *str, cxu32 strLength)
{
  CX_ASSERT (str);
  CX_ASSERT (strLength);
  
  cxu32 dsize = strLength + 1;
  
  char *d = cx_malloc (dsize);
  
  CX_ASSERT (d);
  
  return cx_strcpy (d, dsize, str);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE cxi32 cx_sprintf (char *dst, cxu32 dstSize, const char *format, ...)
{
  CX_ASSERT (dst);
  CX_ASSERT (dstSize);
  CX_ASSERT (format);
  
  cxi32 c;
  va_list args;
  
  va_start (args, format);
  c = vsnprintf (dst, dstSize, format, args);
  va_end (args);
  
  return c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
