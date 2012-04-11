//
//  cx_engine.h
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_ENGINE_H
#define CX_ENGINE_H

#include "cx_system.h"
#include "cx_graphics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_engine_init (void)
{
  cx_system_init ();
  cx_graphics_init ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static CX_INLINE void cx_engine_deinit (void)
{
  cx_graphics_deinit ();
  cx_system_deinit ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif