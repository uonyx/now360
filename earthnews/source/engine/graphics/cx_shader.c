//
//  cx_gdi_shader.c
//
//  Created by Ubaka Onyechi on 29/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "../system/cx_file.h"
#include "../system/cx_json.h"
#include "../system/cx_string.h"

#include "cx_shader.h"
#include "cx_gdi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SHADER_VERTEX_SHADER_FILE_EXTENSION    "vsh"
#define CX_SHADER_FRAGMENT_SHADER_FILE_EXTENSION  "fsh"
#define CX_SHADER_CONFIGURATION_FILE_EXTENSION    "cfg"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool g_initialised = false;

static const char *g_attribEnumStrings [CX_NUM_SHADER_ATTRIBUTES] =
{
  "CX_SHADER_ATTRIBUTE_POSITION",
  "CX_SHADER_ATTRIBUTE_NORMAL",
  "CX_SHADER_ATTRIBUTE_TEXCOORD",
  "CX_SHADER_ATTRIBUTE_TANGENT",
  "CX_SHADER_ATTRIBUTE_BITANGENT",
  "CX_SHADER_ATTRIBUTE_COLOUR",
};

static const char *g_uniformEnumStrings [] =
{  
  "CX_SHADER_UNIFORM_TRANSFORM_P",    
  "CX_SHADER_UNIFORM_TRANSFORM_MV",   
  "CX_SHADER_UNIFORM_TRANSFORM_MVP",  
  "CX_SHADER_UNIFORM_TRANSFORM_N",    
  "CX_SHADER_UNIFORM_EYE_POSITION",
  "CX_SHADER_UNIFORM_LIGHT_POSITION", 
  "CX_SHADER_UNIFORM_DIFFUSE_MAP",
  "CX_SHADER_UNIFORM_SPECULAR_MAP",
  "CX_SHADER_UNIFORM_BUMP_MAP",
  "CX_NUM_SHADER_UNIFORMS",
  "CX_SHADER_UNIFORM_USER_DEFINED",
};

