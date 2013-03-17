//
//  cx_mesh.c
//
//  Created by Ubaka Onyechi on 26/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "../system/cx_string.h"
#include "cx_mesh.h"
#include "cx_gdi.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define ENABLE_VAO 1
#define ENABLE_CONSTANT_ARRAY 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_mesh_gpu_init (cx_mesh *mesh, const cx_shader *shader);
static void cx_mesh_gpu_deinit (cx_mesh *mesh);
#ifdef CX_C99
static void cx_mesh_get_attributes (cx_vertex_format format, bool attr [static CX_NUM_SHADER_ATTRIBUTES]);
#else
static void cx_mesh_get_attributes (cx_vertex_format format, bool *attr, cxu32 attrSize);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_mesh *cx_mesh_create (cx_vertex_data *vertexData, cx_shader *shader, cx_material *material)
{
  CX_ASSERT (shader);
  CX_ASSERT (vertexData);
  
  cx_mesh *mesh = (cx_mesh *) cx_malloc (sizeof (cx_mesh));
  
  mesh->shader = shader;
  mesh->material = material;
  mesh->vertexData = vertexData;
  mesh->vao = 0;
  mesh->gpu = 0;
  
  memset (mesh->vbos, 0, sizeof (mesh->vbos));
  
#if !ENABLE_VAO
  cx_mesh_gpu_init (mesh, shader);
#endif
  
  return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_destroy (cx_mesh *mesh)
{  
  CX_ASSERT (mesh);
  
  cx_mesh_gpu_deinit (mesh);
  
  cx_free (mesh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CX_C99
static void cx_mesh_get_attributes (cx_vertex_format format, bool attr [static CX_NUM_SHADER_ATTRIBUTES])
{  
  memset (attr, false, sizeof (bool) * CX_NUM_SHADER_ATTRIBUTES);
  
  switch (format) 
  {
    case CX_VERTEX_FORMAT_P:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PN:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PT:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PTN:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PTNTB:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      attr [CX_SHADER_ATTRIBUTE_TANGENT] = true;
      attr [CX_SHADER_ATTRIBUTE_BITANGENT] = true;
      break;
    }
      
    default:
    {
      break;
    }
  }
}
#else
static void cx_mesh_get_attributes (cx_vertex_format format, bool *attr, cxu32 attrSize)
{
  CX_ASSERT (attr);
  CX_ASSERT (attrSize == CX_NUM_SHADER_ATTRIBUTES);
  
  memset (attr, false, sizeof (bool) * attrSize);
  
  switch (format) 
  {
    case CX_VERTEX_FORMAT_P:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      break;
    }

    case CX_VERTEX_FORMAT_PN:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PT:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      break;
    }
      
    case CX_VERTEX_FORMAT_PTN:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      break;
    }

    case CX_VERTEX_FORMAT_PTNTB:
    {
      attr [CX_SHADER_ATTRIBUTE_POSITION] = true;
      attr [CX_SHADER_ATTRIBUTE_TEXCOORD] = true;
      attr [CX_SHADER_ATTRIBUTE_NORMAL] = true;
      attr [CX_SHADER_ATTRIBUTE_TANGENT] = true;
      attr [CX_SHADER_ATTRIBUTE_BITANGENT] = true;
      break;
    }
      
    default:
    {
      break;
    }
  }
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_mesh_gpu_init (cx_mesh *mesh, const cx_shader *shader)
{
  // create vertex buffers
  
  CX_ASSERT (shader);
  CX_ASSERT (mesh);
  CX_ASSERT (mesh->vertexData);
  CX_ASSERT (mesh->vertexData->numIndices > 0);
  CX_ASSERT (mesh->vertexData->numVertices > 0);
  
  cx_vertex_data * CX_RESTRICT vertexData = mesh->vertexData;
  
  cxi32 numIndices = vertexData->numIndices;
  cxi32 numVertices = vertexData->numVertices;
  
#if ENABLE_VAO
  glGenVertexArraysOES (1, &mesh->vao);
  glBindVertexArrayOES (mesh->vao);
  cx_gdi_assert_no_errors ();
#endif
  
  bool enabled [CX_NUM_SHADER_ATTRIBUTES];
#ifdef CX_C99
  cx_mesh_get_attributes (mesh->vertexData->format, enabled);
#else
  cx_mesh_get_attributes (mesh->vertexData->format, enabled, CX_NUM_SHADER_ATTRIBUTES);
#endif
  
  {
    glGenBuffers (CX_VERTEX_BUFFER_COUNT, mesh->vbos);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
#if CX_VERTEX_DATA_AOS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    cxu32 offset = 0;
    cxu32 vstride = sizeof (cx_vertex);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * vstride, vertexData->vertices, GL_STATIC_DRAW);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbos [1]);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof (cxu16), vertexData->indices, GL_STATIC_DRAW);
    
    cx_gdi_assert_no_errors ();
    
    if (enabled [CX_SHADER_ATTRIBUTE_POSITION])
    {
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
      glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
    }

    offset += sizeof (cx_vec4);
    
    if (enabled [CX_SHADER_ATTRIBUTE_NORMAL])
    {
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
      glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL]);
    }
    
    offset += sizeof (cx_vec4);
    
    if (enabled [CX_SHADER_ATTRIBUTE_TANGENT])
    {
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
      glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT]);
    }
    
    offset += sizeof (cx_vec4);
    
    if (enabled [CX_SHADER_ATTRIBUTE_BITANGENT])
    {
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
      glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT]);
    }
    
    offset += sizeof (cx_vec4);
    
    if (enabled [CX_SHADER_ATTRIBUTE_TEXCOORD])
    {
      glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
      glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]); 
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////
#else // if CX_VERTEX_DATA_SOA
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * CX_VERTEX_POSITION_SIZE * sizeof (cxf32), vertexData->positions, GL_STATIC_DRAW);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [1]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * CX_VERTEX_NORMAL_SIZE * sizeof (cxf32), vertexData->normals, GL_STATIC_DRAW);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [2]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * CX_VERTEX_TEXCOORD_SIZE * sizeof (cxf32), vertexData->texCoords, GL_STATIC_DRAW);
    
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbos [3]);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof (cxu16), vertexData->indices, GL_STATIC_DRAW);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_POSITION], CX_VERTEX_POSITION_SIZE, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_POSITION]);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [1]);
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_NORMAL], CX_VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_NORMAL]);
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [2]);
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_TEXCOORD], CX_VERTEX_TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_TEXCOORD]);

