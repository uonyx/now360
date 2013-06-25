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

#include "../system/cx_system.h"
#include "../system/cx_file.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_TEXTURE_DEBUG              (CX_DEBUG && 1)
#define CX_TEXTURE_DEBUG_LOG_ENABLE   (CX_DEBUG && 1)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_TEXTURE_MAX_MIPMAP_COUNT        (13)

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
  CX_TEXTURE_FORMAT_ALPHA,
  CX_TEXTURE_FORMAT_LUMINANCE,
  CX_TEXTURE_FORMAT_LUMINANCE_ALPHA,
  CX_TEXTURE_FORMAT_RGB,
  CX_TEXTURE_FORMAT_RGBA,
  CX_TEXTURE_FORMAT_RGB_PVR_4BPP,
  CX_TEXTURE_FORMAT_RGB_PVR_2BPP,
  CX_TEXTURE_FORMAT_RGBA_PVR_4BPP,
  CX_TEXTURE_FORMAT_RGBA_PVR_2BPP,
  CX_TEXTURE_NUM_FORMATS
} cx_texture_format;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_texture
{
  cxu32 id;
  
  cxu8 *data;
  cxu32 dataSize;
  
  cxu8 *imageData [CX_TEXTURE_MAX_MIPMAP_COUNT];
  cxu32 imageDataSize [CX_TEXTURE_MAX_MIPMAP_COUNT];
  
  cxu32 width;
  cxu32 height;
  
  cxu32 mipmapCount;
  
  cx_texture_format format;
  
  cxu32 compressed : 1;
  cxu32 npot : 1;
  
} cx_texture;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create (cxu32 width, cxu32 height, cx_texture_format format);
cx_texture *cx_texture_create_from_file (const char *filename, cx_file_storage_base storage, bool genMipmaps);

void cx_texture_destroy (cx_texture *texture);
void cx_texture_data_destroy (cx_texture *texture);
void cx_texture_set_wrap_mode (cx_texture *texture, cx_texture_wrap_mode mode);

void cx_texture_gpu_init (cx_texture *texture, bool genMipmaps);
void cx_texture_gpu_deinit (cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
