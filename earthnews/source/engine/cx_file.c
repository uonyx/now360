//
//  cx_file.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 02/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include "cx_file.h"

#include <fcntl.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FILE_DEBUG_LOG 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

extern void cx_native_get_filepath_from_resource (const char *filename, char *destFilepath, int destFilePathSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_file_load (cx_file *file, const char *filename)
{
  BOOL success = FALSE;
  
  char fullFilePath [512];
  cx_native_get_filepath_from_resource (filename, fullFilePath, 512);
  
  file->size = 0;
  file->data = NULL;
  file->fileHandle = fopen (fullFilePath, "rb");
  
  if (file->fileHandle)
  {
    // get file size
    fseek (file->fileHandle, 0L, SEEK_END);
    
    file->size = (cxu32) ftell (file->fileHandle);
    
    fseek (file->fileHandle, 0L, SEEK_SET);
    
    // get data
    file->data = (char *) cx_malloc (sizeof(char) * file->size);
  
    if (file->data)
    {
      size_t bytesRead = fread (file->data, sizeof(char), file->size, file->fileHandle);
      CX_ASSERT (bytesRead == file->size);
      CX_REFERENCE_UNUSED_VARIABLE (bytesRead);
      success = TRUE;
    }
    else
    {
      CX_OUTPUTLOG_CONSOLE (CX_FILE_DEBUG_LOG, "cx_file_load: failed to malloc %d bytes", (file->size * sizeof(char)));
    }
  }
  else
  {
    CX_OUTPUTLOG_CONSOLE (CX_FILE_DEBUG_LOG, "cx_file_load: failed to open file [%s]", fullFilePath);
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_file_unload (cx_file *file)
{
  BOOL success = FALSE;
  
  int err = fclose (file->fileHandle);
  
  if (err == 0)
  {
    cx_free (file->data);
    
    file->size = 0;
    file->data = NULL;
    file->fileHandle = NULL;
    
    success = TRUE;
  }
  else
  {
    CX_OUTPUTLOG_CONSOLE (CX_FILE_DEBUG_LOG, "cx_file_unload: failed to close file");
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 cx_file_getsize (const char *filename)
{
  cxu32 size = 0;
  
  char fullFilePath [512];
  cx_native_get_filepath_from_resource (filename, fullFilePath, 512);
  
  FILE *fp = fopen (fullFilePath, "rb");
  
  if (fp)
  {
    // get file size
    fseek (fp, 0L, SEEK_END);
    
    size = (cxu32) ftell (fp);
    
    fseek (fp, 0L, SEEK_SET);
    
    fclose (fp);
  }
  else
  {
    CX_OUTPUTLOG_CONSOLE (CX_FILE_DEBUG_LOG, "cx_file_getsize: failed to open file [%s]", fullFilePath);
  }
  
  return size;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_file_getexists (const char *filename)
{
  FILE *fp = fopen (filename, "rb");
  
  if (fp)
  {
    fclose (fp);
    return TRUE;
  }
  else
  {
    return FALSE;
  }

  /*
  int fd = open (filename, O_RDONLY, 0);
  
  if (fd == -1)
  {
    return FALSE;
  }
  else
  {
    //close (fd);
    return TRUE;
  }
  */
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
