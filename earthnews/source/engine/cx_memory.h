//
//  cx_memory.h
//  earthnews
//
//  Created by Ubaka  Onyechi on 14/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#ifndef CX_MEMORY_H
#define CX_MEMORY_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_defines.h"
#include "cx_debug.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define cx_malloc(X)  malloc(X)
#define cx_free(X)    free(X)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_memory_initialise (void);
void cx_memory_deinitialise (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void *_cx_malloc (size_t size);
void _cx_free (void *data);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
