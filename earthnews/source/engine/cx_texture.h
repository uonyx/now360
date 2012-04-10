//
//  cx_texture.h
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_TEXTURE_H
#define CX_TEXTURE_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_TEXTURE_DEBUG      (CX_DEBUG && 1)
#define CX_TEXTURE_DEBUG_LOG_ENABLE  (CX_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_texture_filter
{
  CX_TEXTURE_FILTER_0,  // nearest
  CX_TEXTURE_FILTER_1,  // linear
  CX_TEXTURE_FILTER_2,  // bilinear
  CX_TEXTURE_FILTER_3,  // trilinear - requires mipmaps
} cx_texture_filter;

typedef enum cx_texture_wrap_mode
{
  CX_TEXTURE_WRAP_MODE_CLAMP,
  CX_TEXTURE_WRAP_MODE_REPEAT,
} cx_texture_wrap_mode;

typedef enum cx_texture_format
{
  CX_TEXTURE_FORMAT_INVALID = -1,
  CX_TEXTURE_FORMAT_RGB,
  CX_TEXTURE_FORMAT_RGBA,
  CX_TEXTURE_FORMAT_ALPHA,
  CX_TEXTURE_NUM_FORMATS
} cx_texture_format;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_texture
{
  cxu32 id;
  
  void *data;
  cxi32 dataSize;
  
  cxi32 width;
  cxi32 height;
  
  cx_texture_format format;
  
  bool compressed;
  
} cx_texture;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create (cxu32 width, cxu32 height, cx_texture_format format);
cx_texture *cx_texture_create_from_file (const char *filename);

void cx_texture_destroy (cx_texture *texture);
void cx_texture_set_wrap_mode (cx_texture *texture, cx_texture_wrap_mode mode);

void cx_texture_gpu_init (cx_texture *texture);
void cx_texture_gpu_deinit (cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
