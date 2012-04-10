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

#define CX_FONT_TEXTURE_WIDTH       512
#define CX_FONT_TEXTURE_HEIGHT      512

#define CX_FONT_MAX_NUM_FONT_CHARS  96
#define CX_FONT_FIRST_CHAR          32

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_font_impl_stb
{
  stbtt_bakedchar *ttfCharData; 
  cx_texture *texture;
  cx_material *material;
} cx_font_impl;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font *cx_font_create (const char *filename, cxf32 fontSize, cx_shader *shader)
{
  CX_ASSERT (filename);
  CX_ASSERT (shader);
  
  const char *fontname = filename;
  
  char mname [64];
  cx_sprintf (mname, 64, "font-%s-material", fontname);
  
  cx_file file;
  
  bool loaded = cx_file_load (&file, filename);
  CX_ASSERT (loaded);
  CX_REFERENCE_UNUSED_VARIABLE (loaded);
  
  cx_font_impl *fontImpl   = (cx_font_impl *) cx_malloc (sizeof (cx_font_impl));
  fontImpl->ttfCharData    = (stbtt_bakedchar *) cx_malloc (96 * sizeof (stbtt_bakedchar));
  fontImpl->texture        = cx_texture_create (CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, 
                                                 CX_TEXTURE_FORMAT_ALPHA);
  
  stbtt_BakeFontBitmap (file.data, 0, fontSize, fontImpl->texture->data, 
                        CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, CX_FONT_FIRST_CHAR, CX_FONT_MAX_NUM_FONT_CHARS, fontImpl->ttfCharData);
  
  cx_texture_gpu_init (fontImpl->texture);
  
  fontImpl->material       = cx_material_create (mname);
  
  cx_material_attach_texture (fontImpl->material, fontImpl->texture, CX_MATERIAL_TEXTURE_AMBIENT);
  cx_material_set_properties (fontImpl->material, CX_MATERIAL_PROPERTY_AMBIENT);

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

void cx_font_render (const cx_font *font, const char *text, cxf32 x, cxf32 y, const cx_colour *colour)
{
  CX_ASSERT (font);
  CX_ASSERT (font->fontImpl);
  CX_ASSERT (text);
  CX_ASSERT (colour);
  
  cx_font_impl *fontImpl = (cx_font_impl *) font->fontImpl;
  cx_material *material = fontImpl->material;
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_FONT);
  
  // use shader
  cx_shader_use (shader);
  
  // set texture
  cx_material_render (material, shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_graphics_get_mvp_matrix (&mvp);
  cx_shader_write_to_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);

  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  char c = *text;
  while (c)
  {
    if ((c >= CX_FONT_FIRST_CHAR) && (c <= (CX_FONT_FIRST_CHAR + CX_FONT_MAX_NUM_FONT_CHARS)))
    {
      cx_vec2 pos [4];
      cx_vec2 uv [4];
      
      stbtt_aligned_quad quad;
      stbtt_GetBakedQuad (fontImpl->ttfCharData, CX_FONT_TEXTURE_WIDTH, CX_FONT_TEXTURE_HEIGHT, (c - CX_FONT_FIRST_CHAR), &x, &y, &quad, 1);
      
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
    
    ++text;
    c = *text;
  }
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
