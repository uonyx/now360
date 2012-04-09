//
//  cx_graphics_shader.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 29/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_graphics.h"
#include "cx_file.h"
#include "cx_string.h"

#include "yajl_tree.h"
#include "yajl_parse.h"
#include "yajl_gen.h"

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
  "CX_SHADER_UNIFORM_TRANSFORM_M",
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

struct cx_shader_node
{
  struct cx_shader_node *next;
  struct cx_shader shader;
};

typedef struct cx_shader_node cx_shader_node;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_shader_node *s_shaders = NULL;
static int s_numShaders = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_shader_compile (GLuint *shader, GLenum type, const char *buffer, cxi32 bufferSize);
static bool cx_shader_link (GLuint *program, GLuint vertexShader, GLuint fragmentShader);
static bool cx_shader_configure (const char *buffer, cx_shader *shader);

void cx_graphics_add_shader (cx_shader_node *shader);
void cx_graphics_remove_shader (cx_shader_node *shader);

void cx_debug_print_gl_attribs_and_uniforms (GLuint shaderProgram);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_add_shader (cx_shader_node *shader)
{
  cx_shader_node *node = shader;
  
  node->next = s_shaders;
  
  s_shaders = node;
  
  ++s_numShaders;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_remove_shader (cx_shader_node *shader)
{
  cx_shader_node *prev, *curr;
  
  prev = NULL;
  curr = s_shaders;
  
  while (curr)
  {
    if (curr == shader)
    {
      if (curr == s_shaders)
      {
        s_shaders = curr->next;
      }
      else
      {
        prev->next = curr->next;
      }
      
      --s_numShaders;
      return;
    }
    
    prev = curr;
    curr = curr->next;
  }
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
      CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "Shader compile log:\n%s", log);
      cx_free (log);
    }
#endif
    
    glDeleteShader (outShader);
    return FALSE;
  }
  
  *shader = outShader;
  
  return TRUE;
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
  
  //
  // bind attributes?
  //
  
#if 0
  glBindAttribLocation(outProgram, CX_GRAPHICS_VERTEX_ATTRIBUTE_POSITION, "position");
  glBindAttribLocation(outProgram, CX_GRAPHICS_VERTEX_ATTRIBUTE_NORMAL, "normal");
#endif
  
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
      CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "Program link log:\n%s", log);
      cx_free (log);
    }
