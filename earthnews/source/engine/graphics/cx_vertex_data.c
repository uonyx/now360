//
//  cx_vertex_data.c
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#include "cx_vertex_data.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

//#define CX_VERTEX_TOTAL_SIZE (CX_VERTEX_POSITION_SIZE + CX_VERTEX_NORMAL_SIZE + CX_VERTEX_TEXCOORD_SIZE)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_VERTEX_DATA_AOS
void cx_vertex_data_create_sphere_aos (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_aos *vertexData);
void cx_vertex_data_destroy_aos (struct cx_vertex_data_aos *vertexData);
void cx_vertex_data_sphere_compute_tangents_aos (struct cx_vertex_data_aos *vertexData);
#else
void cx_vertex_data_create_sphere_soa (cxf32 radius, cxu16 numSlices, cxu16 numParallels, struct cx_vertex_data_soa *vertexData);
void cx_vertex_data_destroy_soa (struct cx_vertex_data_soa *vertexData);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_VERTEX_DATA_AOS
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
  for (i = 0; i < (numParallels + 1); ++i)
  {
    for (j = 0; j < (numSlices + 1); ++j)
    {
      cxf32 a0 = cx_sin (angleStep * (cxf32) i) * cx_sin (angleStep * (cxf32) j);
      cxf32 a1 = cx_cos (angleStep * (cxf32) i);
      cxf32 a2 = cx_sin (angleStep * (cxf32) i) * cx_cos (angleStep * (cxf32) j);
      
      vertex = (i * (numSlices + 1) + j);
      
      // position
      cx_vec4 * CX_RESTRICT position = &vertexData->vertices [vertex].position;
      cx_vec4_set (position, a0 * radius, a1 * radius, a2 * radius, 1.0f);
      
      // normal
      cx_vec4 * CX_RESTRICT normal = &vertexData->vertices [vertex].normal;
      cx_vec4_set (normal, a0, a1, a2, 0.0f);
      
      // tex coord
      cx_vec2 * CX_RESTRICT texCoord = &vertexData->vertices [vertex].texCoord;
      texCoord->x = (cxf32) j / (cxf32) numSlices;
      texCoord->y = (cxf32) i / (cxf32) numParallels;    
    }
  }
  
  // Generate the indices
  cxu16 *indexBuf = vertexData->indices;
  
  for (i = 0; i < numParallels; ++i)
  {
    for (j = 0; j < numSlices; ++j)
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
  
  memset (tan1, 0, (sizeof (cx_vec4) * vertCount * 2));
  
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

void cx_vertex_data_destroy_aos (struct cx_vertex_data_aos *vertexData)
{
  CX_ASSERT (vertexData);
  
  cx_free (vertexData->indices);
  cx_free (vertexData->vertices);
  
  vertexData->numIndices = 0;
  vertexData->numVertices = 0;
  vertexData->indices = NULL;
  vertexData->vertices = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#else
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
  for (i = 0; i < numParallels + 1; ++i)
  {
    for (j = 0; j < numSlices + 1; ++j)
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
  
  for (i = 0; i < numParallels; ++i)
  {
    for (j = 0; j < numSlices; ++j)
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
#endif

cx_vertex_data *cx_vertex_data_create_sphere (cxf32 radius, cxu16 slices, cx_vertex_format format)
{
  cx_vertex_data *vertexData = cx_malloc (sizeof (cx_vertex_data));
  
  vertexData->format = format;
  
#if CX_VERTEX_DATA_AOS
  cx_vertex_data_create_sphere_aos (radius, slices, (slices >> 1), vertexData);
#else
  cx_vertex_data_create_sphere_soa (radius, slices, (slices >> 1), vertexData);
#endif
  
  return vertexData;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_vertex_data_destroy (cx_vertex_data *vertexData)
{
#if CX_VERTEX_DATA_AOS
  cx_vertex_data_destroy_aos (vertexData);
#else
  cx_vertex_data_destroy_soa (vertexData);
#endif
  
  cx_free (vertexData);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
