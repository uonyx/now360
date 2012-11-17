//
//  earth.c
//  earthnews
//
//  Created by Ubaka Onyechi on 01/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "earth.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void earth_convert_dd_to_world (cx_vec4 *world, float latitude, float longitude, float radius, int slices, int parallels, cx_vec4 *n)
{
  // convert lat/long to texture paramter-space (uv coords)
  
  float tx = (longitude + 180.0f) / 360.0f;
  float ty = 1.0f - ((latitude + 90.0f) / 180.0f);
  
  // convert texture coords to sphere slices/parallels coords
  
  float i = ty * (float) parallels;
  float j = tx * (float) slices;
  
  // convert slices/parallels to world coords
  
  cxf32 angleStep = (2.0f * CX_PI) / (float) slices;
  
  cxf32 a0 = cx_sin (angleStep * i) * cx_sin (angleStep * j);
  cxf32 a1 = cx_cos (angleStep * i);
  cxf32 a2 = cx_sin (angleStep * i) * cx_cos (angleStep * j);
  
  world->x = radius * a0;
  world->y = radius * a1;
  world->z = radius * a2;
  world->w = 1.0f;
  
  // compute normal for ray picking
  
  if (n)
  {
    n->x = a0;
    n->y = a1;
    n->z = a2;
    n->w = 0.0f;
    
    cx_vec4_normalize (n);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct earth_data_t *earth_data_create (const char *filename, float radius, int slices, int parallels)
{
  struct earth_data_t *data = NULL;
  
  cx_file file;
  cx_file_load (&file, filename);
  
  char errorBuffer [128];
  json_settings settings;
  memset (&settings, 0, sizeof (settings));
  
  json_value *root = json_parse_ex (&settings, file.data, file.size, errorBuffer, 128);
  
  if (root)
  {
    CX_ASSERT (root->type == json_array);
    
    data = (struct earth_data_t *) cx_malloc (sizeof (struct earth_data_t));
    
    unsigned int count = root->u.array.length;
    
    data->count = count;
    data->location = (cx_vec4 *) cx_malloc (sizeof (cx_vec4) * count);
    data->normal = (cx_vec4 *) cx_malloc (sizeof (cx_vec4) * count);
    data->names = (const char **) cx_malloc (sizeof (char *) * count);
    data->newsFeeds = (const char **) cx_malloc (sizeof (char *) * count);
    data->weatherId = (const char **) cx_malloc (sizeof (char *) * count);
    
    unsigned int i;
    for (i = 0; i < count; ++i)
    {
      json_value *value = root->u.array.values [i];
      CX_ASSERT (value->type == json_object);
      
      unsigned int j;
      for (j = 0; j < value->u.object.length; ++j)
      {
        json_value *v = value->u.object.values [j].value;
        const char *vn = value->u.object.values [j].name;
        
        if (strcmp (vn, "name") == 0)
        {
          CX_ASSERT (v->type == json_string);
          
          data->names [i] = cx_strdup (v->u.string.ptr, v->u.string.length);
        }
        else if (strcmp (vn, "news") == 0)
        {
          CX_ASSERT (v->type == json_string);
          
          data->newsFeeds [i] = cx_strdup (v->u.string.ptr, v->u.string.length);
        }
        else if (strcmp (vn, "weather") == 0)
        {
          CX_ASSERT (v->type == json_string);
          
          data->weatherId [i] = cx_strdup (v->u.string.ptr, v->u.string.length);
        }
        else if (strcmp (vn, "location") == 0)
        {
          CX_ASSERT (v->type == json_object);
          CX_ASSERT (v->u.object.length == 2);
          
          CX_ASSERT (v->u.object.values [0].value->type == json_double);
          CX_ASSERT (v->u.object.values [1].value->type == json_double);
          
          CX_ASSERT (strcmp (v->u.object.values [0].name, "latitude") == 0);
          CX_ASSERT (strcmp (v->u.object.values [1].name, "longitude") == 0);
          
          float lat = (float) v->u.object.values [0].value->u.dbl;
          float lon = (float) v->u.object.values [1].value->u.dbl;
          float r = radius + (radius * 0.015f); // slightly extend radius (for point sprite rendering)
          
          earth_convert_dd_to_world (&data->location [i], lat, lon, r, slices, parallels, &data->normal [i]);
        }
      }
    }  
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, errorBuffer);
    data = NULL;
  }
  
  cx_file_unload (&file);
  
  return data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct earth_visual_t *earth_visual_create (float radius, int slices, int parallels)
{
  CX_ASSERT (radius > 0.0f);
  CX_ASSERT (slices > 0);
  CX_ASSERT (parallels > 0);
  CX_ASSERT (slices <= ((1 << 16) - 1));
  CX_ASSERT (parallels <= ((1 << 16) - 1));
      
  struct earth_visual_t *visual = (struct earth_visual_t *) cx_malloc (sizeof (struct earth_visual_t));
  
  cx_shader *shader     = cx_shader_create ("mesh", "data/shaders");
  cx_material *material = cx_material_create ("earth");
  
  cx_texture *texture   = cx_texture_create_from_file ("data/textures/earthmap1k.png");
  //cx_texture *texture     = cx_texture_create_from_file ("data/maps/monthly/07-4096.png");
  //cx_texture *bumpTexture = cx_texture_create_from_file ("data/maps/4096-normal.png");
  
  cx_material_attach_texture (material, texture, CX_MATERIAL_TEXTURE_DIFFUSE);
  //cx_material_attach_texture (material, bumpTexture, CX_MATERIAL_TEXTURE_BUMP);
  
  visual->nightMap = cx_texture_create_from_file ("data/maps/2048-night.png");
  //visual->nightMap = cx_texture_create_from_file ("data/maps/4096-night.png");
  
  visual->mesh = cx_mesh_create_sphere (radius, (short) slices, (short) parallels, shader, material);
  
  return visual;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

earth_t *earth_create (const char *filename, float radius, int slices, int parallels)
{
  earth_t *earth = (earth_t *) cx_malloc (sizeof (earth_t));
  
  // create earth data    
  earth->data = earth_data_create (filename, radius, slices, parallels);
  CX_FATAL_ASSERT (earth->data);
  
  // create earth visual
  earth->visual = earth_visual_create (radius, slices, parallels);
  CX_FATAL_ASSERT (earth->visual);
  
  return earth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_destroy (earth_t *earth)
{
  cx_free (earth);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_render (earth_t *earth)
{
  CX_ASSERT (earth);
  
  int locCount = earth->data->count;
  
  
  // draw points
  int i;
  for (i = 0; i < locCount; ++i)
  {
    
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
