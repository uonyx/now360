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
#define DEBUG_EQUINOX_LIGHT (CX_DEBUG && 0)
#define DEBUG_EQUINOX_LIGHT_ORBIT (DEBUG_EQUINOX_LIGHT && 0)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WEATHER_ID_MAX_LEN 16

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct earth_visual_t
{
  float radius;
  int slices;
  
  bool animClouds;
  bool highSpec;
  
  cx_mesh *mesh [3];
  cx_texture *nightMap;
};

struct earth_data_t
{
  // city properties
  const char **names;
  const char **newsFeeds;
  const char **tznames;
  char *weatherId;
  int *utcOffset;
  int *dstOffset;
  float *longitude;
  float *latitude;
  cx_vec4 *location;
  cx_vec4 *normal;
  
  // number of cities
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

static void earth_convert_dd_to_world (cx_vec4 *world, cx_vec4 *n, float latitude, float longitude, float radius, int slices)
{
  // convert lat/long to texture paramter-space (uv coords)
  
  float tx = (longitude + 180.0f) / 360.0f;
  float ty = 1.0f - ((latitude + 90.0f) / 180.0f);
  
  // convert texture coords to sphere slices/parallels coords
  
  float i = ty * (float) (slices >> 1); // parallels
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
#if !DEBUG_EQUINOX_LIGHT
static float earth_visual_dump_multiples (float mul, float div)
{
  cxf32 val = mul;
  
  val -= div * (int) (val / div);
  val += (val < 0.0f) ? div : 0.0f;
  
  return val;
}
#endif
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
      earthdata->weatherId = (char *) cx_malloc (sizeof (char) * WEATHER_ID_MAX_LEN * count);
      earthdata->utcOffset = (int *) cx_malloc (sizeof (int) * count);
      earthdata->dstOffset = (int *) cx_malloc (sizeof (int) * count);
      earthdata->tznames = (const char **) cx_malloc (sizeof (char *) * count);
      earthdata->longitude = (float *) cx_malloc (sizeof (float) * count);
      earthdata->latitude = (float *) cx_malloc (sizeof (float) * count);
      
      memset (earthdata->location, 0, (sizeof (cx_vec4) * count));
      memset (earthdata->normal, 0, (sizeof (cx_vec4) * count));
      memset (earthdata->names, 0, (sizeof (char *) * count));
      memset (earthdata->newsFeeds, 0, (sizeof (char *) * count));
      memset (earthdata->weatherId, 0, sizeof (char) * WEATHER_ID_MAX_LEN * count);
      memset (earthdata->utcOffset, 0, (sizeof (int) * count));
      memset (earthdata->dstOffset, 0, (sizeof (int) * count));
      memset (earthdata->tznames, 0, (sizeof (char *) * count));
      memset (earthdata->longitude, 0, (sizeof (float) * count));
      memset (earthdata->latitude, 0, (sizeof (float) * count));
      
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
            int loc = i * WEATHER_ID_MAX_LEN;
            char *d = &earthdata->weatherId [loc];
            cx_strcpy (d, WEATHER_ID_MAX_LEN, str);
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
            
            earthdata->latitude [i] = lat;
            earthdata->longitude [i] = lon;
            
            float r = radius + (radius * 0.025f); // slightly extend radius (for point sprite rendering)
            
            earth_convert_dd_to_world (&earthdata->location [i], &earthdata->normal [i], lat, lon, r, slices);
          }
        }
      }
      
      cx_json_tree_destroy (jsonTree);
    }
    else
    {
      CX_LOG_CONSOLE (1, "JSON parse error: %s", filename);
    }
    
    cx_free (filedata);
  }
  else
  {
    CX_LOG_CONSOLE (1, "Failed to load %s", filename);
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
  
  char diffTexPath [64];
  const char *earthShader = NULL;
  const char *cloudShader = NULL;
  const char *specTexPath = NULL;
  const char *bumpTexPath = NULL;
  const char *cloudTexPath = NULL;
  const char *nightTexPath = NULL;
  
  bool highSpec = false;
  bool animClouds = false;

  device_type_t devType = util_get_device_type ();
  CX_REF_UNUSED (devType);

  switch (devType)
  {
    case DEVICE_TYPE_UNKNOWN:
    case DEVICE_TYPE_IPAD3:
    {
      earthShader   = "earth-hi";     highSpec = true;
      cloudShader   = "clouds-anim";  animClouds = true;
      specTexPath   = "data/images/earth/maps/spec1-2048.png";
      bumpTexPath   = "data/images/earth/maps/norm-sobel3x3-4096.png";
      cloudTexPath  = "data/images/earth/maps/clouds-4096.png";
      nightTexPath  = "data/images/earth/maps/night1-4096.png";
      cx_sprintf (diffTexPath, 64, "data/images/earth/maps/diff-%02d-4096.png", month);
      break;
    }
  
    case DEVICE_TYPE_IPAD2:
    {
#if 0 // test hi
      earthShader   = "earth-hi";     highSpec = true;
      cloudShader   = "clouds-anim";  animClouds = true;
      specTexPath   = "data/images/earth/maps/spec1-2048.png";
      bumpTexPath   = "data/images/earth/maps/norm-sobel3x3-4096.png";
      cloudTexPath  = "data/images/earth/maps/clouds-4096.png";
      nightTexPath  = "data/images/earth/maps/night1-4096.png";
      cx_sprintf (diffTexPath, 64, "data/images/earth/maps/diff-%02d-4096.png", month);
#elif 0 // test mid
        //earthShader   = "earth-hi";     highSpec = true;
      earthShader   = "earth-lo"; highSpec = false;
      cloudShader   = "clouds-anim";  animClouds = true;
      specTexPath   = "data/images/earth/maps/spec1-2048.png";
      bumpTexPath   = "data/images/earth/maps/norm-sobel3x3-2048.png";
      cloudTexPath  = "data/images/earth/maps/clouds-2048.png";
      nightTexPath  = "data/images/earth/maps/night1-4096.png";
      cx_sprintf (diffTexPath, 64, "data/images/earth/maps/diff-%02d-4096.png", month);
#else // test lo
      earthShader   = "earth-lo"; highSpec = false;
      cloudShader   = "clouds";   animClouds = false;
      specTexPath   = "data/images/earth/maps/spec1-1024.jpg";
      bumpTexPath   = "data/images/earth/maps/norm-sobel3x3-2048.png";
      cloudTexPath  = "data/images/earth/maps/clouds-2048.png";
      nightTexPath  = "data/images/earth/maps/night1-2048.jpg";
      cx_sprintf (diffTexPath, 64, "data/images/earth/maps/diff-%02d-2048.jpg", 6);
#endif
      break;
    }
      
    case DEVICE_TYPE_IPAD1:
    default:
    {
      earthShader   = "earth-lo"; highSpec = false;
      cloudShader   = "clouds";   animClouds = false;
      specTexPath   = "data/images/earth/maps/spec1-1024.jpg";
      bumpTexPath   = "data/images/earth/maps/norm-sobel3x3-2048.png";
      cloudTexPath  = "data/images/earth/maps/clouds-2048.png";
      nightTexPath  = "data/images/earth/maps/night1-2048.jpg";
      cx_sprintf (diffTexPath, 64, "data/images/earth/maps/diff-%02d-2048.jpg", 6);
      break;
    }
  }
  
  ////////////////////////////////////////////////////////////////////////////////////////////////////////

  struct earth_visual_t *visual = (struct earth_visual_t *) cx_malloc (sizeof (struct earth_visual_t));
  
  visual->slices = slices;
  visual->radius = radius;
  visual->animClouds = animClouds;
  visual->highSpec = highSpec;
  
#if NEW_EARTH_SHADER
  
#if 0
  cx_texture *specTexture   = cx_texture_create_from_file (specTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *cloudTexture  = cx_texture_create_from_file (cloudTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *bumpTexture   = cx_texture_create_from_file (bumpTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *nightTexture  = cx_texture_create_from_file (nightTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *diffTexture   = cx_texture_create_from_file (diffTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
#else // hi
  cx_texture *diffTexture   = cx_texture_create_from_file (diffTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *specTexture   = cx_texture_create_from_file (specTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  
  cx_texture *cloudTexture  = cx_texture_create_from_file (cloudTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *nightTexture  = cx_texture_create_from_file (nightTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_texture *bumpTexture   = cx_texture_create_from_file (bumpTexPath, CX_FILE_STORAGE_BASE_RESOURCE, true);
#endif
  
  CX_ASSERT (specTexture);
  CX_ASSERT (bumpTexture);
  CX_ASSERT (cloudTexture);
  CX_ASSERT (nightTexture);
  CX_ASSERT (diffTexture);
  
  cx_shader *shader         = cx_shader_create (earthShader, "data/shaders");
  cx_material *material     = cx_material_create (earthShader);
  
  cx_material_set_texture (material, diffTexture, CX_MATERIAL_TEXTURE_DIFFUSE);
  cx_material_set_texture (material, specTexture, CX_MATERIAL_TEXTURE_SPECULAR);
  cx_material_set_texture (material, bumpTexture, CX_MATERIAL_TEXTURE_BUMP);
  
  visual->nightMap = nightTexture;
  
  CX_REF_UNUSED (cloudTexture);
  
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
  
#if NEW_EARTH_SHADER && ENABLE_CLOUDS

  float radius1 = radius + 0.008f;
  
#if BUMP_MAPPED_CLOUDS
  cx_shader *shader1     = cx_shader_create ("clouds-bump", "data/shaders");
  cx_material *material1 = cx_material_create ("clouds-bump");
  cx_material_set_texture (material1, cloudTexture, CX_MATERIAL_TEXTURE_DIFFUSE);
  
  cx_texture *cloudBump = cx_texture_create_from_file ("data/maps/2048-normal-clouds.png", CX_FILE_STORAGE_BASE_RESOURCE, true);
  cx_material_set_texture (material1, cloudBump, CX_MATERIAL_TEXTURE_BUMP);
  
  cx_vertex_data *sphere1 = cx_vertex_data_create_sphere (radius1, 36, CX_VERTEX_FORMAT_PTNTB);
#else
  cx_shader *shader1     = cx_shader_create (cloudShader, "data/shaders");
  cx_material *material1 = cx_material_create (cloudShader);
  
  cx_material_set_texture (material1, cloudTexture, CX_MATERIAL_TEXTURE_DIFFUSE);
  cx_vertex_data *sphere1 = cx_vertex_data_create_sphere (radius1, 36, CX_VERTEX_FORMAT_PTN);
#endif
  
  visual->mesh [1] = cx_mesh_create (sphere1, shader1, material1);
#endif
  
  //////////////////////////////////////////////////////////////////////////////////////////
  
#if NEW_EARTH_SHADER && ENABLE_ATMOSPHERE
  float radius2 = radius + 0.010f;
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
#if !DEBUG_EQUINOX_LIGHT
static void earth_visual_get_sun_position (cx_vec4 *position, const cx_date *date)
{
  CX_ASSERT (position);
  CX_ASSERT (date);
  
  // http://en.wikipedia.org/wiki/Position_of_the_Sun
  
  ////////////////////////////////////////
  // ecliptic coordinates
  ////////////////////////////////////////
  
  // calculate julian date
  
  int sec = date->calendar.tm_sec;
  int min = date->calendar.tm_min;
  int hour = date->calendar.tm_hour;
  int day = date->calendar.tm_mday;
  int month = date->calendar.tm_mon + 1;
  int year = 1900 + date->calendar.tm_year;
  
  float s = (float) ((hour * 3600) + (min * 60) + sec);
  float t = s / 86400.0f;                               // range (0, 1)
  float jdnOffset = ((t * 2.0f) - 1.0f) * 0.5f;         // range (-0.5, 0.5)
  
  int a = (14 - month) / 12;
  int y = year + 4800 - a;
  int m = month + (12 * a) - 3;
  
  float jdn = (float) (day + (((153 * m) + 2) / 5) + (365 * y) + (y / 4) - (y / 100) + (y / 400) - 32045);
  
  // n: number of days since J2000.0
  float n = (jdn + jdnOffset) - 2451545.0f;
  
  // L: mean longitude of the sun
  float L = 280.460f + (0.9856474f * n);
  L = earth_visual_dump_multiples (L, 360.0f);
  
  // g: mean anomaly of the sun
  float g = 357.528f + (0.9856003f * n);
  g = earth_visual_dump_multiples (g, 360.0f);
  float gRad = cx_rad (g);
  
  // lambda: ecliptic longitude
  float ecLong = L + (1.915f * cx_sin (gRad)) + (0.02f * cx_sin (2.0f * gRad));
  ecLong = earth_visual_dump_multiples (ecLong, 360.0f);
  float ecLongRad = cx_rad (ecLong);
  
  // beta: ecliptic latitude
  float ecLat = 0.0f;
  CX_REF_UNUSED (ecLat);
  
  // R: distance of the sun from the earth
  float R = 1.00014f - (0.01671f * cx_cos (gRad)) - (0.00014f * cx_cos (2.0f * gRad));
  CX_REF_UNUSED (R);
  
  ////////////////////////////////////////
  // equatorial coordinates
  ////////////////////////////////////////
  
  // e: obliquity of the ecliptic
  float e = 23.439f - (0.0000004f * n);
  float eRad = cx_rad (e);
  
  // d: declination
  float d = cx_asin (cx_sin (eRad) * cx_sin (ecLongRad));
  float dDeg = cx_deg (d);
  
#if 1 // no hack
  // ra: right ascension
  float raNum = cx_cos (eRad) * cx_sin (ecLongRad);
  float raDen = cx_cos (ecLongRad);
  float ra = cx_atan2 (raNum, raDen);
  float raDeg = cx_deg (ra);
  if (raDeg < 0.0f) { raDeg += 360.0f; }

  // hours
  int tsecs = (hour * 3600) + (min * 60) + sec;
  float hrsf = (float) tsecs / 3600.0f;
  
  // gmst: greenwhich mean sidereal time
  float gmst = 6.697375f + (0.0657098242f * n) + hrsf;
  gmst = earth_visual_dump_multiples (gmst, 24.0f);
  
  float utclong = 0.0f; //-0.196306f;

  // lmst: local mean sidereal time
  float lmst = (gmst * 15.0f) + utclong;
  lmst = earth_visual_dump_multiples (lmst, 360.0f);
  
  // ha: hour angle
  float ha = lmst - raDeg;
  
  if (ha < -180.0f)
  {
    ha += 360.0f;
  }
  else if (ha > 180.0f)
  {
    ha -= 360.0f;
  }
  
  // sun-earth distance
#if 0
  float earthRadiusAu = 4.26349651e-5f;
  float sunEarthDist  = R / earthRadiusAu;
#else
  float sunEarthDist = 250.0f;
#endif
  
  earth_convert_dd_to_world (position, NULL, dDeg, ha * -1.0f, sunEarthDist, g_earth->visual->slices);
  
#else
  
  float sunEarthDist = 250.0f;
  int hr = date->calendar.tm_hour;
  int mn = date->calendar.tm_min;
  int sc = date->calendar.tm_sec;
  int seconds = ((hr * 60) + mn) * 60 + sc;
  int secondsInDay = 86400;
  float angleDeg = ((float) seconds / (float) secondsInDay) * 360.0f; // 0 - 360
  angleDeg -= 180.0f;
  angleDeg *= -1.0f;
  
  earth_convert_dd_to_world (position, NULL, dDeg, angleDeg, sunEarthDist, g_earth->visual->slices);
#endif
}
#endif
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
  
#if DEBUG_EQUINOX_LIGHT
#if DEBUG_EQUINOX_LIGHT_ORBIT
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
  angle *= -1.0f;
#endif
  
  cx_vec4_set (&lightPos, 0.0f, 0.0f, -4.0f, 1.0f);

  // get rotation matrix
  cx_mat4x4 rotation;
  cx_mat4x4_rotation (&rotation, cx_rad (angle), 0.0f, 1.0f, 0.0f);
  
  cx_vec4 origin;
  cx_vec4_set (&origin, 0.0f, 0.0f, 0.0f, 1.0f);
  
  cx_vec4 forward;
  cx_vec4_sub (&forward, &lightPos, &origin);
  
  // transform forward vector
  cx_vec4 newForward;
  cx_mat4x4_mul_vec4 (&newForward, &rotation, &forward);
  
  // update light position
  cx_vec4_add (&lightPos, &origin, &newForward);

#else

  earth_visual_get_sun_position (&lightPos, date);
  
#endif
  
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
    
    cx_colour_set (&ambient, 0.15f, 0.15f, 0.15f, 1.0f);
    cx_colour_set (&diffuse, 1.0f, 1.0f, 1.0f, 1.0f);
    cx_colour_set (&specular, 1.0f, 0.98f, 0.803f, 1.0f);
    
    cx_shader_set_vector4 (mesh->shader, "u_ambientLight", &ambient, 1);
    cx_shader_set_vector4 (mesh->shader, "u_diffuseLight", &diffuse, 1);
    
    if (g_earth->visual->highSpec)
    {
      shininess = 3.5f;
      
      cx_shader_set_vector4 (mesh->shader, "u_specularLight", &specular, 1);
      cx_shader_set_float (mesh->shader, "u_shininess", &shininess, 1);
    }
    
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
    
#else
    
    if (g_earth->visual->animClouds)
    {
      static float angle = 0.0f;
      float dt = (float) cx_system_time_get_delta_time () * 0.125f; // (1.0f / 120.0f);
#if 1
      angle += dt;
      angle = fmodf (angle, 360.0f);
#else
      angle -= dt;
      angle = earth_visual_dump_multiples (angle, 360.0f);
#endif
    
      cx_mat4x4 r; // model-view matrix
      cx_mat4x4_rotation_axis_y (&r, cx_rad (angle));
    
      cx_mat4x4 mvp; // new mvp matrix
      cx_mat4x4_mul (&mvp, &mvpMatrix, &r);
      
      cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_TRANSFORM_MV, &r);
      cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvp);
    }
    else
    {
      cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_TRANSFORM_MVP, &mvpMatrix);
    }
    
#endif
    
    cx_shader_set_uniform (mesh1->shader, CX_SHADER_UNIFORM_LIGHT_POSITION, &lightPos);
    
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
  
  const char *w = &g_earth->data->weatherId [index * WEATHER_ID_MAX_LEN];
  
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

void earth_data_get_terrestrial_coords (int index, float *lat, float *lon)
{
  CX_ASSERT (g_earth);
  CX_ASSERT (g_earth->data);
  CX_ASSERT (earth_data_validate_index (index));
  CX_ASSERT (lat);
  CX_ASSERT (lon);
  
  *lat = g_earth->data->latitude [index];
  *lon = g_earth->data->longitude [index];
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
