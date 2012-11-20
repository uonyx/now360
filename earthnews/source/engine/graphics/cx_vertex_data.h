//
//  cx_vertex_data.h
//
//  Created by Ubaka Onyechi on 20/11/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_VERTEX_DATA_H
#define CX_VERTEX_DATA_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../system/cx_system.h"
#include "../system/cx_vector4.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_VERTEX_DATA_AOS        1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_VERTEX_POSITION_SIZE   3
#define CX_VERTEX_NORMAL_SIZE     3
#define CX_VERTEX_TEXCOORD_SIZE   2
#define CX_VERTEX_TANGENT_SIZE    3

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_vertex_format
{
  CX_VERTEX_FORMAT_INVALID,
  CX_VERTEX_FORMAT_P,       // position
  CX_VERTEX_FORMAT_PT,      // position, texcoord
  CX_VERTEX_FORMAT_PTN,     // position, texcoord, normal
  CX_VERTEX_FORMAT_PTNTB,   // position, texcoord, normal, tangent, bitangent
  NUM_VERTEX_FORMATS
} cx_vertex_format;

typedef struct cx_vertex
{
  cx_vec4 position;
  cx_vec4 normal;
  cx_vec4 tangent;
  cx_vec4 bitangent;
  cx_vec2 texCoord;
} cx_vertex;

#if CX_VERTEX_DATA_AOS
struct cx_vertex_data_aos
{
  cx_vertex *vertices;
  cxu16 *indices;
  cxi32 numVertices;
  cxi32 numIndices;
  cxi32 numTriangles;
  cx_vertex_format format;
};
typedef struct cx_vertex_data_aos cx_vertex_data;
#else
struct cx_vertex_data_soa
{
  cxf32 *positions;
  cxf32 *texCoords;
  cxf32 *normals;
  cxu16 *indices;
  cxi32 numVertices;
  cxi32 numIndices;
  cxi32 numTriangles;
  cx_vertex_format format;
};
typedef struct cx_vertex_data_soa cx_vertex_data;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_vertex_data *cx_vertex_data_create_sphere (cxf32 radius, cxu16 numSlices, cxu16 numParallels, cx_vertex_format format);

void cx_vertex_data_destroy (cx_vertex_data *vertexData);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
