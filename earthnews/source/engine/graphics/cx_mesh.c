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
    
#if CX_VERTEX_DATA_AOS_STRUCT
    cxu32 vstride = sizeof (cx_vertex);
#else
    cxu32 vstride = CX_VERTEX_TOTAL_SIZE * sizeof (cxf32);
#endif
    
    glBindBuffer (GL_ARRAY_BUFFER, mesh->vbos [0]);
    glBufferData (GL_ARRAY_BUFFER, numVertices * vstride, vertexData->vertices, GL_STATIC_DRAW);
    glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, mesh->vbos [1]);
    glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof (cxu16), vertexData->indices, GL_STATIC_DRAW);
    
    cx_gdi_assert_no_errors ();
    
#if CX_VERTEX_DATA_AOS_STRUCT
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_POSITION], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_NORMAL], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
    offset += sizeof (cx_vec4);
    glVertexAttribPointer (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT], 4, GL_FLOAT, GL_FALSE, vstride, (const void *) offset);
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
    glEnableVertexAttribArray (shader->attributes [CX_SHADER_ATTRIBUTE_TANGENT]);
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
      cxf32 a0 = cx_sin (angleStep * (cxf32) i) * cx_sin (angleStep * (cxf32) j);
      cxf32 a1 = cx_cos (angleStep * (cxf32) i);
      cxf32 a2 = cx_sin (angleStep * (cxf32) i) * cx_cos (angleStep * (cxf32) j);
      
#if CX_VERTEX_DATA_AOS_STRUCT
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
  vertexData->numTriangles = numTriangles;
  

#if !CX_VERTEX_DATA_AOS_STRUCT
  cxf32 *tangents = NULL;
  cx_vertex_data_calculate_tangent_array_aos (vertexData, &tangents);
  vertexData->tangents = tangents;
#else
  cx_vertex_data_sphere_compute_tangents_aos (vertexData);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_sphere_compute_tangents_aos (struct cx_vertex_data_aos *vertexData)
{
  // Reference: http://www.terathon.com/code/tangent.html
  
#if CX_VERTEX_DATA_AOS_STRUCT
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
    const cx_vec4 *n = &vertexData->vertices [i].normal;
    const cx_vec4 *t = &tan1 [i];
  
    if (cx_vec4_is_zero (t))
    {
      CX_DEBUG_BREAKABLE_EXPR;
    }
    
    if (cx_vec4_is_zero (n))
    {
      CX_DEBUG_BREAKABLE_EXPR;
    }
    
    // Gram-Schmidt orthogonalize
    
    cxf32 dotp = cx_vec4_dot (n, t);
    
    cx_vec4 ndotp;
    cx_vec4_mul (&ndotp, dotp, n);
    
    cx_vec4_sub (tangent, t, &ndotp);
    
    cxf32 tlen = cx_vec4_length (tangent);

    if (cx_is_zero (tlen))
    {
      cx_vec4_zero (tangent);
    }
    else
    {
      // normalize 
      cx_vec4_mul (tangent, (1.0f / tlen), tangent);
      
      // calculate handedness
      
      cx_vec4 cross;
      cx_vec4_cross (&cross, n, t);
      
      cxf32 dotp2 = cx_vec4_dot (&cross, &tan2 [i]);
      tangent->w = (dotp2 < 0.0f) ? -1.0f : 1.0f;
    }
  }
  
  cx_free (tan1);
#endif
}

