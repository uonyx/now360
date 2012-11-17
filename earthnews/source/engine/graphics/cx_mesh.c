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

#define CX_VERTEX_TOTAL_SIZE (CX_VERTEX_POSITION_SIZE + CX_VERTEX_NORMAL_SIZE + CX_VERTEX_TEXCOORD_SIZE)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_gpu_init (cx_mesh *mesh, const cx_shader *shader);
void cx_mesh_gpu_deinit (cx_mesh *mesh);

void cx_vertex_data_create_sphere_soa (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_soa *vertexData);
void cx_vertex_data_create_sphere_aos (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_aos *vertexData);
void cx_vertex_data_destroy_soa (struct cx_vertex_data_soa *vertexData);
void cx_vertex_data_destroy_aos (struct cx_vertex_data_aos *vertexData);

void cx_vertex_data_calculate_tangent_array_aos (const struct cx_vertex_data_aos *vertexData, cxf32 **tangent);
void cx_vertex_data_sphere_compute_tangents_aos (struct cx_vertex_data_aos *vertexData);

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
  cx_gdi_assert_no_errors ();
  
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
    
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TEXCOORD], 2, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);

    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION]);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL]);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT]);
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_BITANGENT]);
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

  glDrawElements (GL_TRIANGLES, mesh->vertexData.numIndices, GL_UNSIGNED_SHORT, 0);
  
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
  
  cxu16 i, j;
  cxu16 vertex;
  cxi32 numVertices = (numParallels + 1) * (numSlices + 1);
  cxi32 numIndices = numParallels * numSlices * 6;
  cxi32 numTriangles = numParallels * numSlices * 2;
  cxf32 angleStep = (2.0f * CX_PI) / ((cxf32) numSlices);
  
  // Allocate memory for buffers
  vertexData->positions = cx_malloc (sizeof (cxf32) * CX_VERTEX_POSITION_SIZE * numVertices);
  vertexData->normals = cx_malloc (sizeof (cxf32) * CX_VERTEX_NORMAL_SIZE * numVertices);
  vertexData->texCoords = cx_malloc (sizeof (cxf32) * CX_VERTEX_TEXCOORD_SIZE * numVertices);
  vertexData->indices = cx_malloc (sizeof (cxu16) * numIndices);
  
  // Generate vertex data
  for (i = 0; i < numParallels + 1; i++)
  {
    for (j = 0; j < numSlices + 1; j++)
    {
      cxf32 a0 = cx_sin (angleStep * (cxf32) i) * cx_sin (angleStep * (cxf32) j);
      cxf32 a1 = cx_cos (angleStep * (cxf32) i);
      cxf32 a2 = cx_sin (angleStep * (cxf32) i) * cx_cos (angleStep * (cxf32) j);
      
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
  vertexData->numTriangles = numTriangles;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_create_sphere_aos (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_aos *vertexData)
{
  CX_ASSERT (vertexData);
  
  cxu16 i, j;
  cxu16 vertex;
  cxi32 numVertices = (numParallels + 1) * (numSlices + 1);
  cxi32 numIndices = numParallels * numSlices * 6;
  cxi32 numTriangles = numParallels * numSlices * 2;
  cxf32 angleStep = (2.0f * CX_PI) / (cxf32) numSlices;
  
  // Allocate memory for buffers
  vertexData->indices = (cxu16 *) cx_malloc (sizeof (cxu16) * numIndices);
  vertexData->vertices = (cx_vertex *) cx_malloc (sizeof (cx_vertex) * numVertices);
  
  // Generate vertex data
  for (i = 0; i < (numParallels + 1); i++)
  {
    for (j = 0; j < (numSlices + 1); j++)
    {
      cxf32 a0 = cx_sin (angleStep * (cxf32) i) * cx_sin (angleStep * (cxf32) j);
      cxf32 a1 = cx_cos (angleStep * (cxf32) i);
      cxf32 a2 = cx_sin (angleStep * (cxf32) i) * cx_cos (angleStep * (cxf32) j);
      
      vertex = (i * (numSlices + 1) + j);
      
      // position
      cx_vec4 * CX_RESTRICT position = &vertexData->vertices[vertex].position;
      cx_vec4_set (position, a0 * radius, a1 * radius, a2 * radius, 1.0f);
      
      // normal
      cx_vec4 * CX_RESTRICT normal = &vertexData->vertices[vertex].normal;
      cx_vec4_set (normal, a0, a1, a2, 0.0f);
      
      // tex coord
      cx_vec2 * CX_RESTRICT texCoord = &vertexData->vertices[vertex].texCoord;
      texCoord->x = (cxf32) j / (cxf32) numSlices;
      texCoord->y = (cxf32) i / (cxf32) numParallels;    
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
  vertexData->numTriangles = numTriangles;

  // Compute tangents
  cx_vertex_data_sphere_compute_tangents_aos (vertexData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_sphere_compute_tangents_aos (struct cx_vertex_data_aos *vertexData)
{
  // Reference: http://www.terathon.com/code/tangent.html
  
  cxi32 vertCount = vertexData->numVertices;
  cxi32 triCount = vertexData->numTriangles;
  
  cx_vec4 *tan1 = (cx_vec4 *) cx_malloc (sizeof (cx_vec4) * vertCount * 2);
  cx_vec4 *tan2 = tan1 + vertCount;

  CX_ASSERT ((triCount * 3) == vertexData->numIndices);
  
  cxi32 index = 0;

  for (cxi32 i = 0; i < triCount; ++i)
  {
    CX_ASSERT (index < vertexData->numIndices);
    cxu16 i0 = vertexData->indices [index++];
    CX_ASSERT (index < vertexData->numIndices);
    cxu16 i1 = vertexData->indices [index++];
    CX_ASSERT (index < vertexData->numIndices);
    cxu16 i2 = vertexData->indices [index++];
    
    const cx_vec4 *pos0 = &vertexData->vertices [i0].position;
    const cx_vec4 *pos1 = &vertexData->vertices [i1].position;
    const cx_vec4 *pos2 = &vertexData->vertices [i2].position;
    
    const cx_vec2 *tex0 = &vertexData->vertices [i0].texCoord;
    const cx_vec2 *tex1 = &vertexData->vertices [i1].texCoord;
    const cx_vec2 *tex2 = &vertexData->vertices [i2].texCoord;
    
    cxf32 x1 = pos1->x - pos0->x;
    cxf32 x2 = pos2->x - pos0->x;
    cxf32 y1 = pos1->y - pos0->y;
    cxf32 y2 = pos2->y - pos0->y;
    cxf32 z1 = pos1->z - pos0->z;
    cxf32 z2 = pos2->z - pos0->z;
    
    cxf32 s1 = tex1->x - tex0->x;
    cxf32 s2 = tex2->x - tex0->x;
    cxf32 t1 = tex1->y - tex0->y;
    cxf32 t2 = tex2->y - tex0->y;
    
    cx_vec4 sdir, tdir;
    
    cxf32 r = ((s1 * t2) - (s2 * t1));
    
    if (cx_is_zero (r))
    {
      cx_vec4_set (&sdir, 1.0f, 0.0f, 0.0f, 0.0f);
      cx_vec4_set (&tdir, 0.0f, 1.0f, 0.0f, 0.0f);
    }
    else
    {
      r = 1.0f / r;
      
      sdir.x = ((t2 * x1) - (t1 * x2)) * r;
      sdir.y = ((t2 * y1) - (t1 * y2)) * r;
      sdir.z = ((t2 * z1) - (t1 * z2)) * r;
      sdir.w = 0.0f;
      
      tdir.x = ((s1 * x2) - (s2 * x1)) * r;
      tdir.y = ((s1 * y2) - (s2 * y1)) * r;
      tdir.z = ((s1 * z2) - (s2 * z1)) * r;
      tdir.w = 0.0f;
    }
    
    cx_vec4_add (&tan1 [i0], &tan1 [i0], &sdir);
    cx_vec4_add (&tan1 [i1], &tan1 [i1], &sdir);
    cx_vec4_add (&tan1 [i2], &tan1 [i2], &sdir);
    
    cx_vec4_add (&tan2 [i0], &tan2 [i0], &tdir);
    cx_vec4_add (&tan2 [i1], &tan2 [i1], &tdir);
    cx_vec4_add (&tan2 [i2], &tan2 [i2], &tdir);
  }
  
  for (cxi32 i = 0; i < vertCount; i++)
  {
    cx_vec4 *tangent = &vertexData->vertices [i].tangent;
    cx_vec4 *bitangent = &vertexData->vertices [i].bitangent;
    cx_vec4 *normal = &vertexData->vertices [i].normal;
    
    const cx_vec4 *t = &tan1 [i];
    
    // Gram-Schmidt orthogonalize
    
    cxf32 dotp = cx_vec4_dot (normal, t);
    
    cx_vec4 ndotp;
    cx_vec4_mul (&ndotp, dotp, normal);
    
    cx_vec4_sub (tangent, t, &ndotp);
    
    cxf32 tlen = cx_vec4_length (tangent);

    if (cx_is_zero (tlen))
    {
      cx_vec4_zero (tangent);
      cx_vec4_zero (bitangent);
    }
    else
    {
      // normalize 
      cx_vec4_mul (tangent, (1.0f / tlen), tangent);
      
      // calculate handedness
      
      cx_vec4 cross;
      cx_vec4_cross (&cross, normal, t);
      
      cxf32 dotp2 = cx_vec4_dot (&cross, &tan2 [i]);
      
      cxf32 handedness = (dotp2 < 0.0f) ? -1.0f : 1.0f;
      
      // compute bitangent
      cx_vec4_cross (bitangent, normal, tangent);
      cx_vec4_mul (bitangent, handedness, bitangent);
    }
  }
  
  cx_free (tan1);
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
