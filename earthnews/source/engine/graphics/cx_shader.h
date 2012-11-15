//
//  cx_shader.h
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_SHADER_H
#define CX_SHADER_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../system/cx_system.h"
#include "../system/cx_matrix3x3.h"
#include "../system/cx_matrix4x4.h"
#include "cx_texture.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SHADER_DEBUG               (CX_DEBUG && 1)
#define CX_SHADER_DEBUG_LOG_ENABLED   (CX_SHADER_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// vertex attributes
typedef enum cx_shader_attribute
{
  CX_SHADER_ATTRIBUTE_INVALID = -1,
  CX_SHADER_ATTRIBUTE_POSITION,
  CX_SHADER_ATTRIBUTE_NORMAL,
  CX_SHADER_ATTRIBUTE_TEXCOORD,
  CX_SHADER_ATTRIBUTE_TANGENT,
  CX_SHADER_ATTRIBUTE_COLOUR,
  CX_NUM_SHADER_ATTRIBUTES
} 
cx_shader_attribute;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// uniform index
typedef enum cx_shader_uniform
{
  CX_SHADER_UNIFORM_INVALID = -1,
  CX_SHADER_UNIFORM_TRANSFORM_P,        // mat4x4
  CX_SHADER_UNIFORM_TRANSFORM_MV,       // mat4x4
  CX_SHADER_UNIFORM_TRANSFORM_MVP,      // mat4x4
  CX_SHADER_UNIFORM_TRANSFORM_N,        // mat3x3
  CX_SHADER_UNIFORM_CAMERA_POSITION,    // vec4
  CX_SHADER_UNIFORM_LIGHT_POSITION,     // vec4
  CX_SHADER_UNIFORM_LIGHT_DIRECTION,    // vec4
  CX_SHADER_UNIFORM_DIFFUSE_MAP,        // texture (sampler 0)
  CX_SHADER_UNIFORM_BUMP_MAP,           // texture
  CX_NUM_SHADER_UNIFORMS,
  CX_SHADER_UNIFORM_USER_DEFINED,
}
cx_shader_uniform;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_shader_built_in
{
  CX_SHADER_BUILT_IN_INVALID = -1,
  CX_SHADER_BUILT_IN_FONT,
  CX_SHADER_BUILT_IN_DRAW_QUAD,
  CX_SHADER_BUILT_IN_DRAW_QUAD_TEX,
  CX_SHADER_BUILT_IN_DRAW_POINTS,
  CX_SHADER_BUILT_IN_DRAW_POINTS_TEX,
  CX_SHADER_BUILT_IN_DRAW_LINES,
  
  CX_NUM_BUILT_IN_SHADERS
} cx_shader_built_in;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_shader
{
  cxu32 program;
  cxi32 attributes [CX_NUM_SHADER_ATTRIBUTES];
  cxi32 uniforms [CX_NUM_SHADER_UNIFORMS];
  char *name;
  
} cx_shader;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_create (const char *name, const char *dir);
void cx_shader_destroy (cx_shader *shader);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_use (const cx_shader *shader);
void cx_shader_set_uniform (const cx_shader *shader, enum cx_shader_uniform uniform, const void *data);
void cx_shader_set_float (const cx_shader *shader, const char *name, cxf32 *f, cxi32 count);
void cx_shader_set_vector2 (const cx_shader *shader, const char *name, const cx_vec2 *vec2, cxi32 count);
void cx_shader_set_vector4 (const cx_shader *shader, const char *name, const cx_vec4 *vec4, cxi32 count);
void cx_shader_set_matrix3x3 (const cx_shader *shader, const char *name, const cx_mat3x3 *mat3x3, cxi32 count);
void cx_shader_set_matrix4x4 (const cx_shader *shader, const char *name, const cx_mat4x4 *mat4x4, cxi32 count);
void cx_shader_set_texture (const cx_shader *shader, const char *name, const cx_texture *texture, cxi32 sampler);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_get_built_in (cx_shader_built_in type);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_shader_init (void);
bool _cx_shader_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
