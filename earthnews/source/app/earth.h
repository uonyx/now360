//
//  earth.h
//  now360
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef NOW360_EARTH_H
#define NOW360_EARTH_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool        earth_init (const char *filename, const cx_date *date);
void        earth_deinit (void);
void        earth_render (const cx_vec4 *eye, const cx_date *date);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

int             earth_data_get_count (void);
const char **   earth_data_get_names (void);
const char *    earth_data_get_city (int index);
const char *    earth_data_get_weather (int index);
const char *    earth_data_get_feed_query (int index);
const cx_vec4 * earth_data_get_position (int index);
const cx_vec4 * earth_data_get_normal (int index);
int             earth_data_get_tz_offset (int index);
void            earth_data_get_terrestrial_coords (int index, float *lat, float *lon);
cx_vertex_data *earth_data_get_mesh_vertex_data (int meshIndex);
void            earth_data_update_dst_offsets (void);
bool            earth_data_validate_index (int index);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
