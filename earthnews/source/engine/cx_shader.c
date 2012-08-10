//
//  cx_gdi_shader.c
//
//  Created by Ubaka Onyechi on 29/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_graphics.h"
#include "cx_file.h"
#include "cx_string.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SHADER_VERTEX_SHADER_FILE_EXTENSION    "vsh"
#define CX_SHADER_FRAGMENT_SHADER_FILE_EXTENSION  "fsh"
#define CX_SHADER_CONFIGURATION_FILE_EXTENSION    "cfg"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *s_attribEnumStrings [CX_NUM_SHADER_ATTRIBUTES] =
{
  "CX_SHADER_ATTRIBUTE_POSITION",
  "CX_SHADER_ATTRIBUTE_NORMAL",
  "CX_SHADER_ATTRIBUTE_TEXCOORD",
  "CX_SHADER_ATTRIBUTE_TANGENT",
  "CX_SHADER_ATTRIBUTE_COLOUR",
};

static const char *s_uniformEnumStrings [CX_NUM_SHADER_UNIFORMS] =
{
  "CX_SHADER_UNIFORM_TRANSFORM_P",
  "CX_SHADER_UNIFORM_TRANSFORM_MV",
  "CX_SHADER_UNIFORM_TRANSFORM_MVP",
  "CX_SHADER_UNIFORM_TRANSFORM_N",
  "CX_SHADER_UNIFORM_EYE",
  "CX_SHADER_UNIFORM_LIGHT_POSITION",
  "CX_SHADER_UNIFORM_LIGHT_DIRECTION",
  "CX_SHADER_UNIFORM_SAMPLER2D_0",
  "CX_SHADER_UNIFORM_SAMPLER2D_1",
  "CX_SHADER_UNIFORM_SAMPLER2D_2",
  "CX_SHADER_UNIFORM_SAMPLER2D_3",
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;

struct cx_shader_description
{
  cx_shader_built_in type;
  const char *name;
  const char *dir;
};

static struct cx_shader_description s_shaderDescriptions [CX_NUM_BUILT_IN_SHADERS] = 
{
  { CX_SHADER_BUILT_IN_FONT,      "font",         "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW,      "draw",         "data/shaders" },
  { CX_SHADER_BUILT_IN_DRAW_TEX,  "draw_tex",     "data/shaders" },
  { CX_SHADER_BUILT_IN_POINTS,    "draw_points",  "data/shaders" },
};

struct cx_shader *s_builtInShaders [CX_NUM_BUILT_IN_SHADERS];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_built_in_init (void);
void cx_shader_built_in_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_shader_init (void)
{
  CX_ASSERT (!s_initialised);
  
  memset (s_builtInShaders, 0, sizeof (s_builtInShaders));
  
  cx_shader_built_in_init ();
  
  s_initialised = true;
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_shader_deinit (void)
{
  if (s_initialised)
  {
    cx_shader_built_in_deinit ();
    
    s_initialised = false;
  }

  return !s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_shader_attribute cx_get_shader_attribute_from_string (const char *str)
{
  int i;
  
  for (i = 0; i < CX_NUM_SHADER_ATTRIBUTES; ++i)
  {
    if (strcmp (str, s_attribEnumStrings [i]) == 0)
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
  int i;
  
  for (i = 0; i < CX_NUM_SHADER_UNIFORMS; ++i)
  {
    if (strcmp (str, s_uniformEnumStrings [i]) == 0)
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
  
#if CX_DEBUG
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
  json_settings settings;
  memset (&settings, 0, sizeof(settings));
  
  char errorBuffer [512];
  errorBuffer [0] = '\0';
  
  json_value *root = json_parse_ex (&settings, buffer, bufferSize, errorBuffer, 512);
  
  if (root == NULL)
  {
    CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, errorBuffer);
    CX_FATAL_ERROR ("JSON Parse Error");
    
    return false;
  }

#if (CX_SHADER_DEBUG && 0)
  CX_FATAL_ASSERT (root->u.object.length == 2);
  
  const char *attributes = root->u.object.values [0].name;
  const char *uniforms = root->u.object.values [1].name;
  
  CX_FATAL_ASSERT (strcmp (attributes, "attributes") == 0);
  CX_FATAL_ASSERT (strcmp (uniforms, "uniforms") == 0);
#endif
  
  unsigned int i;
  
  //
  // attributes
  //

  int attribCount = 0;
  json_value *attributeNode = root->u.object.values [0].value;
  
  for (i = 0; i < attributeNode->u.object.length; ++i)
  {
    const char *attribStr = attributeNode->u.object.values [i].name;
    json_value *value = attributeNode->u.object.values [i].value;
    
    int attribIdx = cx_get_shader_attribute_from_string (attribStr);
    
    CX_ASSERT (attribIdx != CX_SHADER_ATTRIBUTE_INVALID);
    CX_ASSERT (value->type == json_string);
    
    const char *attribName = value->u.string.ptr;
    
    shader->attributes [attribIdx] = glGetAttribLocation (shader->program, attribName);
    
    cx_gdi_assert_no_errors ();
    CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "%s: %s [%d]", attribStr, attribName, shader->attributes [i]);
     
    attribCount++;
  }
  
#if CX_DEBUG
  GLint numAttributes;
  glGetProgramiv (shader->program, GL_ACTIVE_ATTRIBUTES, &numAttributes);
  CX_ASSERT (numAttributes <= attribCount);
#endif
  
  //
  // uniforms
  //
  
  int uniformCount = 0;
  json_value *uniformNode = root->u.object.values [1].value;
  
  for (i = 0; i < uniformNode->u.object.length; ++i)
  {
    const char *uniformStr = uniformNode->u.object.values [i].name;
    json_value *value = uniformNode->u.object.values [i].value;
    
    int uniformIdx = cx_get_shader_uniform_from_string (uniformStr);
    
    CX_ASSERT (uniformIdx != CX_SHADER_UNIFORM_INVALID);
    CX_ASSERT (value->type == json_string);
    
    const char *uniformName = value->u.string.ptr;
    
    shader->uniforms [uniformIdx] = glGetUniformLocation (shader->program, uniformName);
    
    cx_gdi_assert_no_errors ();
    CX_DEBUGLOG_CONSOLE (CX_SHADER_DEBUG_LOG_ENABLED, "%s: %s [%d]", uniformStr, uniformName, shader->uniforms [i]);
    
    uniformCount++;
  }
  
#if CX_DEBUG
  GLint numUniforms;
  glGetProgramiv (shader->program, GL_ACTIVE_UNIFORMS, &numUniforms);
  CX_ASSERT (numUniforms <= uniformCount);
#endif
  
  json_value_free (root);
  
  return true;
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
  
  cx_file vsFile, fsFile, scFile;
  GLuint vertexShader, fragmentShader, shaderProgram;
  
  success = cx_file_load (&vsFile, vertexShaderFilename);
  CX_FATAL_ASSERT (success);
  
  success = cx_file_load (&fsFile, fragmentShaderFilename);
  CX_FATAL_ASSERT (success);
  
  success = cx_file_load (&scFile, shaderConfigFilename);
  CX_FATAL_ASSERT (success);
  
  success = cx_shader_compile (&vertexShader, GL_VERTEX_SHADER, vsFile.data, vsFile.size);
  CX_FATAL_ASSERT (success);
  
  success = cx_shader_compile (&fragmentShader, GL_FRAGMENT_SHADER, fsFile.data, fsFile.size);
  CX_FATAL_ASSERT (success);
  
  success = cx_shader_link (&shaderProgram, vertexShader, fragmentShader);
  CX_FATAL_ASSERT (success);
  
  cx_shader *shader = (cx_shader *) cx_malloc (sizeof(cx_shader));
  shader->program   = shaderProgram;
  shader->name      = cx_strdup (name, strlen (name));
  
  memset (shader->attributes, -1, sizeof (shader->attributes));
  memset (shader->uniforms, -1, sizeof (shader->uniforms));
  
  success = cx_shader_configure (scFile.data, scFile.size, shader);
  CX_FATAL_ASSERT (success);
  
  cx_file_unload (&vsFile);
  cx_file_unload (&fsFile);
  cx_file_unload (&scFile);
  
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

void cx_shader_use (const cx_shader *shader)
{
  glUseProgram (shader->program);
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_uniform (const cx_shader *shader, enum cx_shader_uniform uniform, cx_shader_datatype type, void *data)
{
  CX_ASSERT (shader);
  CX_ASSERT ((uniform > CX_SHADER_UNIFORM_INVALID) && (uniform < CX_NUM_SHADER_UNIFORMS));
  
  GLint location = shader->uniforms [uniform];
  CX_ASSERT (location >= 0);
 
  switch (type) 
  {
    case CX_SHADER_DATATYPE_FLOAT:      { glUniform1f  (location, *(float *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR2:    { glUniform2fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR3:    { glUniform3fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR4:    { glUniform4fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX3X3:  { glUniformMatrix3fv (location, 1, GL_FALSE, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX4X4:  { glUniformMatrix4fv (location, 1, GL_FALSE, (GLfloat *) data); break; }
    default: { break; }
  }
  
  cx_gdi_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_set_uniform_2 (const cx_shader *shader, const char *uniformName, cx_shader_datatype type, void *data)
{
  CX_ASSERT (shader);
  CX_ASSERT (uniformName);
  
  GLint location = glGetUniformLocation (shader->program, uniformName);
  CX_ASSERT (location >= 0);
  
  switch (type) 
  {
    case CX_SHADER_DATATYPE_FLOAT:      { glUniform1f  (location, *(float *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR2:    { glUniform2fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR3:    { glUniform3fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR4:    { glUniform4fv (location, 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX3X3:  { glUniformMatrix3fv (location, 1, GL_FALSE, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX4X4:  { glUniformMatrix4fv (location, 1, GL_FALSE, (GLfloat *) data); break; }
    default: { break; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_built_in_init (void)
{
  // create built in shaders
  for (cxi16 i = 0; i < CX_NUM_BUILT_IN_SHADERS; ++i)
  {
    CX_ASSERT (i == s_shaderDescriptions [i].type);
    
    cx_shader_built_in type = s_shaderDescriptions [i].type;
    const char *name = s_shaderDescriptions [i].name;
    const char *dir = s_shaderDescriptions [i].dir;
    
    s_builtInShaders [type] = cx_shader_create (name, dir);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_built_in_deinit (void)
{
  // destroy built in shaders
  for (cxi16 i = 0; i < CX_NUM_BUILT_IN_SHADERS; ++i)
  {
    cx_shader *shader = s_builtInShaders [i];
    
    cx_shader_destroy (shader);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_get_built_in (cx_shader_built_in type)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT ((type > CX_SHADER_BUILT_IN_INVALID) && (type < CX_NUM_BUILT_IN_SHADERS));
  
  return s_builtInShaders [type];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
