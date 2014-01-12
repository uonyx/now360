//
//  cx_native_ios.h
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
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

void cx_native_file_get_resource_path (char *dstPath, cxu32 dstSize);

void cx_native_file_get_cache_path (char *dstPath, cxu32 dstSize);

void cx_native_file_get_documents_path (char *dstPath, cxu32 dstSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_native_eagl_context_init (void *eaglContext);

void cx_native_eagl_context_deinit (void);

void cx_native_eagl_context_add (void);

void cx_native_eagl_context_remove (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