#endif
    
    glDeleteProgram (outProgram);
    return FALSE;
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
      CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "Program validate log:\n%s", log);
      cx_free (log);
    }
    
    glDeleteProgram (outProgram);
    return FALSE;
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
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool cx_shader_configure (const char *buffer, cx_shader *shader)
{
  char errorBuffer [512];
  
  yajl_gen g = yajl_gen_alloc (NULL);
  yajl_handle h = yajl_alloc (NULL, NULL, g);
  yajl_config (h, yajl_allow_trailing_garbage);
  yajl_val root = yajl_tree_parse (buffer, errorBuffer, 512);
  
  if (root == NULL)
  {
    if (strlen (errorBuffer) == 0)
    {
      cx_sprintf (errorBuffer, 512, "%s", "Unknown Error");
    }
    
    CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, errorBuffer);
    CX_FATAL_ERROR ("JSON Parse Error: %s");
    
    return FALSE;
  }
  
  int i;
  
  //
  // attributes
  //
  
  int attribCount = 0;
  for (i = 0; i < CX_NUM_SHADER_ATTRIBUTES; ++i)
  {
    const char *attribStr = s_attribEnumStrings [i];
    const char *attribPath [] = { "attributes", attribStr, NULL };
    yajl_val attribNode = yajl_tree_get (root, attribPath, yajl_t_any);
    
    if (attribNode)
    {
      CX_ASSERT (attribNode->type == yajl_t_string);
      
      const char *attribName = YAJL_GET_STRING (attribNode);
      shader->attributes [i] = glGetAttribLocation (shader->program, attribName);
      
      CX_ASSERT (glGetError() == GL_NO_ERROR);
      CX_DEBUGLOG_CONSOLE (1, "%s: %s [%d]", attribStr, attribName, shader->attributes [i]);
      
      attribCount++;
    }
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
  for (i = 0; i < CX_NUM_SHADER_UNIFORMS; ++i)
  {
    const char *uniformStr = s_uniformEnumStrings [i];
    const char *uniformPath [] = { "uniforms", uniformStr, NULL };
    yajl_val uniformNode = yajl_tree_get (root, uniformPath, yajl_t_any);
    
    if (uniformNode)
    {
      CX_ASSERT (uniformNode->type == yajl_t_string);
      
      const char *uniformName = YAJL_GET_STRING (uniformNode);
      shader->uniforms [i] = glGetUniformLocation (shader->program, uniformName);
      
      CX_ASSERT (glGetError() == GL_NO_ERROR);
      CX_DEBUGLOG_CONSOLE (1, "%s: %s [%d]", uniformStr, uniformName, shader->uniforms [i]);
      
      uniformCount++;
    }
  }

#if CX_DEBUG
  GLint numUniforms;
  glGetProgramiv (shader->program, GL_ACTIVE_UNIFORMS, &numUniforms);
  CX_ASSERT (numUniforms <= uniformCount);
#endif
  
  yajl_tree_free (root);
  yajl_free (h);
  yajl_gen_free (g);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_create (const char *name, const char *dir)
{
  CX_ASSERT (name != NULL);
  
  BOOL success = FALSE;

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
  
  cx_shader_node *shaderNode;
  
  shaderNode                  = (cx_shader_node *) cx_malloc (sizeof (cx_shader_node));
  shaderNode->next            = NULL;
  shaderNode->shader.program  = shaderProgram;
  shaderNode->shader.name     = cx_strdup (name);
  
  memset (shaderNode->shader.attributes, -1, sizeof (shaderNode->shader.attributes));
  memset (shaderNode->shader.uniforms, -1, sizeof (shaderNode->shader.uniforms));
  
  success = cx_shader_configure (scFile.data, &shaderNode->shader);
  CX_FATAL_ASSERT (success);
  
  cx_graphics_add_shader (shaderNode);
  
  cx_file_unload (&vsFile);
  cx_file_unload (&fsFile);
  cx_file_unload (&scFile);
  
  return &shaderNode->shader;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_shader_unload_all (void)
{
  struct cx_shader_node *node;
  
  node = s_shaders;
  
  while (node)
  {
    cx_shader_node *nextNode = node->next;
    
    glDeleteProgram (node->shader.program);
    
    cx_free (node->shader.name);
    
    cx_free (node);
    
    node = nextNode;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_shader *cx_shader_get (const char *name)
{
  cx_shader_node *node = s_shaders;
  
  while (node)
  {
    if (!strcmp (node->shader.name, name))
    {
      return &node->shader;
    }
  }
  
  return NULL;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_use (const cx_shader *shader)
{
  glUseProgram (shader->program);
  
#if CX_DEBUG
  GLenum error = glGetError ();
  CX_ASSERT (error == GL_NO_ERROR);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_shader_write_to_uniform (const cx_shader *shader, enum cx_shader_uniform uniform, cx_shader_datatype type, void *data)
{
  CX_ASSERT (shader);
  CX_ASSERT ((uniform > CX_SHADER_UNIFORM_INVALID) && (uniform < CX_NUM_SHADER_UNIFORMS));
 
  switch (type) 
  {
    case CX_SHADER_DATATYPE_VECTOR3:   { glUniform3fv (shader->uniforms[uniform], 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_VECTOR4:   { glUniform4fv (shader->uniforms[uniform], 1, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX3X3: { glUniformMatrix3fv (shader->uniforms[uniform], 1, GL_FALSE, (GLfloat *) data); break; }
    case CX_SHADER_DATATYPE_MATRIX4X4: { glUniformMatrix4fv (shader->uniforms[uniform], 1, GL_FALSE, (GLfloat *) data); break; }
    default: { break; }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_debug_print_gl_attribs_and_uniforms (GLuint shaderProgram)
{
  // set uniforms
  
  GLint maxUniformLen;
  GLint numUniforms;
  char *uniformName;
  GLint index;
  
  glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORMS, &numUniforms);
  glGetProgramiv(shaderProgram, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxUniformLen);
  uniformName = malloc(sizeof(char) * maxUniformLen);
  
  for (index = 0; index < numUniforms; index++)
  {
    GLint size;
    GLenum type;
    GLint location;
    
    glGetActiveUniform(shaderProgram, index, maxUniformLen, NULL, &size, &type, uniformName);
    
    location = glGetUniformLocation(shaderProgram, uniformName);
    
    CX_DEBUGLOG_CONSOLE (1, "Uniform name: %s, Uniform Location: %d", uniformName, location);
  }
  
  free (uniformName);
  
  // set attributes
  GLint maxAttribLen;
  GLint numAttribs;
  char *attribName;
  
  glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTES, &numAttribs);
  glGetProgramiv(shaderProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxAttribLen);
  attribName = malloc (sizeof(char) * maxAttribLen);
  
  for (index = 0; index < numAttribs; index++)
  {
    GLint size;
    GLenum type;
    GLint location;
    
    glGetActiveAttrib (shaderProgram, index, maxAttribLen, NULL, &size, &type, attribName);
    
    location = glGetAttribLocation (shaderProgram, attribName);
    
    CX_DEBUGLOG_CONSOLE (1, "Attribute name: %s, Attribute Location: %d", attribName, location);
  }
  
  free (attribName);
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

