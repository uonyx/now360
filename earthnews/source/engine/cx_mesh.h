//
//  cx_mesh.h
//
//  Created by Ubaka Onyechi on 26/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_MESH_H
#define CX_MESH_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"
#include "cx_colour.h"
#include "cx_vector4.h"
#include "cx_shader.h"
#include "cx_material.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_MESH_DEBUG_LOG         1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_VERTEX_DATA_AOS        1
#define CX_VERTEX_DATA_AOS_STRUCT 0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_VERTEX_POSITION_SIZE   3
#define CX_VERTEX_NORMAL_SIZE     3
#define CX_VERTEX_TEXCOORD_SIZE   2
#define CX_VERTEX_TOTAL_SIZE      (CX_VERTEX_POSITION_SIZE + CX_VERTEX_NORMAL_SIZE + CX_VERTEX_TEXCOORD_SIZE)

#if CX_VERTEX_DATA_AOS
#define CX_VERTEX_BUFFER_COUNT    2
#else
#define CX_VERTEX_BUFFER_COUNT    4
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_VERTEX_DATA_AOS_STRUCT
typedef struct cx_vertex
{
  cx_vec4 position;
  cx_vec4 normal;
  cx_vec2 texCoord;
} cx_vertex;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct cx_vertex_data_aos
{
#if CX_VERTEX_DATA_AOS_STRUCT
  cx_vertex *vertices;
#else
  cxf32 *vertices;
#endif
  cxu16 *indices;
  cxi32 numVertices;
  cxi32 numIndices;
};

struct cx_vertex_data_soa
{
  cxf32 *positions;
  cxf32 *texCoords;
  cxf32 *normals;
  cxu16 *indices;
  cxi32 numVertices;
  cxi32 numIndices;
};

#if CX_VERTEX_DATA_AOS
typedef struct cx_vertex_data_aos cx_vertex_data;
#else
typedef struct cx_vertex_data_soa cx_vertex_data;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_mesh
{
  cx_vertex_data vertexData;
  cxu32 vbos [CX_VERTEX_BUFFER_COUNT];
  cxu32 vao;
  cx_shader *shader;
  cx_material *material;
} cx_mesh;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_mesh *cx_mesh_create_sphere (cxu16 slices, cxf32 radius, cx_shader *shader, cx_material *material);
void cx_mesh_destroy (cx_mesh *mesh);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_mesh_render (cx_mesh *mesh);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
