//
//  earth.c
//  earthnews
//
//  Created by Ubaka Onyechi on 01/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "earth.h"
#include "util.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NEW_EARTH_SHADER (!TARGET_IPHONE_SIMULATOR && 1)
#define ENABLE_TOPOGRAPHY 1
#define ENABLE_CLOUDS 1
#define ENABLE_ATMOSPHERE 1
#define BUMP_MAPPED_CLOUDS 0
#define DEBUG_FAST_LIGHT_ORBIT (CX_DEBUG && 0)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define OPTIMIZED_WEATHER_DATA 1
#define WEATHER_ID_MAX_LEN 16

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct earth_visual_t
{
  float radius;
  int slices;
  int parallels;
  
  cx_mesh *mesh [3];
  cx_texture *nightMap;
};

struct earth_data_t
{
  const char **names;
  const char **newsFeeds;
  
#if OPTIMIZED_WEATHER_DATA
  char *weatherId;
#else
  const char **weatherId;
#endif
  
  cx_vec4 *location;
  cx_vec4 *normal;
  int *utcOffset;
  int *dstOffset;
  const char **tznames;
  int count;
};

struct earth_t
{
  struct earth_data_t *data;
  struct earth_visual_t *visual;
};

typedef struct earth_t earth_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static earth_t *g_earth = NULL;

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

static struct earth_data_t *earth_data_create (const char *filename, float radius, int slices)
{
  struct earth_data_t *earthdata = NULL;
  
  cxu8 *filedata = NULL;
  cxu32 filedataSize = 0;
  
