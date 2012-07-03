//
//  cx_draw.c
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_draw.h"
#include "cx_matrix4x4.h"
#include "cx_graphics.h"
#include "cx_shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_points_colour (cxi32 numPoints, const cx_vec4 *pos, const cx_colour *colour)
{
  CX_ASSERT (numPoints > 0);
  CX_ASSERT (pos);
  CX_ASSERT (colour);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_POINTS);
  CX_ASSERT (shader);
  
  cx_shader_use (shader);
  
  // set mvp
  cx_mat4x4 mvp, p, mv;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MV, &mv);
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_P, &p);
  
  //cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MV, CX_SHADER_DATATYPE_MATRIX4X4, mv.f16);
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_P, CX_SHADER_DATATYPE_MATRIX4X4, p.f16);
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, 0, (void*) pos);
  cx_graphics_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_graphics_assert_no_errors ();
  
  glDrawArrays (GL_POINTS, 0, numPoints);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_colour (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, const cx_colour *colour)
{
  CX_ASSERT (colour);
  
  //cx_graphics_unbind_all_buffers ();
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW);
  
  // enable shader
  cx_shader_use (shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  cx_vec2 pos [4];
  
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  cx_graphics_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_graphics_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_graphics_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_texture (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (colour);
  CX_ASSERT (texture);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_TEX);
  
  // enable shader
  cx_shader_use (shader);
  
  // render texture
  cx_material_render_texture (texture, CX_MATERIAL_TEXTURE_AMBIENT, shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
  cx_vec2 pos [4];
  cx_vec2 uv [4];
  
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
  
  // top left = (0,0)
  uv [0].x = 0.0f;
  uv [0].y = 0.0f;
  uv [1].x = 0.0f;
  uv [1].y = 1.0f;
  uv [2].x = 1.0f;
  uv [2].y = 0.0f;
  uv [3].x = 1.0f;
  uv [3].y = 1.0f;

  /*
  // bottom left = (0,0)
  uv [0].x = 0.0f;
  uv [0].y = 1.0f;
  uv [1].x = 0.0f;
  uv [1].y = 0.0f;
  uv [2].x = 1.0f;
  uv [2].y = 1.0f;
  uv [3].x = 1.0f;
  uv [3].y = 0.0f;
  */
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
  cx_graphics_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_graphics_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_texture2 (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (colour);
  CX_ASSERT (texture);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_TEX);
  
  // enable shader
  cx_shader_use (shader);
  
  // render texture
  cx_material_render_texture (texture, CX_MATERIAL_TEXTURE_AMBIENT, shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_graphics_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
  cx_vec2 pos [4];
  cx_vec2 uv [4];
  
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
  
  uv [0].x = u1;
  uv [0].y = v1;
  uv [1].x = u1;
  uv [1].y = v2;
  uv [2].x = u2;
  uv [2].y = v1;
  uv [3].x = u2;
  uv [3].y = v2;
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_graphics_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
  cx_graphics_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_graphics_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
