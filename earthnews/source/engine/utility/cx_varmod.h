//
//  cx_varmod.h
//
//  Created by Ubaka Onyechi on 24/11/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_VARMOD_H
#define CX_VARMOD_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../system/cx_system.h"
#include "../system/cx_vector4.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*cx_varmod_render_func) (void);

typedef struct cx_varmod_settings
{
  cx_varmod_render_func renderFunc;
  
} cx_varmod_settings;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_varmod_init (cx_varmod_settings *settings);
bool cx_varmod_deinit (void);

void cx_varmod_register_bool (bool *val, const char *name);
void cx_varmod_register_float (cxf32 *val, const char *name, cxf32 min, cxf32 max);
void cx_varmod_register_int (cxi32 *val, const char *name, cxi32 min, cxi32 max);
void cx_varmod_register_vec4 (cx_vec4 *val, const char *name, cxf32 min, cxf32 max);

void cx_varmod_render (void);
void cx_varmod_input (cxf32 touchx, cxf32 touchy);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif