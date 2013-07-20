//
//  cx_font.c
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "../system/cx_file.h"
#include "../system/cx_matrix4x4.h"
#include "../system/cx_vector2.h"
#include "../system/cx_utility.h"
#include "../3rdparty/stb/stb_truetype.h"

#include "cx_font.h"
#include "cx_gdi.h"
#include "cx_shader.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FONT_MAX_TEXT_LENGTH     (512)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_font_impl_stb
{
  stbtt_bakedchar *ttfCharData; 
  cx_texture *texture;
  cxu32 textureWidth, textureHeight;
  cxf32 scaleX, scaleY;
  cxf32 height;
  cxu32 *unicodePts;
  cxu32  unicodePtsSize;
  const cx_font *chain;
} cx_font_impl;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int cmp_codepoints (const void *a, const void *b)
{
  CX_ASSERT (a);
  CX_ASSERT (b);
  
  cxu32 va = *(cxu32 *) a;
  cxu32 vb = *(cxu32 *) b;
  
  if (va < vb)
  {
    return -1;
  }
  else if (va > vb)
  {
    return 1;
  }
  else// if (va == vb )
  {
    CX_ERROR ("code point duplicated");
    return 0;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_font_get_unicode_codepoint_range (cx_str_unicode_block block, cxu32 *rmin, cxu32 *rmax)
{
  CX_ASSERT (rmin);
  CX_ASSERT (rmax);
  
  bool success = true;
  
  switch (block)
  {
    case CX_STR_UNICODE_BLOCK_LATIN_BASIC:          { *rmin = 32;  *rmax = 127; break; }
    case CX_STR_UNICODE_BLOCK_LATIN_SUPPLEMENT:     { *rmin = 160; *rmax = 255; break; }
    case CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_A:     { *rmin = 256; *rmax = 383; break; }
    case CX_STR_UNICODE_BLOCK_LATIN_EXTENDED_B:     { *rmin = 384; *rmax = 591; break; }
    case CX_STR_UNICODE_BLOCK_IPA:                  { *rmin = 592; *rmax = 687; break; }
    case CX_STR_UNICODE_BLOCK_GREEK_COPTIC:         { *rmin = 880; *rmax = 1023; break; }
    case CX_STR_UNICODE_BLOCK_CYRILLIC:             { *rmin = 1024; *rmax = 1279; break; }
    case CX_STR_UNICODE_BLOCK_CYRILLIC_SUPPLEMENT:  { *rmin = 1280; *rmax = 1327; break; }
    case CX_STR_UNICODE_BLOCK_HEBREW:               { *rmin = 1424; *rmax = 1535; break; }
    case CX_STR_UNICODE_BLOCK_ARABIC:               { *rmin = 1536; *rmax = 1791; break; }
    case CX_STR_UNICODE_BLOCK_CURRENCY_SYMBOLS:     { *rmin = 8352; *rmax = 8399; break; }
    case CX_STR_UNICODE_BLOCK_CJK_FULL:             { *rmin = 19968; *rmax = 40908; break; }
    default:                                        { *rmin = 0; *rmax = 0; success = false; break; }
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cxu32 *cx_font_create_unicode_codepoints (cx_str_unicode_block *unicodeBlocks, cxu32 unicodeBlockCount,
                                                 cxu32 *extraUnicodeCodepts, cxu32 extraUnicodeCodeptsCount,
                                                 cxu32 *codePointsSize)
{
  CX_ASSERT (codePointsSize);
  
  cxu32 *unicodeCodePoints = NULL;
  cxu32 unicodeCodePointsSize = 0;

  for (cxu32 i = 0; i < unicodeBlockCount; ++i)
  {
    CX_ASSERT (unicodeBlocks);
    
    cx_str_unicode_block block = unicodeBlocks [i];
    
    cxu32 rmin;
    cxu32 rmax;
    
    if (cx_font_get_unicode_codepoint_range (block, &rmin, &rmax))
    {
      CX_ASSERT (rmax > rmin);
      
      cxu32 numCodepts = (rmax - rmin) + 1;
      
      unicodeCodePointsSize += numCodepts;
    }
  }
  
  unicodeCodePointsSize += extraUnicodeCodeptsCount;
  
  if (unicodeCodePointsSize > 0)
  {
    cxu32 ucount = 0;
    unicodeCodePoints = cx_malloc (sizeof (cxu32) * unicodeCodePointsSize);
    
    for (cxu32 j = 0; j < unicodeBlockCount; ++j)
    {
      cx_str_unicode_block block = unicodeBlocks [j];
      
      cxu32 rmin;
      cxu32 rmax;
      
      if (cx_font_get_unicode_codepoint_range (block, &rmin, &rmax))
      {
        CX_ASSERT (rmax > rmin);
        
        for (cxu32 k = rmin; k <= rmax; ++k)
        {
          CX_ASSERT (ucount < unicodeCodePointsSize);
          
          unicodeCodePoints [ucount++] = k;
        }
      }
    }
    
    for (cxu32 k = 0; k < extraUnicodeCodeptsCount; ++k)
    {
      CX_ASSERT (extraUnicodeCodepts);
      unicodeCodePoints [ucount++] = extraUnicodeCodepts [k];
    }
    
    CX_ASSERT (ucount == unicodeCodePointsSize);
    
    qsort (unicodeCodePoints, unicodeCodePointsSize, sizeof (cxu32), cmp_codepoints);
  }

  *codePointsSize = unicodeCodePointsSize;
  
#if CX_DEBUG && 0
  for (cxu32 i = 0; i < unicodePtsSize; ++i)
  {
    CX_LOG_CONSOLE (1, "%d: %d", i, unicodePts [i]);
  }
#endif
  
  return unicodeCodePoints;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_font_get_texture_dims (cxu32 unicodePtsSize, cxu32 *width, cxu32 *height)
{
  CX_ASSERT (width);
  CX_ASSERT (height);
  
  cxu32 size = unicodePtsSize / 2;
  size = cx_util_roundup_pow2 (size);
  size = cx_clamp (size, 128, 4096);
  //size = cx_clamp (size, 128, 2048);
  
  *width = size;
  *height = size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font * cx_font_create (const char *filename, cxf32 fontsize,
                          cx_str_unicode_block *unicodeBlocks, cxu32 unicodeBlockCount,
                          cxu32 *extraUnicodeCodepts, cxu32 extraUnicodeCodeptsCount){
  CX_ASSERT (filename);
  
  cx_font *font = NULL;
  
  cxu8 *filedata = NULL;
  cxu32 filedataSize = 0;
  
  if (cx_file_storage_load_contents (&filedata, &filedataSize, filename, CX_FILE_STORAGE_BASE_RESOURCE))
  {
    cxu32 unicodePtsSize = 0;
    cxu32 *unicodePts = cx_font_create_unicode_codepoints (unicodeBlocks, unicodeBlockCount,
                                                           extraUnicodeCodepts, extraUnicodeCodeptsCount,
                                                           &unicodePtsSize);
    cxu32 textureWidth = 0;
    cxu32 textureHeight = 0;
    cx_font_get_texture_dims (unicodePtsSize, &textureWidth, &textureHeight);

    cx_font_impl *fontImpl = (cx_font_impl *) cx_malloc (sizeof (cx_font_impl));
  
    fontImpl->unicodePts     = unicodePts;
    fontImpl->unicodePtsSize = unicodePtsSize;
    fontImpl->textureWidth   = textureWidth;
    fontImpl->textureHeight  = textureWidth;
    fontImpl->scaleX         = 1.0f;
    fontImpl->scaleY         = 1.0f;
    fontImpl->height         = fontsize;
    fontImpl->ttfCharData    = (stbtt_bakedchar *) cx_malloc (fontImpl->unicodePtsSize * sizeof (stbtt_bakedchar));
    fontImpl->texture        = cx_texture_create (fontImpl->textureWidth, fontImpl->textureHeight, CX_TEXTURE_FORMAT_ALPHA);
    
    cxi32 ret = stbtt_BakeFontBitmap2 (filedata,
                                       0,
                                       fontsize,
                                       fontImpl->texture->data,
                                       fontImpl->textureWidth,
                                       fontImpl->textureHeight,
                                       fontImpl->unicodePts,
                                       fontImpl->unicodePtsSize,
                                       fontImpl->ttfCharData);
    //CX_ASSERT (ret != -1);
    CX_REF_UNUSED (ret);
    
    cx_texture_gpu_init (fontImpl->texture, true);
    cx_texture_data_destroy (fontImpl->texture);
    
    font = (cx_font *) cx_malloc (sizeof (cx_font));
    font->fontdata = fontImpl;
    
    cx_free (filedata);
  }
  
  return font;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_destroy (cx_font *font)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontdata);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
  
  cx_texture_destroy (fontImpl->texture);
  cx_free (fontImpl->ttfCharData);
  cx_free (font->fontdata);
  cx_free (font);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_set_scale (const cx_font *font, cxf32 x, cxf32 y)
{
  CX_ASSERT (font);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
#if 0
  fontImpl->scaleX = cx_clamp (x, 0.0f, 1.0f);
  fontImpl->scaleY = cx_clamp (y, 0.0f, 1.0f);
#else
  fontImpl->scaleX = x;
  fontImpl->scaleY = y;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_chain (cx_font *dst, const cx_font *src)
{
  CX_ASSERT (dst);
  CX_ASSERT (src);
  CX_ASSERT (dst != src);
  
  cx_font_impl *fontImpl = (cx_font_impl *) dst->fontdata;

  fontImpl->chain = src;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_render (const cx_font *font, const char *text, cxf32 x, cxf32 y, cxf32 z,
                     cx_font_alignment alignment, const cx_colour *colour)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontdata);
  CX_ASSERT (text);
  CX_ASSERT (colour);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_FONT);
  
  // use shader
  cx_shader_begin (shader);
  
  // set texture
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_DIFFUSE_MAP, fontImpl->texture);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
  cx_shader_set_float (shader, "u_z", &z, 1);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  
  cx_gdi_assert_no_errors ();
  
  cxf32 sx = fontImpl->scaleX;
  cxf32 sy = fontImpl->scaleY;
  cxf32 px = x;
  cxf32 py = y + (fontImpl->height * sy);
  
  if (alignment & CX_FONT_ALIGNMENT_CENTRE_X)
  {
    cxf32 tw = cx_font_get_text_width (font, text);
    px = px - (tw * 0.5f);
  }
  else if (alignment & CX_FONT_ALIGNMENT_RIGHT_X)
  {
    cxf32 tw = cx_font_get_text_width (font, text);
    px = px + (tw * 0.5f);
  }
  
  if (alignment & CX_FONT_ALIGNMENT_CENTRE_Y)
  {
    cxf32 th = cx_font_get_height (font);
    py = py - (th * 0.5f);
  }
  
  // gather vertices
  
  cxu32 srcSize = strlen (text); // data length
  const cxu8 *src = (const cxu8 *) text;
  
  cxi32 ss = srcSize;
  cxi32 ds = 0;
  
  cx_vec2 pos [srcSize * 4]; // unicode decoded strlen <= srcSize
  cx_vec2 uv [srcSize * 4];
  
  cxu32 vcount = 0;
  
  while (ss > 0)
  {
    cxu32 ch = 0;
    cxu32 offset = cx_str_utf8_decode (&ch, src);
    cxi32 cIndex = cx_util_bsearch_int (ch, fontImpl->unicodePts, fontImpl->unicodePtsSize);
    
    if (cIndex > -1)
    {
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, fontImpl->textureWidth, fontImpl->textureHeight,
                          cIndex, sx, sy, &px, &py, &quad, 1);
      
      cxu32 i = vcount * 4;
      
      CX_ASSERT ((i + 3) < (srcSize * 4));
      
      pos [i + 0].x = quad.x0;
      pos [i + 0].y = quad.y0;
      pos [i + 1].x = quad.x0;
      pos [i + 1].y = quad.y1;
      pos [i + 2].x = quad.x1;
      pos [i + 2].y = quad.y0;
      pos [i + 3].x = quad.x1;
      pos [i + 3].y = quad.y1;
      
      uv [i + 0].x = quad.s0;
      uv [i + 0].y = quad.t0;
      uv [i + 1].x = quad.s0;
      uv [i + 1].y = quad.t1;
      uv [i + 2].x = quad.s1;
      uv [i + 2].y = quad.t0;
      uv [i + 3].x = quad.s1;
      uv [i + 3].y = quad.t1;
      
      ++vcount;
    }
    
    src += offset;
    ss -= offset;
    
    ds++;
  }
  
  // render text
  
  if (vcount > 0)
  {
    // batch triangestrip points
    //(012) (213) (234) (435) ... total number of trianges == points - 2 == (len * 4) - 2
    
    cxu32 numTri = (vcount * 4) - 2;
    cxu32 numPoints = numTri * 3;
    
    cx_vec2 pos2 [numPoints];
    cx_vec2 uv2 [numPoints];
    cx_colour col [numPoints];
    
    cxu32 tri = 0;
    cxu32 tript = 0;
    cxu32 ni = 0;
    
    const cx_colour *null = cx_colour_null ();
    
    bool spacing = false; // space between characters
    
    while (tri < numTri)
    {
      cxu32 sa0 = tript;      // 0
      cxu32 sa1 = sa0 + 1;    // 1
      cxu32 sa2 = sa1 + 1;    // 2
      
      cxu32 sa3 = sa2;        // 2
      cxu32 sa4 = sa1;        // 1
      cxu32 sa5 = sa3 + 1;    // 3
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa0];
      uv2 [ni++] = uv [sa0];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa1];
      uv2 [ni++] = uv [sa1];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa2];
      uv2 [ni++] = uv [sa2];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa3];
      uv2 [ni++] = uv [sa3];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa4];
      uv2 [ni++] = uv [sa4];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa5];
      uv2 [ni++] = uv [sa5];
      
      tript = sa3;
      
      tri += 2;
      
      spacing = !spacing;
    }
    
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos2);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv2);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], 4, GL_FLOAT, GL_FALSE, 0, col);
    cx_gdi_assert_no_errors ();
    
    glDrawArrays (GL_TRIANGLE_STRIP, 0, numPoints);
    cx_gdi_assert_no_errors ();
    
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  }
  
  cx_shader_end (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_font_render_word_wrap (const cx_font *font, const char *text, cxf32 x, cxf32 y, cxf32 w, cxf32 h, 
                               cxf32 z, cx_font_alignment alignment, const cx_colour *colour)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontdata);
  CX_ASSERT (text);
  CX_ASSERT (colour);
  
