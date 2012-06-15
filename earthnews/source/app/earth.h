//
//  earth.h
//  earthnews
//
//  Created by Ubaka Onyechi on 01/05/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef EARTHNEWS_CITIES_H
#define EARTHNEWS_CITIES_H

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct earth_visual_t
{
  float radius;
  int slices;
  int parallels;
  
  cx_mesh *mesh;
};

struct earth_data_t
{
  const char **names;
  const char **newsFeeds;
  const char **weatherFeeds;
  cx_vec4 *location;
  cx_vec4 *normal;
  unsigned int count;
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

earth_t *earth_create (const char *filename, float radius, int slices, int parallels);
void earth_destroy (earth_t *earth);
void earth_render (earth_t *earth);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
