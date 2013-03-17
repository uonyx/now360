//
//  cx_file.h
//
//  Created by Ubaka Onyechi on 02/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_FILE_H
#define CX_FILE_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"
#include "../3rdparty/json-parser/json.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FILENAME_MAX 256

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define	CX_FILE_SEEK_OFFSET_SET SEEK_SET
#define	CX_FILE_SEEK_OFFSET_CUR SEEK_CUR
#define	CX_FILE_SEEK_OFFSET_END SEEK_END

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  CX_FILE_MODE_READ,
  CX_FILE_MODE_WRITE,
  CX_FILE_MODE_APPEND,
  CX_FILE_NUM_MODES,
} cx_file_mode;

typedef enum
{
  CX_FILE_STORAGE_BASE_RESOURCE,
  CX_FILE_STORAGE_BASE_DOCUMENTS,
  CX_FILE_STORAGE_BASE_CACHE,
  CX_FILE_STORAGE_BASE_ROOT,
} cx_file_storage_base;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef FILE *cx_file;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool  cx_file_open (cx_file *file, const char *filename, cxu32 mode);
bool  cx_file_close (cx_file *file);
cxu32 cx_file_read (cx_file *file, void *buffer, cxu32 size, cxu32 count);
cxu32 cx_file_write (cx_file *file, const void *buffer, cxu32 size, cxu32 count);
cxi32 cx_file_flush (cx_file *file);
cxi32 cx_file_seek (cx_file *file, cxi32 offset, cxi32 origin);
cxu32 cx_file_size (cx_file *file);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_file_storage_path (char *dstpath, int dstpathSize, const char *filename, cx_file_storage_base base);
bool cx_file_storage_exists (const char *filename, cx_file_storage_base base);
bool cx_file_storage_load_contents (cxu8 **buffer, cxu32 *size, const char *filename, cx_file_storage_base base);
bool cx_file_storage_save_contents (const cxu8 *buffer, cxu32 size, const char *filename, cx_file_storage_base base);
bool cx_file_storage_create_dir (const char *dirname, cx_file_storage_base base);
bool cx_file_storage_delete (const char *dirname, cx_file_storage_base base);                                 
bool cx_file_storage_copy (const char *tofilename, cx_file_storage_base tobase, 
                           const char *frmfilename, cx_file_storage_base frmbase);
bool cx_file_storage_move (const char *tofilename, cx_file_storage_base tobase, 
                           const char *frmfilename, cx_file_storage_base frmbase);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
