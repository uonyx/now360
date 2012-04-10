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

#include "cx_system.h"
#include "cx_colour.h"
#include "cx_vector4.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SHADER_DEBUG     (CX_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

// Vertex attributes
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

// Uniform index.
typedef enum cx_shader_uniform
{
  CX_SHADER_UNIFORM_INVALID = -1,
  CX_SHADER_UNIFORM_TRANSFORM_M,
  CX_SHADER_UNIFORM_TRANSFORM_MV,
  CX_SHADER_UNIFORM_TRANSFORM_MVP,
  CX_SHADER_UNIFORM_TRANSFORM_N,
  
  CX_SHADER_UNIFORM_EYE,
  CX_SHADER_UNIFORM_LIGHT_POSITION,
  CX_SHADER_UNIFORM_LIGHT_DIRECTION,
  
  CX_SHADER_UNIFORM_SAMPLER2D_0,
  CX_SHADER_UNIFORM_SAMPLER2D_1,
  CX_SHADER_UNIFORM_SAMPLER2D_2,
  CX_SHADER_UNIFORM_SAMPLER2D_3,
  
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
  CX_SHADER_BUILT_IN_DRAW,
  CX_SHADER_BUILT_IN_DRAW_TEX,
  
  CX_NUM_BUILT_IN_SHADERS
} cx_shader_built_in;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_shader_datatype
{
  CX_SHADER_DATATYPE_VECTOR3,
  CX_SHADER_DATATYPE_VECTOR4,
  CX_SHADER_DATATYPE_MATRIX3X3,
  CX_SHADER_DATATYPE_MATRIX4X4,
}
cx_shader_datatype;

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

cx_shader *cx_shader_get_built_in (cx_shader_built_in type);

void cx_shader_use (const cx_shader *shader);
void cx_shader_write_to_uniform (const cx_shader *shader, enum cx_shader_uniform uniform, cx_shader_datatype type, void *data);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_init (void);
void cx_shader_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
