//
//  cx_native_ios.h
//
//  Created by Ubaka Onyechi on 31/01/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#ifndef CX_NATIVE_IOS_H
#define CX_NATIVE_IOS_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_file_get_path_from_resource (const char *filename, char *destFilepath, int destFilePathSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_init (void *eaglContext);

void cx_native_eagl_context_add (void);

void cx_native_eagl_context_remove (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif