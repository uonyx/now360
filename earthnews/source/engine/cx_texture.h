//
//  cx_texture.h
//  earthnews
//
//  Created by Ubaka  Onyechi on 19/02/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
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
#define CX_TEXTURE_DEBUG_LOG  (CX_DEBUG && 1)

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
  CX_TEXTURE_FORMAT_INVALID,
  CX_TEXTURE_FORMAT_RGB,
  CX_TEXTURE_FORMAT_RGBA,
  CX_TEXTURE_FORMAT_ALPHA,
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
  cxi32 bpp;
  cxi32 format;
  
  bool iscompressed;
  
#if CX_TEXTURE_DEBUG
  char *filename;
#endif
} cx_texture;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create (const char *filename);
void cx_texture_destroy (cx_texture *texture);

void cx_texture_set_wrap_mode (cx_texture *texture, cx_texture_wrap_mode mode);
void cx_texture_db_clean_up (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
