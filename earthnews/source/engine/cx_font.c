//
//  cx_font.c
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "cx_font.h"
#include "cx_file.h"
#include "cx_matrix4x4.h"
#include "cx_graphics.h"

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype/stb_truetype.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FONT_TEXTURE_WIDTH       (512)
#define CX_FONT_TEXTURE_HEIGHT      (512)

#define CX_FONT_MAX_NUM_FONT_CHARS  (96)
#define CX_FONT_FIRST_CHAR          (32)
#define CX_FONT_LAST_CHAR           (CX_FONT_FIRST_CHAR + CX_FONT_MAX_NUM_FONT_CHARS)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_font_impl_stb
{
  stbtt_bakedchar *ttfCharData; 
  cx_texture *texture;
  cxf32 scaleX, scaleY;
} cx_font_impl;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font *cx_font_create (const char *filename, cxf32 fontSize)
{
  CX_ASSERT (filename);

  cx_file file;
  
  bool loaded = cx_file_load (&file, filename);
  CX_ASSERT (loaded);
  CX_REFERENCE_UNUSED_VARIABLE (loaded);
  
  cx_font_impl *fontImpl   = (cx_font_impl *) cx_malloc (sizeof (cx_font_impl));
  fontImpl->scaleX         = 1.0f;
  fontImpl->scaleY         = 1.0f;
  fontImpl->ttfCharData    = (stbtt_bakedchar *) cx_malloc (96 * sizeof (stbtt_bakedchar));
  fontImpl->texture        = cx_texture_create (CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, 
                                                 CX_TEXTURE_FORMAT_ALPHA);
  
  stbtt_BakeFontBitmap (file.data, 0, fontSize, fontImpl->texture->data, 
                        CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, CX_FONT_FIRST_CHAR, CX_FONT_MAX_NUM_FONT_CHARS, fontImpl->ttfCharData);
  
  cx_texture_gpu_init (fontImpl->texture);

  cx_font *font   = (cx_font *) cx_malloc (sizeof (cx_font));
  font->size      = fontSize;
  font->fontImpl  = fontImpl;
  
  cx_file_unload (&file);
  
  return font;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_destroy (cx_font *font)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontImpl);
  
  cx_font_impl *font_impl = (cx_font_impl *) font->fontImpl;
  
  cx_texture_destroy (font_impl->texture);
  
  cx_free (font_impl->ttfCharData);
  cx_free (font->fontImpl);
  cx_free (font);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_set_scale (const cx_font *font, cxf32 x, cxf32 y)
{
  CX_ASSERT (font);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontImpl;
  
  fontImpl->scaleX = x;
  fontImpl->scaleY = y;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_font_render (const cx_font *font, const char *text, cxf32 x, cxf32 y, cx_font_alignment alignment, const cx_colour *colour)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontImpl);
  CX_ASSERT (text);
  CX_ASSERT (colour);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontImpl;
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_FONT);
  
  // use shader
  cx_shader_use (shader);
  
  // set texture
  cx_material_render_texture (fontImpl->texture, CX_MATERIAL_TEXTURE_AMBIENT, shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);

  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  cxf32 px = x;
  cxf32 py = y;
  cxf32 sx = fontImpl->scaleX;
  cxf32 sy = fontImpl->scaleY;
  
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
  
  char c = 0;
  while ((c = *text++))
  {
    if ((c >= CX_FONT_FIRST_CHAR) && (c <= CX_FONT_LAST_CHAR))
    {
      cx_vec2 pos [4];
      cx_vec2 uv [4];
      
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, (c - CX_FONT_FIRST_CHAR), sx, sy, &px, &py, &quad, 1);
      
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
      
      cx_graphics_assert_no_errors ();
      
      glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
      cx_graphics_assert_no_errors ();
    }
  }
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_font_get_text_width (const cx_font *font, const char *text)
{
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontImpl;
  CX_ASSERT (fontImpl);
  
  cxf32 sx = fontImpl->scaleX;
  cxf32 width = 0.0f;
  char c = 0;
  while ((c = *text++))
  {
    if ((c >= CX_FONT_FIRST_CHAR) && (c <= CX_FONT_LAST_CHAR))
    {
      stbtt_bakedchar *bakedChar = fontImpl->ttfCharData + (c - CX_FONT_FIRST_CHAR);
      width += bakedChar->xadvance * sx;
    }
    
    ++text;
    c = *text;
  }
  
  return width;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
