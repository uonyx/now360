//
//  cx_draw.c
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "../system/cx_matrix4x4.h"
#include "cx_draw.h"
#include "cx_gdi.h"
#include "cx_shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_colour (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, const cx_colour *colour);
void cx_draw_quad_texture (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, 
                           const cx_colour *colour, const cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_points (cxi32 numPoints, const cx_vec4 *pos, const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (numPoints > 0);
  CX_ASSERT (pos);
  CX_ASSERT (colour);
  
  cx_shader_built_in shaderType = texture ? CX_SHADER_BUILT_IN_DRAW_POINTS_TEX : CX_SHADER_BUILT_IN_DRAW_POINTS;
  
  cx_shader *shader = cx_shader_get_built_in (shaderType);
  CX_ASSERT (shader);
  
  cx_shader_use (shader);
  
  if (texture)
  {
    cx_material_render_texture (texture, CX_MATERIAL_TEXTURE_AMBIENT, shader);
  }
  
  // set mvp
  cx_mat4x4 mvp, p, mv;
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_MV, &mv);
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_P, &p);
  
  //cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MV, CX_SHADER_DATATYPE_MATRIX4X4, mv.f16);
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_P, CX_SHADER_DATATYPE_MATRIX4X4, p.f16);
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, 0, (void*) pos);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_POINTS, 0, numPoints);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_colour (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, const cx_colour *colour)
{
  CX_ASSERT (colour);
  
  //cx_gdi_unbind_all_buffers ();
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_QUAD);
  
  // enable shader
  cx_shader_use (shader);
  
  // set mvp
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  cx_shader_set_uniform_2 (shader, "u_z", CX_SHADER_DATATYPE_FLOAT, (void *) &z, 1);
  
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
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_gdi_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_texture (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (colour);
  CX_ASSERT (texture);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_QUAD_TEX);
  
  // enable shader
  cx_shader_use (shader);
  
  // render texture
  cx_material_render_texture (texture, CX_MATERIAL_TEXTURE_AMBIENT, shader);
  
#if 0
  float rotangleDeg = 5.0f;
  
  cxf32 rad = cx_rad (rotangleDeg);
  
  cx_mat4x4 p, v, mvp;
  
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_P, &p);
  
  cx_mat4x4_rotation_axis_z (&v, rad);
  
  cx_mat4x4_mul (&mvp, &p, &v);
  
#endif
  
#if 0
  float rotangleDeg = 0.0f;
  
  cxf32 rad = cx_rad (rotangleDeg);
  cxf32 sin = cx_sin (rad);
  cxf32 cos = cx_cos (rad);
  //cxf32 rotMatrix [4] = { cos, sin, -sin, cos };

  cxf32 p0x = (cos * x1) - (sin * y1);
  cxf32 p0y = (sin * x1) + (cos * y1);

  cxf32 p1x = (cos * x1) - (sin * y2);
  cxf32 p1y = (sin * x1) + (cos * y2);
  
  cxf32 p2x = (cos * x2) - (sin * y1);
  cxf32 p2y = (sin * x2) + (cos * y1);
  
  cxf32 p3x = (cos * x2) - (sin * y2);
  cxf32 p3y = (sin * x2) + (cos * y2);
  
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_P, &mvp);
#endif
  
  // set mvp
#if 1
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GRAPHICS_TRANSFORM_MVP, &mvp);
#endif
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, CX_SHADER_DATATYPE_MATRIX4X4, mvp.f16);
  
  cx_shader_set_uniform_2 (shader, "u_z", CX_SHADER_DATATYPE_FLOAT, (void *) &z, 1);
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
  cx_vec2 pos [4];
  cx_vec2 uv [4];
  
#if 0
  pos [0].x = p0x;
  pos [0].y = p0y;
  pos [1].x = p1x;
  pos [1].y = p1y;
  pos [2].x = p2x;
  pos [2].y = p2y;
  pos [3].x = p3x;
  pos [3].y = p3y;
#else
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
#endif
  
  uv [0].x = u1;
  uv [0].y = v1;
  uv [1].x = u1;
  uv [1].y = v2;
  uv [2].x = u2;
  uv [2].y = v1;
  uv [3].x = u2;
  uv [3].y = v2;
  
  /*
  // top left = (0,0)
  uv [0].x = 0.0f;
  uv [0].y = 0.0f;
  uv [1].x = 0.0f;
  uv [1].y = 1.0f;
  uv [2].x = 1.0f;
  uv [2].y = 0.0f;
  uv [3].x = 1.0f;
  uv [3].y = 1.0f;
  */
  
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
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_gdi_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, const cx_colour *colour, const cx_texture *texture)
{
  if (texture)
  {
    cx_draw_quad_texture (x1, y1, x2, y2, z, 0.0f, 0.0f, 1.0f, 1.0f, colour, texture);
  }
  else 
  {
    cx_draw_quad_colour (x1, y1, x2, y2, z, colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad2 (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, 
                    const cx_colour *colour, const cx_texture *texture)
{
  cx_draw_quad_texture (x1, y1, x2, y2, z, u1, v1, u2, v2, colour, texture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