#if CX_DEBUG
  cxu32 titleLen = strlen (text);
  CX_ASSERT (titleLen < CX_FONT_MAX_TEXT_LENGTH);
#endif
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_FONT);
  
  // use shader
  cx_shader_begin (shader);
  
  // set texture
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_DIFFUSE_MAP, fontImpl->texture);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
  cx_shader_set_float (shader, "u_z", &z, 1);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  cx_gdi_assert_no_errors ();
  
  cxf32 sx = fontImpl->scaleX;
  cxf32 sy = fontImpl->scaleY;
  cxf32 px = x;
  cxf32 py = y + (fontImpl->height * sy);
  
  if (alignment & CX_FONT_ALIGNMENT_CENTRE_X)
  {
    cxf32 tw = cx_font_get_text_width (font, text);
    px = px - (tw * 0.5f);
  }
  else if (alignment & CX_FONT_ALIGNMENT_RIGHT_X)
  {
    cxf32 tw = cx_font_get_text_width (font, text);
    px = px + (tw * 0.5f);
  }
  
  if (alignment & CX_FONT_ALIGNMENT_CENTRE_Y)
  {
    cxf32 th = cx_font_get_height (font);
    py = py - (th * 0.5f);
  }
  
  cxf32 ox = px;
  cxf32 bw = w - ox;
  cxf32 tw = 0.0f;
  
  // convert from utf8 to unicode
  cxu32 textBuffer [CX_FONT_MAX_TEXT_LENGTH];
  cxu32 tLen = cx_str_utf8_to_unicode (textBuffer, CX_FONT_MAX_TEXT_LENGTH, text);
  CX_REF_UNUSED (tLen);
  
  // set up newlines
  cxu32 c = 0;
  cxu32 *t = textBuffer;
  cxu32 *lastSpacePos = NULL;
  while ((c = *t))
  {
    cxi32 cIndex = cx_util_bsearch_int (c, fontImpl->unicodePts, fontImpl->unicodePtsSize);
    
    if (cIndex > -1)
    {
      stbtt_bakedchar *bakedChar = fontImpl->ttfCharData + cIndex;
      tw += bakedChar->xadvance * sx;
      
      if (c == 0x20)
      {
        lastSpacePos = t;
      }
      
      if ((tw > bw) && lastSpacePos)
      {
        *lastSpacePos = '\n';
        tw = 0.0f;
      }
    }
    
    t++;
  }
  
  // now draw text
  cx_vec2 pos [tLen * 4];
  cx_vec2 uv [tLen * 4];
  cxu32 vcount = 0;
  
  cxi32 newlines = 0;
  cxf32 fontHeight = fontImpl->height * sy;
  
  c = 0;
  t = textBuffer; 
  while ((c = *t++))
  {
    if (c == '\n')
    {
      px = ox;
      py += fontHeight;
      newlines++;
    }
    else
    {
      cxi32 cIndex = cx_util_bsearch_int (c, fontImpl->unicodePts, fontImpl->unicodePtsSize);
      
      if (cIndex > -1)
      {
        stbtt_aligned_quad quad;
        stbtt_GetBakedQuad (fontImpl->ttfCharData, fontImpl->textureWidth, fontImpl->textureHeight,
                            cIndex, sx, sy, &px, &py, &quad, 1);
        
        cxu32 i = vcount * 4;
        
        pos [i + 0].x = quad.x0;
        pos [i + 0].y = quad.y0;
        pos [i + 1].x = quad.x0;
        pos [i + 1].y = quad.y1;
        pos [i + 2].x = quad.x1;
        pos [i + 2].y = quad.y0;
        pos [i + 3].x = quad.x1;
        pos [i + 3].y = quad.y1;
        
        uv [i + 0].x = quad.s0;
        uv [i + 0].y = quad.t0;
        uv [i + 1].x = quad.s0;
        uv [i + 1].y = quad.t1;
        uv [i + 2].x = quad.s1;
        uv [i + 2].y = quad.t0;
        uv [i + 3].x = quad.s1;
        uv [i + 3].y = quad.t1;
        
        ++vcount;
      }
    }
  }
  
  if (vcount > 0)
  {
    cxu32 numTri = (vcount * 4) - 2;
    cxu32 numPoints = numTri * 3;
    
    cx_vec2 pos2 [numPoints];
    cx_vec2 uv2 [numPoints];
    cx_colour col [numPoints];
    
    cxu32 tri = 0;
    cxu32 tript = 0;
    cxu32 ni = 0;
    
    const cx_colour *null = cx_colour_null ();
    
    bool spacing = false; // space between characters
    
    while (tri < numTri)
    {
      cxu32 sa0 = tript;      // 0
      cxu32 sa1 = sa0 + 1;    // 1
      cxu32 sa2 = sa1 + 1;    // 2
      
      cxu32 sa3 = sa2;        // 2
      cxu32 sa4 = sa1;        // 1
      cxu32 sa5 = sa3 + 1;    // 3
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa0];
      uv2 [ni++] = uv [sa0];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa1];
      uv2 [ni++] = uv [sa1];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa2];
      uv2 [ni++] = uv [sa2];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa3];
      uv2 [ni++] = uv [sa3];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa4];
      uv2 [ni++] = uv [sa4];
      
      col [ni] = spacing ? *null : *colour;
      pos2 [ni] = pos [sa5];
      uv2 [ni++] = uv [sa5];
      
      tript = sa3;
      
      tri += 2;
      
      spacing = !spacing;
    }
    
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos2);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv2);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], 4, GL_FLOAT, GL_FALSE, 0, col);
    cx_gdi_assert_no_errors ();
    
    glDrawArrays (GL_TRIANGLE_STRIP, 0, numPoints);
    cx_gdi_assert_no_errors ();
    
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
    glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  }

  cx_shader_end (shader);
  
  return newlines;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_font_get_text_width (const cx_font *font, const char *text)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontdata);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
  
  cxf32 sx = fontImpl->scaleX;
  cxf32 width = 0.0f;
  
  cxu32 srcSize = strlen (text);
  const cxu8 *src = (const cxu8 *) text;
  cxi32 ss = srcSize;
  
  while (ss > 0)
  {
    cxu32 ch = 0;
    cxu32 offset = cx_str_utf8_decode (&ch, src);
    cxi32 cIndex = cx_util_bsearch_int (ch, fontImpl->unicodePts, fontImpl->unicodePtsSize);
    
    if (cIndex > -1)
    {
      stbtt_bakedchar *bakedChar = fontImpl->ttfCharData + cIndex;
      width += bakedChar->xadvance * sx;
    }
    src += offset;
    ss -= offset;
  }
  
  return width;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_font_get_height (const cx_font *font)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontdata);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontdata;
  
  return (fontImpl->height * fontImpl->scaleY);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