#endif
  }
  
  cx_gdi_assert_no_errors ();
  
  mesh->gpu = 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_mesh_gpu_deinit (cx_mesh *mesh)
{
  CX_ASSERT (mesh);
  
  glDeleteBuffers (CX_VERTEX_BUFFER_COUNT, mesh->vbos);
  
#if ENABLE_VAO
  glDeleteVertexArraysOES (1, &mesh->vao);
#endif
  
  mesh->vao = 0;
  mesh->gpu = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_render (cx_mesh *mesh)
{
  CX_ASSERT (mesh);
  CX_ASSERT (mesh->shader);

#if ENABLE_VAO
  if (!mesh->gpu) // support vao/vbo rendering?
  {
    // opengl vaos can't be shared across contexts
    cx_mesh_gpu_init (mesh, mesh->shader);
  }
#endif
  
  if (mesh->material)
  {
    cx_material_render (mesh->material, mesh->shader);
  }
  
#if ENABLE_VAO
  glBindVertexArrayOES (mesh->vao);
  
  glDrawElements (GL_TRIANGLES, mesh->vertexData->numIndices, GL_UNSIGNED_SHORT, 0);
  
  glBindVertexArrayOES (0);
#endif
  
#if !ENABLE_VAO
  
  bool enabled [CX_NUM_SHADER_ATTRIBUTES];
#ifdef CX_C99
  cx_mesh_get_attributes (mesh->vertexData->format, enabled);
#else
  cx_mesh_get_attributes (mesh->vertexData->format, enabled, CX_NUM_SHADER_ATTRIBUTES);
#endif
  
  cxu32 offset = 0;
  cxu32 vstride = sizeof (cx_vertex);
  
  glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
  
  cx_gdi_assert_no_errors ();
  
  cx_shader *shader = mesh->shader;
  
  if (enabled [CX_SHADER_ATTRIBUTE_POSITION])
  {
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
  }
  
  offset += sizeof (cx_vec4);
  
  if (enabled [CX_SHADER_ATTRIBUTE_NORMAL])
  {
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL]);
  }
  
  offset += sizeof (cx_vec4);
  
  if (enabled [CX_SHADER_ATTRIBUTE_TANGENT])
  {
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT]);
  }
  
  offset += sizeof (cx_vec4);
  
  if (enabled [CX_SHADER_ATTRIBUTE_BITANGENT])
  {
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT]);
  }
  
  offset += sizeof (cx_vec4);
  
  if (enabled [CX_SHADER_ATTRIBUTE_TEXCOORD])
  {
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]); 
  }
  
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbos [1]);
  
  glDrawElements (GL_TRIANGLES, mesh->vertexData->numIndices, GL_UNSIGNED_SHORT, 0);
  
#endif
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
