//
//  cx_draw.c
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "../system/cx_vector2.h"
#include "../system/cx_matrix4x4.h"
#include "cx_draw.h"
#include "cx_gdi.h"
#include "cx_shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_colour (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, const cx_colour *colour);
void cx_draw_quad_texture (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, 
                           const cx_colour *colour, const cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_lines (cxi32 numLines, const cx_line *lines, const cx_colour *colour, cxf32 scale)
{
  CX_ASSERT (numLines > 0);
  CX_ASSERT (scale > 0.0f);
  CX_ASSERT (lines);
  CX_ASSERT (colour);
  
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvp);
  
  cxi32 numSegments = numLines * 2;
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_LINES);
  CX_ASSERT (shader);
  
  cx_shader_begin (shader);
  
  // set mvp
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
  
  // line width
  glLineWidth (scale);
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, 0, lines);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_LINES, 0, numSegments);
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  
  cx_shader_end (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_points (cxi32 numPoints, const cx_vec4 *points, const cx_colour *colours, const cx_texture *texture, cxf32 scale)
{
  CX_ASSERT (numPoints > 0);
  CX_ASSERT (points);
  CX_ASSERT (colours);
  
  cx_shader_built_in shaderType = texture ? CX_SHADER_BUILT_IN_DRAW_POINTS_TEX : CX_SHADER_BUILT_IN_DRAW_POINTS;
  
  cx_shader *shader = cx_shader_get_built_in (shaderType);
  CX_ASSERT (shader);
  
  cx_shader_begin (shader);
  
  if (texture)
  {
    cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_DIFFUSE_MAP, texture);
  }
  
  // set mvp
  cx_mat4x4 p, mv;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MV, &mv);
  cx_gdi_get_transform (CX_GDI_TRANSFORM_P, &p);
  
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MV, &mv);
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_P, &p);
  
  // set scene width
  cxf32 sceneWidth = cx_gdi_get_screen_height () * 0.5f;
  cx_shader_set_float (shader, "u_sw", &sceneWidth, 1);
  
  // set scale
  cx_shader_set_float (shader, "u_pw", &scale, 1);
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, 0, points);
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], 4, GL_FLOAT, GL_FALSE, 0, colours);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_POINTS, 0, numPoints);
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR]);
  
  cx_shader_end (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_colour (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, const cx_colour *colour)
{
  CX_ASSERT (colour);
  
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvp);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_QUAD);
  
  // enable shader
  cx_shader_begin (shader);
  
  // set mvp

  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
  
  // set depth
  cx_shader_set_float (shader, "u_z", &z, 1);
  
  // position attribute
  cx_vec2 pos [4];
  
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
  
  // rotation by r radians about the origin ox, oy
  cxf32 ox = (x1 + x2) * 0.5f;
  cxf32 oy = (y1 + y2) * 0.5f;
  
  cxf32 sin = cx_sin (r);
  cxf32 cos = cx_cos (r);
  
  for (cxu8 i = 0; i < 4; ++i)
  {
    cxf32 vx = pos [i].x - ox;
    cxf32 vy = pos [i].y - oy;
    
    cxf32 rx = (cos * vx) - (sin * vy);
    cxf32 ry = (sin * vx) + (cos * vy);
    
    pos [i].x = rx + ox;
    pos [i].y = ry + oy;
  }
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  cx_gdi_assert_no_errors ();
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_gdi_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  
  cx_shader_end (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_texture (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, 
                           cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (colour);
  CX_ASSERT (texture);
  
  // get mvp
  cx_mat4x4 mvp;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvp);
  
  cx_shader *shader = cx_shader_get_built_in (CX_SHADER_BUILT_IN_DRAW_QUAD_TEX);
  
  // enable shader
  cx_shader_begin (shader);
  
  // render texture
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_DIFFUSE_MAP, texture);
  
  // set mvp
  cx_shader_set_uniform (shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
  
  // set depth
  cx_shader_set_float (shader, "u_z", &z, 1);
  
  cx_vec2 uv [4], pos [4];

  uv [0].x = u1;
  uv [0].y = v1;
  uv [1].x = u1;
  uv [1].y = v2;
  uv [2].x = u2;
  uv [2].y = v1;
  uv [3].x = u2;
  uv [3].y = v2;
  
  pos [0].x = x1;
  pos [0].y = y1;
  pos [1].x = x1;
  pos [1].y = y2;
  pos [2].x = x2;
  pos [2].y = y1;
  pos [3].x = x2;
  pos [3].y = y2;
  
  cxf32 ox = (x1 + x2) * 0.5f;
  cxf32 oy = (y1 + y2) * 0.5f;
  
  cxf32 sin = cx_sin (r);
  cxf32 cos = cx_cos (r);
  
  for (cxu8 i = 0; i < 4; ++i)
  {
    cxf32 vx = pos [i].x - ox;
    cxf32 vy = pos [i].y - oy;
    
    cxf32 rx = (cos * vx) - (sin * vy);
    cxf32 ry = (sin * vx) + (cos * vy);
    
    pos [i].x = rx + ox;
    pos [i].y = ry + oy;
  }
  
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
  
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
  glVertexAttrib4fv (shader->attributes [CX_SHADER_ATTRIBUTE_COLOUR], colour->f4);
  cx_gdi_assert_no_errors ();
  
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 2, GL_FLOAT, GL_FALSE, 0, pos);
  glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, uv);
  cx_gdi_assert_no_errors ();
  
  glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
  cx_gdi_assert_no_errors ();
  
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  glDisableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
  
  cx_shader_end (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, const cx_colour *colour, const cx_texture *texture)
{
  if (texture)
  {
    cx_draw_quad_texture (x1, y1, x2, y2, z, r, 0.0f, 0.0f, 1.0f, 1.0f, colour, texture);
  }
  else 
  {
    cx_draw_quad_colour (x1, y1, x2, y2, z, r, colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad_uv (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 r, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, 
                    const cx_colour *colour, const cx_texture *texture)
{
  CX_ASSERT (texture);
  
  cx_draw_quad_texture (x1, y1, x2, y2, z, r, u1, v1, u2, v2, colour, texture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