  if (cx_file_storage_load_contents (&filedata, &filedataSize, filename, CX_FILE_STORAGE_BASE_RESOURCE))
  {
    cx_json_tree jsonTree = cx_json_tree_create ((const char *) filedata, filedataSize);
    
    if (jsonTree)
    {
      cx_json_node rootNode = cx_json_tree_root_node (jsonTree);
      
      unsigned int count = cx_json_array_size (rootNode);
      
      earthdata = (struct earth_data_t *) cx_malloc (sizeof (struct earth_data_t));
      
      earthdata->count = count;
      earthdata->location = (cx_vec4 *) cx_malloc (sizeof (cx_vec4) * count);
      earthdata->normal = (cx_vec4 *) cx_malloc (sizeof (cx_vec4) * count);
      earthdata->names = (const char **) cx_malloc (sizeof (char *) * count);
      earthdata->newsFeeds = (const char **) cx_malloc (sizeof (char *) * count);
#if OPTIMIZED_WEATHER_DATA
      earthdata->weatherId = (char *) cx_malloc (sizeof (char) * WEATHER_ID_MAX_LEN * count);
#else
      earthdata->weatherId = (const char **) cx_malloc (sizeof (char *) * count);
#endif
      earthdata->utcOffset = (int *) cx_malloc (sizeof (int) * count);
      earthdata->dstOffset = (int *) cx_malloc (sizeof (int) * count);
      earthdata->tznames = (const char **) cx_malloc (sizeof (char *) * count);
    
      
      memset (earthdata->location, 0, (sizeof (cx_vec4) * count));
      memset (earthdata->normal, 0, (sizeof (cx_vec4) * count));
      memset (earthdata->names, 0, (sizeof (char *) * count));
      memset (earthdata->newsFeeds, 0, (sizeof (char *) * count));
#if OPTIMIZED_WEATHER_DATA
      memset (earthdata->weatherId, 0, sizeof (char) * WEATHER_ID_MAX_LEN * count);
#else
      memset (earthdata->weatherId, 0, (sizeof (char *) * count));
#endif
      memset (earthdata->utcOffset, 0, (sizeof (int) * count));
      memset (earthdata->dstOffset, 0, (sizeof (int) * count));
      memset (earthdata->tznames, 0, (sizeof (char *) * count));
      
      for (unsigned int i = 0; i < count; ++i)
      {
        cx_json_node objectNode = cx_json_array_member (rootNode, i);
       
        unsigned int objectLength = cx_json_object_length (objectNode);
        
        for (unsigned int j = 0; j < objectLength; ++j)
        {
          cx_json_node p = cx_json_object_child_node (objectNode, j);
          const char *pk = cx_json_object_child_key (objectNode, j);
          
          if (strcmp (pk, "name") == 0)
          {
            const char *str = cx_json_value_string (p);
            earthdata->names [i] = cx_strdup (str, 32);
          }
          else if (strcmp (pk, "news") == 0)
          {
            const char *str = cx_json_value_string (p);
            earthdata->newsFeeds [i] = cx_strdup (str, 32);
          }
          else if (strcmp (pk, "weather") == 0)
          {
            const char *str = cx_json_value_string (p);
            
#if OPTIMIZED_WEATHER_DATA
            int loc = i * WEATHER_ID_MAX_LEN;
            
            char *d = &earthdata->weatherId [loc];
            
            cx_strcpy (d, WEATHER_ID_MAX_LEN, str);
#else
            earthdata->weatherId [i] = cx_strdup (str, WEATHER_ID_MAX_LEN);
#endif
          }
          else if (strcmp (pk, "timezone") == 0)
          {
            cx_json_node utcNode = cx_json_object_child (p, "utcoffset");
            cx_json_node tznNode = cx_json_object_child (p, "name");
            
            const char *tzn = cx_json_value_string (tznNode);
            unsigned int tznlen = cx_roundupPow2 (strlen (tzn));
            
            if (tznlen > 0)
            {
              earthdata->tznames [i] = cx_strdup (tzn, tznlen);
            }
            else
            {
              earthdata->tznames [i] = cx_strdup ("", WEATHER_ID_MAX_LEN);
            }
            
            int utcOffsetSecs = cx_json_value_int (utcNode);
            int dstOffsetSecs = util_get_dst_offset_secs (tzn);
            
            earthdata->dstOffset [i] = dstOffsetSecs;
            
            earthdata->utcOffset [i] = utcOffsetSecs;
          }
          else if (strcmp (pk, "location") == 0)
          {
            cx_json_node latNode = cx_json_object_child (p, "latitude");
            cx_json_node lonNode = cx_json_object_child (p, "longitude");
            
            float lat = cx_json_value_float (latNode);
            float lon = cx_json_value_float (lonNode);
            
            float r = radius + (radius * 0.025f); // slightly extend radius (for point sprite rendering)
            
            earth_convert_dd_to_world (&earthdata->location [i], lat, lon, r, slices, (slices >> 1), &earthdata->normal [i]);
          }
        }
      }
      
      cx_json_tree_destroy (jsonTree);
    }
    else
    {
      CX_DEBUGLOG_CONSOLE (1, "JSON parse error: %s", filename);
    }
    
    cx_free (filedata);
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "Failed to load %s", filename);
  }

  return earthdata;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct earth_visual_t *earth_visual_create (const cx_date *date, float radius, int slices)
{
  CX_ASSERT (radius > 0.0f);
  CX_ASSERT (slices > 0);
  CX_ASSERT (slices <= ((1 << 16) - 1));
  
  int month = date->calendar.tm_mon + 1;  
  char diffmap [128];
  cx_sprintf (diffmap, 128, "data/images/maps/diff-%02d-4096.%s", month, "png");

  struct earth_visual_t *visual = (struct earth_visual_t *) cx_malloc (sizeof (struct earth_visual_t));
  
#if NEW_EARTH_SHADER
  cx_shader *shader       = cx_shader_create ("earth", "data/shaders");
  cx_material *material   = cx_material_create ("earth");
  
  cx_texture *specTexture = cx_texture_create_from_file ("data/maps/2048-spec.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *bumpTexture = cx_texture_create_from_file ("data/maps/2048-normal.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *texture     = cx_texture_create_from_file ("data/images/maps/diff-01-4096b.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  
  cx_material_set_texture (material, texture, CX_MATERIAL_TEXTURE_DIFFUSE);
  cx_material_set_texture (material, specTexture, CX_MATERIAL_TEXTURE_SPECULAR);
  cx_material_set_texture (material, bumpTexture, CX_MATERIAL_TEXTURE_BUMP);
  
  visual->nightMap = cx_texture_create_from_file ("data/images/maps/nightm-01-4096.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  
#else
  cx_shader *shader     = cx_shader_create ("mesh", "data/shaders");
  cx_material *material = cx_material_create ("earth");

  cx_texture *texture   = cx_texture_create_from_file ("data/maps/earthmap1k.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_material_set_texture (material, texture, CX_MATERIAL_TEXTURE_DIFFUSE);
  
  visual->nightMap = cx_texture_create_from_file ("data/maps/2048-night.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
#endif
  
  cx_vertex_data *sphere = cx_vertex_data_create_sphere (radius, (short) slices, CX_VERTEX_FORMAT_PTNTB);
  
  visual->mesh [0] = cx_mesh_create (sphere, shader, material);
  
  //cx_vertex_data_destroy (sphere);
  //////////////////////////////////////////////////////////////////////////////////////////
#if ENABLE_CLOUDS
  cx_shader *shader1     = cx_shader_create ("clouds", "data/shaders");
  cx_material *material1 = cx_material_create ("clouds");
  
  cx_texture *cloudImage = cx_texture_create_from_file ("data/maps/2048-clouds.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_material_set_texture (material1, cloudImage, CX_MATERIAL_TEXTURE_DIFFUSE);
  
#if BUMP_MAPPED_CLOUDS
  cx_texture *cloudBump = cx_texture_create_from_file ("data/maps/2048-normal-clouds.png", CX_FILE_STORAGE_BASE_RESOURCE true);
  cx_material_set_texture (material1, cloudBump, CX_MATERIAL_TEXTURE_BUMP);
#endif
  
  float radius1 = radius + 0.01f;
#if BUMP_MAPPED_CLOUDS
  cx_vertex_data *sphere1 = cx_vertex_data_create_sphere (radius1, 32, CX_VERTEX_FORMAT_PTNTB);
#else
  cx_vertex_data *sphere1 = cx_vertex_data_create_sphere (radius1, 32, CX_VERTEX_FORMAT_PTN);
#endif
  
  visual->mesh [1] = cx_mesh_create (sphere1, shader1, material1);
#endif
  
  //////////////////////////////////////////////////////////////////////////////////////////
#if ENABLE_ATMOSPHERE
  float radius2 = radius + 0.02f;
  cx_vertex_data *sphere2 = cx_vertex_data_create_sphere (radius2, 96, CX_VERTEX_FORMAT_PN);
  
  cx_shader *shader2 = cx_shader_create ("atmos", "data/shaders");
  cx_material *material2 = cx_material_create ("atmos");
  
  material2->diffuse = *cx_colour_blue ();
  
  visual->mesh [2] = cx_mesh_create (sphere2, shader2, material2);
#endif
  
  return visual;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void earth_visual_get_sun_position (cx_vec4 *position, const cx_date *date)
{
  CX_ASSERT (position);
  CX_ASSERT (date);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void earth_visual_render (const cx_vec4 *eye, const cx_date *date)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (eye);
  CX_ASSERT (date);
  
#if NEW_EARTH_SHADER
  
  cx_mat4x4 mvpMatrix;
  cx_vec4 eyePos, lightPos;
  
  cx_colour ambient, diffuse, specular;
  float shininess = 0.0f;
  
  CX_REF_UNUSED (ambient);
  CX_REF_UNUSED (diffuse);
  CX_REF_UNUSED (specular);
  CX_REF_UNUSED (shininess);
  
  /////////////////////////////
  // mvp matrix
  /////////////////////////////
  
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
  /////////////////////////////
  // eye position
  /////////////////////////////
  
  cx_vec4_set (&eyePos, 0.0f, 0.0f, -3.0f, 1.0f);
  eyePos = *eye;
  
  /////////////////////////////
  // light position
  /////////////////////////////
  
  cx_vec4 sunPos;
  earth_visual_get_sun_position (&sunPos, date);
  
#if DEBUG_FAST_LIGHT_ORBIT
  static float angle = 0.0f;
  angle += (1.0f / 3.0f);
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
  
  cx_vec4_set (&lightPos, 0.0f, 0.0f, -4.0f, 1.0f);

#if 0
  // get rotation matrix
  cx_mat4x4 rotation2;
  cx_mat4x4_rotation (&rotation2, cx_rad (45.0f), 1.0f, 0.0f, 0.0f);
  
  cx_vec4 origin2;
  cx_vec4_set (&origin2, 0.0f, 0.0f, 0.0f, 1.0f);
  
  cx_vec4 forward2;
  cx_vec4_sub (&forward2, &lightPos, &origin2);
  
  // transform forward vector
  cx_vec4 newForward2;
  cx_mat4x4_mul_vec4 (&newForward2, &rotation2, &forward2);
  
  // update light position
  cx_vec4_add (&lightPos, &origin2, &newForward2);
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

  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////////////////////
  
#if ENABLE_TOPOGRAPHY
  
  {
    cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_DEPTH_TEST);
    cx_gdi_enable_z_write (true);
    
    // get mesh
    cx_mesh *mesh = g_earth->visual->mesh [0];
    
    // use shader
    cx_shader_begin (mesh->shader);
    
    // set u_mvpMatrix
    cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
    
    // night map
    
    cx_texture *nightMap = g_earth->visual->nightMap;
    cx_shader_set_texture (mesh->shader, "u_nightMap", nightMap, 3);
    
    cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_EYE_POSITION, &eyePos);
    cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_LIGHT_POSITION, &lightPos);
    
    shininess = 0.5f;
    
    cx_colour_set (&ambient, 0.15f, 0.15f, 0.15f, 1.0f);
    cx_colour_set (&diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
    cx_colour_set (&specular, 0.9f, 0.9f, 0.9f, 1.0f);
    cx_colour_set (&specular, 1.0f, 250.0f/255.0f, 205.0f/255.0f, 1.0f);
    
    cx_shader_set_vector4 (mesh->shader, "u_ambientLight", &ambient, 1);
    cx_shader_set_vector4 (mesh->shader, "u_diffuseLight", &diffuse, 1);
    cx_shader_set_vector4 (mesh->shader, "u_specularLight", &specular, 1);
    cx_shader_set_float (mesh->shader, "u_shininess", &shininess, 1);
    
    cx_mesh_render (mesh);   // set u_diffuseMap, u_normalMap
    
    cx_shader_end (mesh->shader);
  }
  
#endif
  
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////////////////////
  
#if ENABLE_CLOUDS
  
  {
    cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
    cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
    cx_gdi_enable_z_write (false);
    
    cx_mesh *mesh1 = g_earth->visual->mesh [1];
    
    cx_shader_begin (mesh1->shader);
    
    cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
    cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_LIGHT_POSITION, &lightPos);
    
#if BUMP_MAPPED_CLOUDS
    cx_colour_set (&ambient, 0.0f, 0.0f, 0.0f, 0.0f);
    cx_colour_set (&diffuse, 0.8f, 0.8f, 0.8f, 1.0f);
    cx_colour_set (&specular, 0.0f, 0.0f, 0.0f, 0.0f);
    
    shininess = 1.5f;
    
    cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_EYE_POSITION, &eyePos);
    cx_shader_set_vector4 (mesh1->shader, "u_ambientLight", &ambient, 1);
    cx_shader_set_vector4 (mesh1->shader, "u_diffuseLight", &diffuse, 1);
    cx_shader_set_vector4 (mesh1->shader, "u_specularLight", &specular, 1);
    cx_shader_set_float (mesh1->shader, "u_shininess", &shininess, 1);
#endif
    
    cx_mesh_render (mesh1);
    
    cx_shader_end (mesh1->shader);
    
    cx_gdi_enable_z_write (true);
  }
  
#endif
  
  ////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////
  
#if ENABLE_ATMOSPHERE
  
  {
    cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
    cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
    cx_gdi_enable_z_write (false);
    
    cx_mesh *mesh2 = g_earth->visual->mesh [2];
    
    cx_shader_begin (mesh2->shader);
    
    cx_shader_set_uniform (mesh2->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
    cx_shader_set_uniform (mesh2->shader, CX_SHADER_UNIFORM_LIGHT_POSITION, &lightPos);
    cx_shader_set_uniform (mesh2->shader, CX_SHADER_UNIFORM_EYE_POSITION, &eyePos);
    
    cx_shader_end (mesh2->shader);
    
    cx_mesh_render (mesh2);
    cx_gdi_enable_z_write (true);
  }
  
#endif
  
#else
  cx_mat4x4 mvpMatrix;
  cx_gdi_get_transform (CX_GDI_TRANSFORM_MVP, &mvpMatrix);
  
  cx_mat3x3 normalMatrix;
  cx_mat3x3_identity (&normalMatrix);
  
  cx_gdi_set_renderstate (CX_GDI_RENDER_STATE_CULL | CX_GDI_RENDER_STATE_BLEND | CX_GDI_RENDER_STATE_DEPTH_TEST);
  cx_gdi_set_blend_mode (CX_GDI_BLEND_MODE_SRC_ALPHA, CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA);
  cx_gdi_enable_z_write (true);
  
  // get mesh
  cx_mesh *mesh = g_earth->visual->mesh [0];
  
  // use shader
  cx_shader_begin (mesh->shader);
  
  // set matrices
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
  cx_shader_set_uniform (mesh->shader, CX_SHADER_UNIFORM_TRANSFORM_N, &normalMatrix);
  
  // night map
  cx_texture *nightMap = g_earth->visual->nightMap;
  cx_shader_set_texture (mesh->shader, "u_nightMap", nightMap, 3);
  
  cx_mesh_render (mesh);
  
  cx_shader_end (mesh->shader);
  
  cx_gdi_enable_z_write (false);
#endif
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static earth_t *earth_create (const char *filename, const cx_date *date)
{
  CX_ASSERT (filename);
  CX_ASSERT (date);
  
  earth_t *earth = (earth_t *) cx_malloc (sizeof (earth_t));
  
  float radius = 1.0f;
#if 1
  // ipad 2 configuration
  int slices = 64;
  //int parallels = 32;
#else
  int slices = 128;
  //int parallels = 64;
#endif
  
  // create earth data    
  earth->data = earth_data_create (filename, radius, slices);
  CX_FATAL_ASSERT (earth->data);
  
  // create earth visual
  earth->visual = earth_visual_create (date, radius, slices);
  CX_FATAL_ASSERT (earth->visual);
  
  return earth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void earth_destroy (earth_t *earth)
{
  CX_ASSERT (earth);
  
  // destroy SPHERES
  
  cx_mesh_destroy (earth->visual->mesh [0]);
  cx_mesh_destroy (earth->visual->mesh [1]);
  cx_mesh_destroy (earth->visual->mesh [2]);
  
  cx_free (earth);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool earth_init (const char *filename, const cx_date *date)
{
  g_earth = earth_create (filename, date);
  
  return g_earth ? true : false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_deinit (void)
{
  earth_destroy (g_earth);
  
  g_earth = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_render (const cx_vec4 *eye, const cx_date *date)
{
  earth_visual_render (eye, date);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int earth_data_get_count (void)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  
  int c = g_earth->data->count;
  
  return c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char **earth_data_get_names (void)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  
  const char **n = g_earth->data->names;
  
  return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *earth_data_get_weather (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
#if OPTIMIZED_WEATHER_DATA
  const char *w = &g_earth->data->weatherId [index * WEATHER_ID_MAX_LEN];
#else
  const char *w = g_earth->data->weatherId [index];
#endif
  
  return w;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *earth_data_get_city (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
  const char *c = g_earth->data->names [index];
  
  return c;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *earth_data_get_feed_query (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
  const char *q = g_earth->data->newsFeeds [index];
  
  return q;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_vec4 *earth_data_get_position (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
  const cx_vec4 *p = &g_earth->data->location [index];
  
  return p;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_vec4 *earth_data_get_normal (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
  const cx_vec4 *n = &g_earth->data->normal [index];
  
  return n;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int earth_data_get_tz_offset (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  
  int u = g_earth->data->utcOffset [index];
  int d = g_earth->data->dstOffset [index];
  int o = u + d;
  
  return o;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void earth_data_update_dst_offsets (void)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  
  for (int i = 0, c = g_earth->data->count; i < c; ++i)
  {
    const char *tzn = g_earth->data->tznames [i];
  
    int dstOffsetSecs = util_get_dst_offset_secs (tzn);
    
    g_earth->data->dstOffset [i] = dstOffsetSecs;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool earth_data_validate_index (int index)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  
  return (index > -1) && (index < g_earth->data->count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
