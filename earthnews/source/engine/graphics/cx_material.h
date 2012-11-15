//
//  cx_material.h
//
//  Created by Ubaka Onyechi on 26/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_MATERIAL_H
#define CX_MATERIAL_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_colour.h"
#include "cx_texture.h"
#include "cx_shader.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_MATERIAL_DEBUG               (CX_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_material_texture
{
  CX_MATERIAL_TEXTURE_DIFFUSE,
  CX_MATERIAL_TEXTURE_BUMP,
  
  CX_NUM_MATERIAL_TEXTURES,
} cx_material_texture;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_material_map
{
  CX_MATERIAL_MAP_DIFFUSE,
  CX_MATERIAL_MAP_GLOSS,
  CX_MATERIAL_MAP_GLOW,
} cx_material_map;

typedef struct cx_material
{ 
  const cx_texture *textures [CX_NUM_MATERIAL_TEXTURES];
  
  cx_colour ambient;
  cx_colour diffuse;    // texture (diffuse map). same as ambient?
  cx_colour specular;   // grayscale (gloss map). single channel?
  cx_colour emmissive;  // texture (glow map). night time
  
  cxf32 shininess;
  cxu32 properties;
  
  bool twoSided;
  bool alpha;

#if CX_MATERIAL_DEBUG
  char *name;
#endif
  
} cx_material;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_material *cx_material_create (const char *name);
void cx_material_destroy (cx_material *material);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_material_render (const cx_material *material, const cx_shader *shader);
void cx_material_attach_texture (cx_material *material, const cx_texture *texture, cx_material_texture type);
void cx_material_detach_texture (cx_material *material, cx_material_texture type);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