static GLenum g_glTextureUnits [] = 
{
  GL_TEXTURE0,
  GL_TEXTURE1,
  GL_TEXTURE2,
  GL_TEXTURE3,
  GL_TEXTURE4,
  GL_TEXTURE5,
  GL_TEXTURE6,
  GL_TEXTURE7,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct cx_shader_description
{
  cx_shader_built_in type;
  const char *name;
  const char *dir;
};

static struct cx_shader_description g_shaderDescriptions [CX_NUM_BUILT_IN_SHADERS] = 
{
  { CX_SHADER_BUILT_IN_FONT,              "font",         "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_QUAD,         "quad",         "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_QUAD_TEX,     "quad_tex",     "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_POINTS,       "points",       "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_POINTS_TEX,   "points_tex",   "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_LINES,        "lines",        "data/shaders" },
};

static struct cx_shader *g_builtInShaders [CX_NUM_BUILT_IN_SHADERS];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_shader_built_in_init (void);
static void cx_shader_built_in_deinit (void);
static cxi32 cx_shader_get_uniform_sampler (cx_shader_uniform uniform);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_shader_init (void)
{
  CX_ASSERT (!g_initialised);
  
  memset (g_builtInShaders, 0, sizeof (g_builtInShaders));
  
  cx_shader_built_in_init ();
  
  g_initialised = true;
  
  return g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_shader_deinit (void)
{
  if (g_initialised)
  {
    cx_shader_built_in_deinit ();
    
    g_initialised = false;
  }

  return !g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_shader_attribute cx_get_shader_attribute_from_string (const char *str)
{
  int i;
  
  for (i = 0; i < CX_NUM_SHADER_ATTRIBUTES; ++i)
  {
    if (strcmp (str, g_attribEnumStrings [i]) == 0)
    {
      return i;
    }
  }
  
  return CX_SHADER_ATTRIBUTE_INVALID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_shader_uniform cx_get_shader_uniform_from_string (const char *str)
{
  for (int i = 0, c = sizeof (g_uniformEnumStrings); i < c; ++i)
  {
    if (strcmp (str, g_uniformEnumStrings [i]) == 0)
    {
      return i;
    }
  }
  
  return CX_SHADER_UNIFORM_INVALID;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_shader_compile (GLuint *shader, GLenum type, const char *buffer, cxi32 bufferSize)
{
  GLuint outShader = 0;
  GLint compiled = 0;
#if CX_DEBUG
  GLenum error;
#endif
  
  outShader = glCreateShader (type);
  
#if CX_DEBUG
  error = glGetError ();
  CX_ASSERT (error == GL_NO_ERROR); 
  CX_ASSERT (outShader != 0);
#endif
  
  glShaderSource (outShader, 1, &buffer, &bufferSize);
  
  glCompileShader (outShader);
  
  glGetShaderiv (outShader, GL_COMPILE_STATUS, &compiled);
  
  if (!compiled) 
  {
#if CX_DEBUG
    GLint logLength = 0;
    glGetShaderiv (outShader, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0) 
    {
      char *log = (char *) cx_malloc (sizeof(char) * logLength);
      glGetShaderInfoLog (outShader, logLength, &logLength, log);
      CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "Shader compile log:\n%s", log);
      cx_free (log);
    }
#endif
    
    glDeleteShader (outShader);
    return false;
  }
  
  *shader = outShader;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_shader_link (GLuint *program, GLuint vertexShader, GLuint fragmentShader)
{
  CX_ASSERT (vertexShader != 0);
  CX_ASSERT (fragmentShader != 0);
  
  GLuint outProgram = 0;
  GLint linked = 0;
  
  outProgram = glCreateProgram ();
  CX_ASSERT (outProgram != 0);
  
  // attach vertex and fragment shader
  glAttachShader (outProgram, vertexShader);
  glAttachShader (outProgram, fragmentShader);
  
  // link program 
  glLinkProgram (outProgram);
  glGetProgramiv (outProgram, GL_LINK_STATUS, &linked);
  
  if (!linked)
  {
    // delete program and associated shaders
#if CX_DEBUG
    GLint logLength = 0;
    glGetProgramiv (outProgram, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0) 
    {
      char *log = (char *) cx_malloc (sizeof (char) * logLength);
      glGetProgramInfoLog (outProgram, logLength, &logLength, log);
      CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "Program link log:\n%s", log);
      cx_free (log);
    }
#endif
    
    glDeleteProgram (outProgram);
    return false;
  }
  
#if CX_DEBUG && 0 // disabled for iOS6
  GLint validated = 0;
  glValidateProgram (outProgram);
  glGetProgramiv (outProgram, GL_VALIDATE_STATUS, &validated);
  if (!validated)
  {
    // delete program and associated shaders
    
    GLint logLength = 0;
    glGetProgramiv (outProgram, GL_INFO_LOG_LENGTH, &logLength);
    
    if (logLength > 0) 
    {
      char *log = (char *) cx_malloc (sizeof (char) * logLength);
      glGetProgramInfoLog (outProgram, logLength, &logLength, log);
      CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "Program validate log:\n%s", log);
      cx_free (log);
    }
    
    glDeleteProgram (outProgram);
    return false;
  }
#endif
  
  //
  // get uniform locations
  //
  
  // detach vertex and fragment shaders
  glDetachShader (outProgram, vertexShader);
  glDetachShader (outProgram, fragmentShader);
  glDeleteShader (vertexShader);
  glDeleteShader (fragmentShader);
  
  *program = outProgram;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_shader_configure (const char *buffer, unsigned int bufferSize, cx_shader *shader)
{
  bool success = false;
  
  cx_json_tree jsonTree = cx_json_tree_create (buffer, bufferSize);
  
  if (jsonTree)
  {
    cx_json_node rootNode = cx_json_tree_root_node (jsonTree);
    
    {
      //
      // attributes
      //
      
      cx_json_node attributesNode = cx_json_object_child_node (rootNode, 0);
      cxu32 attributesCount = cx_json_object_length (attributesNode);
      
      for (cxu32 i = 0; i < attributesCount; ++i)
      {
        const char *attribStr = cx_json_object_child_key (attributesNode, i);
        cx_json_node attribNode = cx_json_object_child_node (attributesNode, i);
        
        int attribIdx = cx_get_shader_attribute_from_string (attribStr);
        CX_ASSERT (attribIdx != CX_SHADER_ATTRIBUTE_INVALID);
        
        const char *attribName = cx_json_value_string (attribNode);
        int attribLocation = glGetAttribLocation (shader->program, attribName);
        
        CX_ASSERT ((attribLocation >= 0) && "unused attribute");
        
        shader->attributes [attribIdx] = attribLocation;
        
        cx_gdi_assert_no_errors ();
        CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "%s: %s [%d]", attribStr, attribName, attribLocation);
      }
      
#if CX_DEBUG
      GLint activeAttribs = 0;
      glGetProgramiv (shader->program, GL_ACTIVE_ATTRIBUTES, &activeAttribs);
      CX_ASSERT (activeAttribs <= (GLint) attributesCount);
#endif
    }
    
    {
      //
      // uniforms
      //
      
      cx_json_node uniformsNode = cx_json_object_child_node (rootNode, 1);
      cxu32 uniformsCount = cx_json_object_length (uniformsNode);
      
      for (cxu32 i = 0; i < uniformsCount; ++i)
      {
        const char *uniformStr = cx_json_object_child_key (uniformsNode, i);
        cx_json_node uniformNode = cx_json_object_child_node (uniformsNode, i);
        
        int uniformIdx = cx_get_shader_uniform_from_string (uniformStr);
        CX_ASSERT (uniformIdx != CX_SHADER_UNIFORM_INVALID);
        
        if (uniformIdx != CX_SHADER_UNIFORM_USER_DEFINED)
        {
          const char *uniformName = cx_json_value_string (uniformNode);
          int uniformLocation = glGetUniformLocation (shader->program, uniformName);
          
          CX_ASSERT ((uniformLocation >= 0) && "unused uniform");
          
          shader->uniforms [uniformIdx] = uniformLocation;
          
          cx_gdi_assert_no_errors ();
          CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "%s: %s [%d]", uniformStr, uniformName, uniformLocation);
        }
      }
      
#if CX_DEBUG
      GLint activeUniforms = 0;
      glGetProgramiv (shader->program, GL_ACTIVE_UNIFORMS, &activeUniforms);
      CX_ASSERT (activeUniforms <= (GLint) uniformsCount);
#endif
    }
  
    cx_json_tree_destroy (jsonTree);
    
    success = true;
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "JSON parse error");
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_create (const char *name, const char *dir)
{
  CX_ASSERT (name != NULL);
  
  bool success = false;

  char 
    vertexShaderFilename [CX_FILENAME_MAX], 
    fragmentShaderFilename [CX_FILENAME_MAX], 
    shaderConfigFilename [CX_FILENAME_MAX];
  
  if (dir)
  {
    cx_sprintf (vertexShaderFilename, CX_FILENAME_MAX, "%s/%s.%s", dir, name, CX_SHADER_VERTEX_SHADER_FILE_EXTENSION);
    cx_sprintf (fragmentShaderFilename, CX_FILENAME_MAX, "%s/%s.%s", dir, name, CX_SHADER_FRAGMENT_SHADER_FILE_EXTENSION);
    cx_sprintf (shaderConfigFilename, CX_FILENAME_MAX, "%s/%s.%s", dir, name, CX_SHADER_CONFIGURATION_FILE_EXTENSION);
  }
  else
  {
    cx_sprintf (vertexShaderFilename, CX_FILENAME_MAX, "%s.%s", name, CX_SHADER_VERTEX_SHADER_FILE_EXTENSION);
    cx_sprintf (fragmentShaderFilename, CX_FILENAME_MAX, "%s.%s", name, CX_SHADER_FRAGMENT_SHADER_FILE_EXTENSION);
    cx_sprintf (shaderConfigFilename, CX_FILENAME_MAX, "%s.%s", name, CX_SHADER_CONFIGURATION_FILE_EXTENSION);
  }
  
  GLuint vertexShader, fragmentShader, shaderProgram;
  
  cxu8 *vsData = NULL, *fsData = NULL, *scData = NULL;
  cxu32 vsDataSize = 0, fsDataSize = 0, scDataSize = 0;
  
  bool loaded = false;
  
  loaded = cx_file_storage_load_contents (&vsData, &vsDataSize, vertexShaderFilename, CX_FILE_STORAGE_BASE_RESOURCE);
  CX_FATAL_ASSERT (loaded);

  loaded = cx_file_storage_load_contents (&fsData, &fsDataSize, fragmentShaderFilename, CX_FILE_STORAGE_BASE_RESOURCE);
  CX_FATAL_ASSERT (loaded);
  
  loaded = cx_file_storage_load_contents (&scData, &scDataSize, shaderConfigFilename, CX_FILE_STORAGE_BASE_RESOURCE);
  CX_FATAL_ASSERT (loaded);
  
  success = cx_shader_compile (&vertexShader, GL_VERTEX_SHADER, (const char *) vsData, vsDataSize);
  CX_FATAL_ASSERT (success);
  
  success = cx_shader_compile (&fragmentShader, GL_FRAGMENT_SHADER, (const char *) fsData, fsDataSize);
  CX_FATAL_ASSERT (success);
  
  success = cx_shader_link (&shaderProgram, vertexShader, fragmentShader);
  CX_FATAL_ASSERT (success);
  
  cx_shader *shader = (cx_shader *) cx_malloc (sizeof(cx_shader));
  shader->program   = shaderProgram;
  shader->enabled   = false;
  shader->name      = cx_strdup (name, strlen (name));
  
  memset (shader->attributes, -1, sizeof (shader->attributes));
  memset (shader->uniforms, -1, sizeof (shader->uniforms));
  
  success = cx_shader_configure ((const char *) scData, scDataSize, shader);
  CX_FATAL_ASSERT (success);
  
  cx_free (vsData);
  cx_free (fsData);
  cx_free (scData);
  
  return shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_destroy (cx_shader *shader)
{
  CX_ASSERT (shader);
  
  glDeleteProgram (shader->program);
  
  cx_free (shader->name);
  cx_free (shader);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_begin (cx_shader *shader)
{
  CX_ASSERT (shader);
  CX_ASSERT (!shader->enabled);
  
  glUseProgram (shader->program);
  cx_gdi_assert_no_errors ();
  
  shader->enabled = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_end (cx_shader *shader)
{
  CX_ASSERT (shader);
  CX_ASSERT (shader->enabled);
  
  shader->enabled = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_uniform (const cx_shader *shader, enum cx_shader_uniform uniform, const void *data)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (data);
  CX_ASSERT ((uniform > CX_SHADER_UNIFORM_INVALID) && (uniform < CX_NUM_SHADER_UNIFORMS));
  
  GLint location = shader->uniforms [uniform];
  CX_ASSERT (location >= 0);
  
  switch (uniform)
  {
    case CX_SHADER_UNIFORM_TRANSFORM_P:
    case CX_SHADER_UNIFORM_TRANSFORM_MV:
    case CX_SHADER_UNIFORM_TRANSFORM_MVP:
    {
#if CX_SHADER_DEBUG && 0
      GLint usize;
      GLenum utype;
      glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
      
      CX_ASSERT (utype == GL_FLOAT_MAT4);
#endif
      cx_mat4x4 *mat4 = (cx_mat4x4 *) data;
      glUniformMatrix4fv (location, 1, GL_FALSE, mat4->f16);
      cx_gdi_assert_no_errors ();
      break;
    }
      
    case CX_SHADER_UNIFORM_TRANSFORM_N:
    {
#if CX_SHADER_DEBUG && 0
      GLint usize;
      GLenum utype;
      glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
      
      CX_ASSERT (utype == GL_FLOAT_MAT3);
#endif
      cx_mat3x3 *mat3 = (cx_mat3x3 *) data;
      glUniformMatrix3fv (location, 1, GL_FALSE, mat3->f9);
      cx_gdi_assert_no_errors ();
      break;
    }
      
    case CX_SHADER_UNIFORM_EYE_POSITION:
    case CX_SHADER_UNIFORM_LIGHT_POSITION:
    {
#if CX_SHADER_DEBUG && 0
      GLint usize;
      GLenum utype;
      glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
      
      CX_ASSERT (utype == GL_FLOAT_VEC4);
#endif
      cx_vec4 *vec4 = (cx_vec4 *) data;
      glUniform4fv (location, 1, vec4->f4);
      cx_gdi_assert_no_errors ();
      break;
    }
      
    case CX_SHADER_UNIFORM_DIFFUSE_MAP:
    case CX_SHADER_UNIFORM_SPECULAR_MAP:
    case CX_SHADER_UNIFORM_BUMP_MAP:
    {
#if CX_SHADER_DEBUG && 0
      GLint usize;
      GLenum utype;
      glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
      
      CX_ASSERT (utype == GL_SAMPLER_2D);
#endif
      cx_texture *tex = (cx_texture *) data;
      cxi32 sampler = cx_shader_get_uniform_sampler (uniform);
      GLenum textureUnit = g_glTextureUnits [sampler];
      
      glActiveTexture (textureUnit);
      cx_gdi_assert_no_errors ();
      
      glBindTexture (GL_TEXTURE_2D, tex->id);
      cx_gdi_assert_no_errors ();
      
      glUniform1i (location, sampler);
      cx_gdi_assert_no_errors ();
      
      break;
    }
      
    default:
    {
      CX_FATAL_ERROR ("CX_SHADER_UNIFORM_INVALID");
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_float (const cx_shader *shader, const char *name, cxf32 *f, cxi32 count)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (f);
  CX_ASSERT (count > 0);
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  
  CX_ASSERT (count == usize);
  CX_ASSERT (utype == GL_FLOAT);
#endif
  
  glUniform1fv (location, count, f); 
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_vector2 (const cx_shader *shader, const char *name, const cx_vec2 *vec2, cxi32 count)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (vec2);
  CX_ASSERT (count > 0);
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  
  CX_ASSERT (count == usize);
  CX_ASSERT (utype == GL_FLOAT_VEC2);
#endif
  
  glUniform2fv (location, count, vec2->f2);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_vector4 (const cx_shader *shader, const char *name, const cx_vec4 *vec4, cxi32 count)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (vec4);
  CX_ASSERT (count > 0);
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  
  CX_ASSERT (count == usize);
  CX_ASSERT (utype == GL_FLOAT_VEC4);
#endif
  
  glUniform4fv (location, count, vec4->f4);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_matrix3x3 (const cx_shader *shader, const char *name, const cx_mat3x3 *mat3x3, cxi32 count)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (mat3x3);
  CX_ASSERT (count > 0);
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  
  CX_ASSERT (count == usize);
  CX_ASSERT (utype == GL_FLOAT_MAT3);
#endif
  
  glUniform4fv (location, count, mat3x3->f9);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_matrix4x4 (const cx_shader *shader, const char *name, const cx_mat4x4 *mat4x4, cxi32 count)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (mat4x4);
  CX_ASSERT (count > 0);
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  
  CX_ASSERT (count == usize);
  CX_ASSERT (utype == GL_FLOAT_MAT4);
#endif
  
  glUniform4fv (location, count, mat4x4->f16);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_texture (const cx_shader *shader, const char *name, const cx_texture *texture, cxi32 sampler)
{
  CX_ASSERT (shader && shader->enabled);
  CX_ASSERT (name);
  CX_ASSERT (texture);
  CX_ASSERT ((sampler >= 0) && (sampler < (cxi32) (sizeof (g_glTextureUnits) / sizeof (GLenum))));
  
  GLint location = glGetUniformLocation (shader->program, name);
  CX_ASSERT (location >= 0);
  
#if CX_SHADER_DEBUG && 0
  GLint usize;
  GLenum utype;
  glGetActiveUniform (shader->program, location, 0, NULL, &usize, &utype, NULL);
  CX_ASSERT (utype == GL_SAMPLER_2D);
#endif
  
#if CX_SHADER_DEBUG
  for (int i = CX_SHADER_UNIFORM_DIFFUSE_MAP; i <= CX_SHADER_UNIFORM_BUMP_MAP; ++i)
  {
    if (shader->uniforms [i] >= 0)
    {
      cx_shader_uniform uniform = (cx_shader_uniform) i;
      CX_ASSERT (sampler != cx_shader_get_uniform_sampler (uniform));
      CX_REF_UNUSED (uniform);
    }
  }
#endif
  
  GLenum textureUnit = g_glTextureUnits [sampler];
  
  glActiveTexture (textureUnit);
  cx_gdi_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_gdi_assert_no_errors ();
  
  glUniform1i (location, sampler);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_get_built_in (cx_shader_built_in type)
{
  CX_ASSERT (g_initialised);
  CX_ASSERT ((type > CX_SHADER_BUILT_IN_INVALID) && (type < CX_NUM_BUILT_IN_SHADERS));
  
  return g_builtInShaders [type];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_shader_built_in_init (void)
{
  // create built in shaders
  for (cxi16 i = 0; i < CX_NUM_BUILT_IN_SHADERS; ++i)
  {
    CX_ASSERT (i == g_shaderDescriptions [i].type);
    
    cx_shader_built_in type = g_shaderDescriptions [i].type;
    const char *name = g_shaderDescriptions [i].name;
    const char *dir = g_shaderDescriptions [i].dir;
    
    g_builtInShaders [type] = cx_shader_create (name, dir);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_shader_built_in_deinit (void)
{
  // destroy built in shaders
  for (cxi16 i = 0; i < CX_NUM_BUILT_IN_SHADERS; ++i)
  {
    cx_shader *shader = g_builtInShaders [i];
    
    cx_shader_destroy (shader);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cxi32 cx_shader_get_uniform_sampler (cx_shader_uniform uniform)
{
  switch (uniform)
  {
    case CX_SHADER_UNIFORM_DIFFUSE_MAP:   { return 0; }
    case CX_SHADER_UNIFORM_SPECULAR_MAP:  { return 1; }
    case CX_SHADER_UNIFORM_BUMP_MAP:      { return 2; }
    default:                              { CX_FATAL_ERROR ("invalid uniform"); return -1; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
