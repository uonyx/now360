//
//  cx_file.h
//  earthnews
//
//  Created by Ubaka  Onyechi on 02/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#ifndef CX_FILE_H
#define CX_FILE_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  CX_FILE_MODE_READ   = 1,
  CX_FILE_MODE_WRITE  = 1 << 1,
  CX_FILE_MODE_APPEND = 1 << 2
} cx_file_mode;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_file
{
  FILE *fileHandle;
  char *data;
  cxu32 size;
} cx_file;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_file_load (cx_file *file, const char *filename);
BOOL cx_file_unload (cx_file *file);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_file_getsize (const char *filename);
BOOL cx_file_getexists (const char *filename);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
