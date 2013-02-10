//
//  cx_engine.h
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_ENGINE_H
#define CX_ENGINE_H

#include "system/cx_system.h"
#include "system/cx_time.h"
#include "system/cx_file.h"
#include "system/cx_list.h"
#include "system/cx_thread.h"
#include "system/cx_xml.h"
#include "system/cx_utility.h"

#include "graphics/cx_gdi.h"
#include "graphics/cx_font.h"
#include "graphics/cx_mesh.h"
#include "graphics/cx_draw.h"

#include "network/cx_http.h"

#include "utility/cx_varmod.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_ENGINE_INIT_GRAPHICS   0x1
#define CX_ENGINE_INIT_NETWORK    0x2
#define CX_ENGINE_INIT_ALL        (CX_ENGINE_INIT_GRAPHICS | CX_ENGINE_INIT_NETWORK)

typedef cxu32 cx_engine_init_flags;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_engine_init_params
{
  cxi32 screenWidth;
  cxi32 screenHeight;
  void *graphicsContext;
} cx_engine_init_params;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_engine_init (cx_engine_init_flags flags, cx_engine_init_params *params)
{
  CX_ASSERT (params);
  
  _cx_system_init ();
  
  if ((flags & CX_ENGINE_INIT_GRAPHICS) == CX_ENGINE_INIT_GRAPHICS)
  {
    CX_ASSERT (params);
    
    _cx_shader_init ();
    _cx_gdi_init (params->graphicsContext, params->screenWidth, params->screenHeight);
  }
  
  if ((flags & CX_ENGINE_INIT_NETWORK) == CX_ENGINE_INIT_NETWORK)
  {
    _cx_http_init ();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_engine_deinit (void)
{
  // graphics
  _cx_shader_deinit ();
  _cx_gdi_deinit ();
  
  // network
  _cx_http_deinit ();
  
  // system
  _cx_system_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
