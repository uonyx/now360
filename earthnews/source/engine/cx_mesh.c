//
//  cx_mesh.c
//
//  Created by Ubaka Onyechi on 26/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_graphics.h"
#include "cx_mesh.h"
#include "cx_math.h"
#include "cx_string.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_gpu_init (cx_mesh *mesh, const cx_shader *shader);
void cx_mesh_gpu_deinit (cx_mesh *mesh);

void cx_vertex_data_create_sphere_soa (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_soa *vertexData);
void cx_vertex_data_create_sphere_aos (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_aos *vertexData);
void cx_vertex_data_destroy_soa (struct cx_vertex_data_soa *vertexData);
void cx_vertex_data_destroy_aos (struct cx_vertex_data_aos *vertexData);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_mesh *cx_mesh_create_sphere (cxf32 radius, cxu16 slices, cxu16 parallels, cx_shader *shader, cx_material *material)
{
  CX_ASSERT (shader);
  
  cx_mesh *mesh = (cx_mesh *) cx_malloc (sizeof (cx_mesh));
  
  mesh->shader = shader;
  mesh->material = material;
  mesh->vao = 0;
  memset (mesh->vbos, 0, sizeof (mesh->vbos));
  
#if CX_VERTEX_DATA_AOS
  cx_vertex_data_create_sphere_aos (radius, slices, parallels, &mesh->vertexData);
#else
  cx_vertex_data_create_sphere_soa (radius, slices, parallels, &mesh->vertexData);
#endif
  
  cx_mesh_gpu_init (mesh, shader);
  
  return mesh;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_destroy (cx_mesh *mesh)
{  
  CX_ASSERT (mesh);
  
  cx_mesh_gpu_deinit (mesh);
#if CX_VERTEX_DATA_AOS
  cx_vertex_data_destroy_aos (&mesh->vertexData);
#else
  cx_vertex_data_destroy_soa (&mesh->vertexData);
#endif
  
  cx_free (mesh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_gpu_init (cx_mesh *mesh, const cx_shader *shader)
{
  // create vertex buffers
  
  CX_ASSERT (shader);
  CX_ASSERT (mesh);
  CX_ASSERT (mesh->vertexData.numIndices > 0);
  CX_ASSERT (mesh->vertexData.numVertices > 0);
  
  cx_vertex_data * CX_RESTRICT vertexData = &mesh->vertexData;
  
  cxi32 numIndices = vertexData->numIndices;
  cxi32 numVertices = vertexData->numVertices;
  
  glGenVertexArraysOES (1, &mesh->vao);
  glBindVertexArrayOES (mesh->vao);
  cx_graphics_assert_no_errors ();
  
  {
    glGenBuffers (CX_VERTEX_BUFFER_COUNT, mesh->vbos);
    
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
  #if CX_VERTEX_DATA_AOS
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    
    cxu32 offset = 0;
    
  #if CX_VERTEX_DATA_AOS_STRUCT
    cxu32 vstride = sizeof (cx_vertex);
  #else
    cxu32 vstride = CX_VERTEX_TOTAL_SIZE * sizeof (cxf32);
  #endif
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * vstride, vertexData->vertices, GL_STATIC_DRAW);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbos [1]);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof (cxu16), vertexData->indices, GL_STATIC_DRAW);
    
    cx_graphics_assert_no_errors ();
    
  #if CX_VERTEX_DATA_AOS_STRUCT
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
  #else
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], CX_VERTEX_POSITION_SIZE, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += (CX_VERTEX_POSITION_SIZE * sizeof (cxf32));
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], CX_VERTEX_NORMAL_SIZE, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += (CX_VERTEX_NORMAL_SIZE * sizeof (cxf32));
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], CX_VERTEX_TEXCOORD_SIZE, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
  #endif

    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL]);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD]);
    
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
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_POSITION], 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_POSITION]);
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [1]);
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_NORMAL], 3, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_NORMAL]);
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [2]);
    glVertexAttribPointer (shader->attributes[CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, 0, (const void *) 0);
    glEnableVertexAttribArray (shader->attributes[CX_SHADER_ATTRIBUTE_TEXCOORD]);

  #endif
  }
  
  cx_graphics_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_gpu_deinit (cx_mesh *mesh)
{
  CX_ASSERT (mesh);
  
  glDeleteBuffers (CX_VERTEX_BUFFER_COUNT, mesh->vbos);
  glDeleteVertexArraysOES (1, &mesh->vao);
  
  mesh->vao = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_render (cx_mesh *mesh)
{
  CX_ASSERT (mesh);
  CX_ASSERT (mesh->shader);

  if (mesh->material)
  {
    cx_material_render (mesh->material, mesh->shader);
  }
  
  glBindVertexArrayOES (mesh->vao);

  glDrawElements (GL_TRIANGLE_STRIP, mesh->vertexData.numIndices, GL_UNSIGNED_SHORT, 0);
  
  glBindVertexArrayOES (0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_create_sphere_soa (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_soa *vertexData)
{
  // Taken from the utility library for the OpenGL ES 2.0 programming guide
  /// \return The number of indices required for rendering the buffers (the number of indices stored in the indices array
  ///         if it is not NULL ) as a GL_TRIANGLE_STRIP
  
  CX_ASSERT (vertexData);
  
  cxu16 i;
  cxu16 j;
  cxu16 numVertices = (numParallels + 1) * (numSlices + 1);
  cxu16 numIndices = numParallels * numSlices * 6;
  cxf32 angleStep = (2.0f * CX_PI) / ((cxf32) numSlices);
  
  // Allocate memory for buffers
  vertexData->positions = cx_malloc (sizeof(cxf32) * CX_VERTEX_POSITION_SIZE * numVertices);
  vertexData->normals = cx_malloc (sizeof(cxf32) * CX_VERTEX_NORMAL_SIZE * numVertices);
  vertexData->texCoords = cx_malloc (sizeof(cxf32) * CX_VERTEX_TEXCOORD_SIZE * numVertices);
  vertexData->indices = cx_malloc (sizeof(cxu16) * numIndices);
  
  // Generate vertex data
  cxu16 vertex;
  
  for (i = 0; i < numParallels + 1; i++)
  {
    for (j = 0; j < numSlices + 1; j++)
    {
      cxf32 a0 = cx_sin (angleStep * (cxf32)i) * cx_sin (angleStep * (cxf32)j);
      cxf32 a1 = cx_cos (angleStep * (cxf32)i);
      cxf32 a2 = cx_sin (angleStep * (cxf32)i) * cx_cos (angleStep * (cxf32)j);
      
      vertex = (i * (numSlices + 1) + j) * CX_VERTEX_POSITION_SIZE; 
      
      vertexData->positions [vertex + 0] = radius * a0;
      vertexData->positions [vertex + 1] = radius * a1;
      vertexData->positions [vertex + 2] = radius * a2;
      
      vertex = (i * (numSlices + 1) + j) * CX_VERTEX_NORMAL_SIZE; 
      
      vertexData->normals [vertex + 0] = a0;
      vertexData->normals [vertex + 1] = a1;
      vertexData->normals [vertex + 2] = a2;
      
      vertex = (i * (numSlices + 1) + j) * CX_VERTEX_TEXCOORD_SIZE;
      
      vertexData->texCoords [vertex + 0] = (cxf32) j / (cxf32) numSlices;
      vertexData->texCoords [vertex + 1] = (cxf32) i / (cxf32) numParallels;
    }
  }
  
  // Generate the indices
  cxu16 *indexBuf = vertexData->indices;
  
  for (i = 0; i < numParallels ; i++) 
  {
    for (j = 0; j < numSlices; j++)
    {
      *indexBuf++ = i * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
      
      *indexBuf++ = i * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
      *indexBuf++ = i * (numSlices + 1) + (j + 1);
    }
  }
  
  vertexData->numIndices = numIndices;
  vertexData->numVertices = numVertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_create_sphere_aos (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_aos *vertexData)
{
  CX_ASSERT (vertexData);
  
  cxu16 i, j;
  cxu16 vertex;
  cxu16 numVertices = (numParallels + 1) * (numSlices + 1);
  cxu16 numIndices = numParallels * numSlices * 6;
  cxf32 angleStep = (2.0f * CX_PI) / (cxf32) numSlices;
  
  // Allocate memory for buffers
  vertexData->indices = (cxu16 *) cx_malloc (sizeof (cxu16) * numIndices);
#if CX_VERTEX_DATA_AOS_STRUCT
  vertexData->vertices = (cx_vertex *) cx_malloc (sizeof (cx_vertex) * numVertices);
#else
  vertexData->vertices = (cxf32 *) cx_malloc (sizeof (cxf32) * CX_VERTEX_TOTAL_SIZE * numVertices);
#endif 
  
  // Generate vertex data
  for (i = 0; i < (numParallels + 1); i++)
  {
    for (j = 0; j < (numSlices + 1); j++)
    {
      cxf32 a0 = cx_sin (angleStep * (cxf32) i) * cx_sin (angleStep * (cxf32)j);
      cxf32 a1 = cx_cos (angleStep * (cxf32) i);
      cxf32 a2 = cx_sin (angleStep * (cxf32) i) * cx_cos (angleStep * (cxf32)j);
      
#if CX_VERTEX_DATA_AOS_STRUCT
      vertex = (i * (numSlices + 1) + j);
      cx_vec4 * CX_RESTRICT position = &vertexData->vertices[vertex].position;
      cx_vec4 * CX_RESTRICT normal = &vertexData->vertices[vertex].normal;
      cx_vec2 * CX_RESTRICT texCoord = &vertexData->vertices[vertex].texCoord;
      
      cx_vec4_set (position, a0 * radius, a1 * radius, a2 * radius, 1.0f);
      cx_vec4_set (normal, a0, a1, a2, 0.0f);
      texCoord->x = (cxf32) j / (cxf32) numSlices;
      texCoord->y = (cxf32) (i - 1) / (cxf32) (numParallels - 1);
#else
      vertex = (i * (numSlices + 1) + j) * CX_VERTEX_TOTAL_SIZE;
      // position 
      vertexData->vertices [vertex + 0] = radius * a0;
      vertexData->vertices [vertex + 1] = radius * a1;
      vertexData->vertices [vertex + 2] = radius * a2;
      // normal
      vertexData->vertices [vertex + 3] = a0;
      vertexData->vertices [vertex + 4] = a1;
      vertexData->vertices [vertex + 5] = a2;
      // tex coord
      vertexData->vertices [vertex + 6] = (cxf32) j / (cxf32) numSlices;
      vertexData->vertices [vertex + 7] = (cxf32) i / (cxf32) numParallels;
#endif      
    }
  }
  
  // Generate the indices
  cxu16 *indexBuf = vertexData->indices;
  
  for (i = 0; i < numParallels ; i++) 
  {
    for (j = 0; j < numSlices; j++)
    {
      *indexBuf++ = i * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
      
      *indexBuf++ = i * (numSlices + 1) + j;
      *indexBuf++ = (i + 1) * (numSlices + 1) + (j + 1);
      *indexBuf++ = i * (numSlices + 1) + (j + 1);
    }
  }
  
  vertexData->numIndices = numIndices;
  vertexData->numVertices = numVertices;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_destroy_soa (struct cx_vertex_data_soa *vertexData)
{
  CX_ASSERT (vertexData);
  
  vertexData->numIndices = 0;
  vertexData->numVertices = 0;
  
  cx_free (vertexData->indices);
  cx_free (vertexData->positions);
  cx_free (vertexData->normals);
  cx_free (vertexData->texCoords);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_destroy_aos (struct cx_vertex_data_aos *vertexData)
{
  CX_ASSERT (vertexData);
  
  vertexData->numIndices = 0;
  vertexData->numVertices = 0;
  
  cx_free (vertexData->indices);
  cx_free (vertexData->vertices);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
