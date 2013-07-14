//
//  cx_font.c
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "../system/cx_file.h"
#include "../system/cx_matrix4x4.h"
#include "../system/cx_vector2.h"
#include "../3rdparty/stb/stb_truetype.h"

#include "cx_font.h"
#include "cx_gdi.h"
#include "cx_shader.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FONT_TEXTURE_WIDTH       (512)
#define CX_FONT_TEXTURE_HEIGHT      (512)
#define CX_FONT_MAX_NUM_FONT_CHARS  (256)
#define CX_FONT_BEGIN_CHAR          (32)
#define CX_FONT_END_CHAR            (CX_FONT_BEGIN_CHAR + CX_FONT_MAX_NUM_FONT_CHARS)
#define CX_FONT_MAX_TEXT_LENGTH     (512)
#define CX_FONT_BATCH_CHAR_RENDER   (1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_font_impl_stb
{
  stbtt_bakedchar *ttfCharData; 
  cx_texture *texture;
  cxf32 scaleX, scaleY;
  cxf32 height;
} cx_font_impl;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font *cx_font_create (const char *filename, cxf32 fontSize)
{
  CX_ASSERT (filename);

  cx_font *font = NULL;
  
  cxu8 *filedata = NULL;
  cxu32 filedataSize = 0;
  
  if (cx_file_storage_load_contents (&filedata, &filedataSize, filename, CX_FILE_STORAGE_BASE_RESOURCE))
  { 
    cx_font_impl *fontImpl   = (cx_font_impl *) cx_malloc (sizeof (cx_font_impl));
    fontImpl->scaleX         = 1.0f;
    fontImpl->scaleY         = 1.0f;
    fontImpl->height         = fontSize;
    fontImpl->ttfCharData    = (stbtt_bakedchar *) cx_malloc (CX_FONT_MAX_NUM_FONT_CHARS * sizeof (stbtt_bakedchar));
    fontImpl->texture        = cx_texture_create (CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, 
                                                   CX_TEXTURE_FORMAT_ALPHA);
    
    stbtt_BakeFontBitmap (filedata,
                          0, 
                          fontSize, 
                          fontImpl->texture->data, 
                          CX_FONT_TEXTURE_WIDTH, 
                          CX_FONT_TEXTURE_HEIGHT, 
                          CX_FONT_BEGIN_CHAR, 
                          CX_FONT_MAX_NUM_FONT_CHARS, 
                          fontImpl->ttfCharData);
    
    cx_texture_gpu_init (fontImpl->texture, true);
    cx_texture_data_destroy (fontImpl->texture);

    font = (cx_font *) cx_malloc (sizeof (cx_font));
    font->fontdata  = fontImpl;
  
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
  
  fontImpl->scaleX = cx_clamp (x, 0.0f, 1.0f);;
  fontImpl->scaleY = cx_clamp (y, 0.0f, 1.0f);;
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
#if CX_FONT_BATCH_CHAR_RENDER
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
#else
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
#endif

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
  
  // convert from utf8 to unicode
  cxu32 textBuffer [CX_FONT_MAX_TEXT_LENGTH];
  cxu32 tLen = cx_str_utf8_to_unicode (textBuffer, CX_FONT_MAX_TEXT_LENGTH, text);
  CX_REF_UNUSED (tLen);
  
  cxu32 *t = textBuffer;
  cxu32 c = 0;

#if CX_FONT_BATCH_CHAR_RENDER
  
  cx_vec2 pos [CX_FONT_MAX_TEXT_LENGTH * 4];
  cx_vec2 uv [CX_FONT_MAX_TEXT_LENGTH * 4];
  
  cxu32 len = 0;

  while ((c = *t++))
  {
    if ((c >= CX_FONT_BEGIN_CHAR) && (c < CX_FONT_END_CHAR))
    {
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, 
                          (c - CX_FONT_BEGIN_CHAR), sx, sy, &px, &py, &quad, 1);
      
      int i = len * 4;
      
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
      
      ++len;
    }
  }
  
  // batch triangestrip points
  //(012) (213) (234) (435) ... total number of trianges == points - 2 == (len * 4) - 2
  
  cxu32 numTri = (len * 4) - 2;
  cxu32 numPoints = numTri * 3;
  
  
  cx_vec2 pos2 [numPoints];
  cx_vec2 uv2 [numPoints];
  cx_colour col [numPoints];
  
  cxu32 tri = 0;
  cxu32 tript = 0;
  cxu32 ni = 0;
  
  cx_vec2 zero;
  cx_vec2_set (&zero, 0.0f, 0.0f);
  
  const cx_colour *null = cx_colour_null ();
  
  bool spacingQuad = false; // space between characters
  
  while (tri < numTri)
  {
    cxu32 sa0 = tript;      // 0
    cxu32 sa1 = sa0 + 1;    // 1
    cxu32 sa2 = sa1 + 1;    // 2
    
    cxu32 sa3 = sa2;        // 2
    cxu32 sa4 = sa1;        // 1
    cxu32 sa5 = sa3 + 1;    // 3
   
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa0];
    uv2 [ni++] = uv [sa0];
    
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa1];
    uv2 [ni++] = uv [sa1];
    
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa2];
    uv2 [ni++] = uv [sa2];
    
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa3];
    uv2 [ni++] = uv [sa3];
    
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa4];
    uv2 [ni++] = uv [sa4];
    
    col [ni] = spacingQuad ? *null : *colour;
    pos2 [ni] = pos [sa5];
    uv2 [ni++] = uv [sa5];
    
    tript = sa3;
    
    tri += 2;
    
    spacingQuad = !spacingQuad;
  }
  
  if (len > 0)
  {
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
  
#else
  while ((c = *t++))
  {
    if ((c >= CX_FONT_BEGIN_CHAR) && (c < CX_FONT_END_CHAR))
    {
      cx_vec2 pos [4];
      cx_vec2 uv [4];
      
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT,
                          (c - CX_FONT_BEGIN_CHAR), sx, sy, &px, &py, &quad, 1);
      
      pos [0].x = quad.x0;
      pos [0].y = quad.y0;
      pos [1].x = quad.x0;
      pos [1].y = quad.y1;
      pos [2].x = quad.x1;
      pos [2].y = quad.y0;
      pos [3].x = quad.x1;
      pos [3].y = quad.y1;
      
      uv [0].x = quad.s0;
      uv [0].y = quad.t0;
      uv [1].x = quad.s0;
      uv [1].y = quad.t1;
      uv [2].x = quad.s1;
      uv [2].y = quad.t0;
      uv [3].x = quad.s1;
      uv [3].y = quad.t1;
      
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
      
      cx_gdi_assert_no_errors ();
      
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
      cx_gdi_assert_no_errors ();
    }
  }
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
#endif

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
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
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
  
  char textBuffer [CX_FONT_MAX_TEXT_LENGTH];
  cx_strcpy (textBuffer, CX_FONT_MAX_TEXT_LENGTH, text);
  
  unsigned char c = 0;
  char *t = textBuffer; 
  char *lastSpacePos = NULL;
  while ((c = *t))
  {
    if ((c >= CX_FONT_BEGIN_CHAR) && (c < CX_FONT_END_CHAR))
    {
      stbtt_bakedchar *bakedChar = fontImpl->ttfCharData + (c - CX_FONT_BEGIN_CHAR);
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
    else if ((c >= CX_FONT_BEGIN_CHAR) && (c < CX_FONT_END_CHAR))
    {
      cx_vec2 pos [4];
      cx_vec2 uv [4];
      
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, 
                          (c - CX_FONT_BEGIN_CHAR), sx, sy, &px, &py, &quad, 1);
      
      pos [0].x = quad.x0;
      pos [0].y = quad.y0;
      pos [1].x = quad.x0;
      pos [1].y = quad.y1;
      pos [2].x = quad.x1;
      pos [2].y = quad.y0;
      pos [3].x = quad.x1;
      pos [3].y = quad.y1;
      
      uv [0].x = quad.s0;
      uv [0].y = quad.t0;
      uv [1].x = quad.s0;
      uv [1].y = quad.t1;
      uv [2].x = quad.s1;
      uv [2].y = quad.t0;
      uv [3].x = quad.s1;
      uv [3].y = quad.t1;
      
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
      
      cx_gdi_assert_no_errors ();
      
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
      cx_gdi_assert_no_errors ();
    }
  }
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
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
  unsigned char c = 0;
  while ((c = *text++))
  {
    if ((c >= CX_FONT_BEGIN_CHAR) && (c < CX_FONT_END_CHAR))
    {
      stbtt_bakedchar *bakedChar = fontImpl->ttfCharData + (c - CX_FONT_BEGIN_CHAR);
      width += bakedChar->xadvance * sx;
    }
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
