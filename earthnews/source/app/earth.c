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

#define NEW_EARTH_SHADER 1

static struct earth_visual_t *earth_visual_create (float radius, int slices, int parallels)
{
  CX_ASSERT (radius > 0.0f);
  CX_ASSERT (slices > 0);
  CX_ASSERT (parallels > 0);
  CX_ASSERT (slices <= ((1 << 16) - 1));
  CX_ASSERT (parallels <= ((1 << 16) - 1));
      
  struct earth_visual_t *visual = (struct earth_visual_t *) cx_malloc (sizeof (struct earth_visual_t));
  
#if NEW_EARTH_SHADER
  cx_shader *shader     = cx_shader_create ("earth", "data/shaders");
  
  cx_material *material = cx_material_create ("earth");
  
  //cx_texture *texture   = cx_texture_create_from_file ("data/textures/earthmap1k.png");
  //cx_texture *texture     = cx_texture_create_from_file ("data/maps/4096-clean.png");
  //cx_texture *texture     = cx_texture_create_from_file ("data/maps/4096-diff.png");
  cx_texture *texture     = cx_texture_create_from_file ("data/maps/monthly/07-4096.png");
  cx_texture *bumpTexture = cx_texture_create_from_file ("data/maps/4096-normal.png");
  //cx_texture *bumpTexture = cx_texture_create_from_file ("data/maps/4096-normal-30.png");
  cx_texture *specTexture = cx_texture_create_from_file ("data/maps/2048-spec.png");
  
  cx_material_attach_texture (material, texture, CX_MATERIAL_TEXTURE_DIFFUSE);
  cx_material_attach_texture (material, specTexture, CX_MATERIAL_TEXTURE_SPECULAR);
  cx_material_attach_texture (material, bumpTexture, CX_MATERIAL_TEXTURE_BUMP);
  
  //visual->nightMap = cx_texture_create_from_file ("data/maps/2048-night.png");
  visual->nightMap = cx_texture_create_from_file ("data/maps/4096-night.png");
  
#else
  cx_shader *shader     = cx_shader_create ("mesh", "data/shaders");
  
  cx_material *material = cx_material_create ("earth");
  
  cx_texture *texture   = cx_texture_create_from_file ("data/textures/earthmap1k.png");
  //cx_texture *texture     = cx_texture_create_from_file ("data/maps/monthly/07-4096.png");
  
  cx_material_attach_texture (material, texture, CX_MATERIAL_TEXTURE_DIFFUSE);
  //cx_material_attach_texture (material, bumpTexture, CX_MATERIAL_TEXTURE_BUMP);
  
  visual->nightMap = cx_texture_create_from_file ("data/maps/2048-night.png");
#endif
  
  cx_vertex_data *sphere = cx_vertex_data_create_sphere (radius, (short) slices, (short) parallels, CX_VERTEX_FORMAT_PTNTB);
  // destroy SPHERE
  
  visual->mesh = cx_mesh_create (sphere, shader, material);

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
  CX_ASSERT (earth);
  
  cx_mesh_destroy (earth->visual->mesh);
  cx_free (earth);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_render (const earth_t *earth, const cx_date *date, const cx_vec4 *eye)
{
  CX_ASSERT (earth);
  CX_ASSERT (date);
  CX_ASSERT (eye);
  
#if NEW_EARTH_SHADER
  cx_mat4x4 mvpMatrix;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
  // get mesh
  cx_mesh *mesh = earth->visual->mesh;
  
  // use shader
  cx_shader_use (mesh->shader);
  
  // set u_mvpMatrix
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
  
  // night map
  cx_texture *nightMap = earth->visual->nightMap;
  cx_shader_set_texture (mesh->shader, "u_nightMap", nightMap, 3);
  
  cx_vec4 eyePos, lightPos;
  cx_vec4_set (&eyePos, 0.0f, 0.0f, -3.0f, 1.0f);
  cx_vec4_set (&lightPos, 0.0f, 0.0f, -4.0f, 1.0f);
  
#if 1
  static float angle = 0.0f;
  angle += (1.0f / 6.0f);
  angle = fmodf (angle, 360.0f);
#else
  int hr = date->calendar.tm_hour;
  int mn = date->calendar.tm_min;
  int sc = date->calendar.tm_sec;
  int seconds = ((hr * 60) + mn) * 60 + sc;
  int secondsInDay = 86400;
  
  CX_ASSERT ((seconds >= 0) && (seconds <= secondsInDay));
  
  float angle = ((float) seconds / (float) secondsInDay) * 360.0f;
  angle += 180.0f;
  angle = fmodf (angle, 360.0f);
#endif
  
  // get rotation matrix
  cx_mat4x4 rotation;
  cx_mat4x4_rotation (&rotation, cx_rad (angle), 0.0f, -1.0f, 0.0f);
  
  cx_vec4 origin;
  cx_vec4_set (&origin, 0.0f, 0.0f, 0.0f, 1.0f);
  
  cx_vec4 forward;
  cx_vec4_sub (&forward, &lightPos, &origin);
  
  // transform forward vector
  cx_vec4 newForward;
  cx_mat4x4_mul_vec4 (&newForward, &rotation, &forward);
  
  // update light position
  cx_vec4_add (&lightPos, &origin, &newForward);
  
  eyePos = *eye;
  
  cx_shader_set_vector4 (mesh->shader, "u_eyePosition", &eyePos, 1);
  cx_shader_set_vector4 (mesh->shader, "u_lightPosition", &lightPos, 1);
  
  float shininess = 0.8f;
  const cx_colour *white = cx_colour_white ();
  
  cx_colour dark;
  //cx_colour_set (&dark, 0.15f, 0.15f, 0.15f, 1.0f);
  cx_colour_set (&dark, 0.05f, 0.05f, 0.05f, 1.0f);
  
  
  cx_shader_set_vector4 (mesh->shader, "u_ambientLight", &dark, 1);
  cx_shader_set_vector4 (mesh->shader, "u_diffuseLight", white, 1);
  cx_shader_set_vector4 (mesh->shader, "u_specularLight", white, 1);
  cx_shader_set_float (mesh->shader, "u_shininess", &shininess, 1);
  
  // set u_diffuseMap, u_normalMap
  cx_mesh_render (mesh);
  
#else
  cx_mat4x4 mvpMatrix;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
  cx_mat3x3 normalMatrix;
  cx_mat3x3_identity (&normalMatrix);
  
  // get mesh
  cx_mesh *mesh = earth->visual->mesh;
  
  // use shader
  cx_shader_use (mesh->shader);
  
  // set matrices
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_N, &normalMatrix);
  
  // night map
  cx_texture *nightMap = earth->visual->nightMap;
  cx_shader_set_texture (mesh->shader, "u_nightMap", nightMap, 3);

  cx_mesh_render (mesh);
#endif
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