void cx_vertex_data_calculate_tangent_array_aos (const struct cx_vertex_data_aos *vertexData, cxf32 **tangent)
{
#if !CX_VERTEX_DATA_AOS_STRUCT
  cxi32 vertexCount = vertexData->numVertices;
  cxi32 triangleCount = vertexData->numTriangles;
  
  cxf32 *tan1 = (cxf32 *) cx_malloc (sizeof (cxf32) * vertexCount * CX_VERTEX_TANGENT_SIZE * 2);  // tangent T
  cxf32 *tan2 = tan1 + (vertexCount * CX_VERTEX_TANGENT_SIZE);                                    // bitangent B
  
  cxi32 index = 0;
  
  for (cxi32 i = 0; i < triangleCount; ++i)
  {
    int a, b, c;
    
    a = index ++;
    CX_ASSERT (a < vertexData->numIndices);
    cxu16 i0 = vertexData->indices [a];
    
    b = index++;
    CX_ASSERT (b < vertexData->numIndices);
    cxu16 i1 = vertexData->indices [b];
    
    c = index++;
    CX_ASSERT (c < vertexData->numIndices);
    cxu16 i2 = vertexData->indices [c];
    
    
    cxi32 vi0 = i0 * CX_VERTEX_TOTAL_SIZE;
    cxi32 vi1 = i1 * CX_VERTEX_TOTAL_SIZE;
    cxi32 vi2 = i2 * CX_VERTEX_TOTAL_SIZE;
    
    cxi32 ti0 = i0 * CX_VERTEX_TANGENT_SIZE;
    cxi32 ti1 = i1 * CX_VERTEX_TANGENT_SIZE;
    cxi32 ti2 = i2 * CX_VERTEX_TANGENT_SIZE;
    
    cxf32 *pos0 = &vertexData->vertices [vi0];
    cxf32 *pos1 = &vertexData->vertices [vi1];
    cxf32 *pos2 = &vertexData->vertices [vi2];
    
    cxf32 *tex0 = &vertexData->vertices [vi0 + 6];
    cxf32 *tex1 = &vertexData->vertices [vi1 + 6];
    cxf32 *tex2 = &vertexData->vertices [vi2 + 6];
    
    cxf32 x1 = pos1 [0] - pos0 [0];
    cxf32 x2 = pos2 [0] - pos0 [0];
    cxf32 y1 = pos1 [1] - pos0 [1];
    cxf32 y2 = pos2 [1] - pos0 [1];
    cxf32 z1 = pos1 [2] - pos0 [2];
    cxf32 z2 = pos2 [2] - pos0 [2];
    
    cxf32 s1 = tex1 [0] - tex0 [0];
    cxf32 s2 = tex2 [0] - tex0 [0];
    cxf32 t1 = tex1 [1] - tex0 [1];
    cxf32 t2 = tex2 [1] - tex0 [1];
  
    cxf32 sdir [3], tdir [3];
    
    // scalar
    cxf32 r = ((s1 * t2) - (s2 * t1));
    
    if (fabsf (r) <= CX_EPSILON)
    {
      sdir [0] = 1.0f;
      sdir [1] = 0.0f;
      sdir [2] = 0.0f;
      
      tdir [0] = 0.0f;
      tdir [1] = 1.0f;
      tdir [2] = 0.0f;
    }
    else 
    {
      r = 1.0f / r;
      
      CX_ASSERT (cx_validatef (r));
      
      // 2x2 st matrix multiplied by 3x3 xyz matrix
      // 1st row = tangent
      // 2nd row = bitangent
      
      sdir [0] = ((t2 * x1) - (t1 * x2)) * r;
      sdir [1] = ((t2 * y1) - (t1 * y2)) * r;
      sdir [2] = ((t2 * z1) - (t1 * z2)) * r;
      
      tdir [0] = ((s1 * x2) - (s2 * x1)) * r;
      tdir [1] = ((s1 * y2) - (s2 * y1)) * r;
      tdir [2] = ((s1 * z2) - (s2 * z1)) * r;
    }

    // set tangent for each vertex
    
    tan1 [ti0 + 0] += sdir [0];
    tan1 [ti0 + 1] += sdir [1];
    tan1 [ti0 + 2] += sdir [2];
    
    tan1 [ti1 + 0] += sdir [0];
    tan1 [ti1 + 1] += sdir [1];
    tan1 [ti1 + 2] += sdir [2];
    
    tan1 [ti2 + 0] += sdir [0];
    tan1 [ti2 + 1] += sdir [1];
    tan1 [ti2 + 2] += sdir [2];
    
    // set bitangent for each vertex
    
    tan2 [ti0 + 0] += tdir [0];
    tan2 [ti0 + 1] += tdir [1];
    tan2 [ti0 + 2] += tdir [2];
    
    tan2 [ti1 + 0] += tdir [0];
    tan2 [ti1 + 1] += tdir [1];
    tan2 [ti1 + 2] += tdir [2];
    
    tan2 [ti2 + 0] += tdir [0];
    tan2 [ti2 + 1] += tdir [1];
    tan2 [ti2 + 2] += tdir [2];
  }
  
  *tangent = (cxf32 *) cx_malloc (sizeof (cxf32) * vertexCount * CX_VERTEX_TANGENT_SIZE * 2);
  
  for (cxi32 i = 0; i < vertexCount; ++i)
  {
    cxi32 vi = i * CX_VERTEX_TOTAL_SIZE;
    cxi32 ti = i * CX_VERTEX_TANGENT_SIZE;
    
    cxf32 *n = &vertexData->vertices [vi + 3]; // normal
    cxf32 *t = &tan1 [ti]; // tangent
    
    if ((fabsf (t [0]) <= CX_EPSILON) &&
        (fabsf (t [1]) <= CX_EPSILON) &&
        (fabsf (t [2]) <= CX_EPSILON))
    {
      CX_DEBUG_BREAKABLE_EXPR;
    }
    
    if ((fabsf (n [0]) <= CX_EPSILON) &&
        (fabsf (n [1]) <= CX_EPSILON) &&
        (fabsf (n [2]) <= CX_EPSILON))
    {
      CX_DEBUG_BREAKABLE_EXPR;
    }
    
    // Gram-Schmidt orthogonalize
    
    cxf32 dotnt = (n [0] * t [0]) + (n [1] * t [1]) + (n [2] * t [2]);
    
    cxf32 rt [3];
    rt [0] = t[0] - (n [0] * dotnt);
    rt [1] = t[1] - (n [1] * dotnt);
    rt [2] = t[2] - (n [2] * dotnt);
    
    cxf32 rtlenSqr = (rt [0] * rt [0]) + (rt [1] * rt [1]) + (rt [2] * rt [2]);
    
    if (fabsf (rtlenSqr) <= CX_EPSILON)
    {
      (*tangent) [ti + 0] = 0.0f;
      (*tangent) [ti + 1] = 0.0f;
      (*tangent) [ti + 2] = 0.0f;
    }
    else
    {
      CX_ASSERT (cx_validatef (rtlenSqr));
      
      cxf32 rtlenInv = 1.0f / sqrtf (rtlenSqr);
      
      CX_ASSERT (cx_validatef (rtlenInv));
      
      (*tangent) [ti + 0] = rt [0] * rtlenInv;
      (*tangent) [ti + 1] = rt [1] * rtlenInv;
      (*tangent) [ti + 2] = rt [2] * rtlenInv;
      
      cxf32 ntcross [3];
      
      ntcross [0] = (n [1] * t [2]) - (n [2] - t [1]);
      ntcross [1] = (n [2] * t [0]) - (n [0] - t [2]);
      ntcross [2] = (n [0] * t [1]) - (n [1] - t [0]);
    }
  }
  
  cx_free (tan1);
#endif
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
