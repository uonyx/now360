//
//  cx_texture.
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "../system/cx_utility.h"
#include "../system/cx_string.h"

#include "cx_texture.h"
#include "cx_gdi.h"

#include "../3rdparty/stb/stb_image.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cxu8 g_texture_format_pixel_size [CX_TEXTURE_NUM_FORMATS] =
{
  1, /* CX_TEXTURE_FORMAT_ALPHA */
  1, /* CX_TEXTURE_FORMAT_LUMINANCE */
  2, /* CX_TEXTURE_FORMAT_LUMINANCE_ALPHA */
  3, /* CX_TEXTURE_FORMAT_RGB */
  4, /* CX_TEXTURE_FORMAT_RGBA */
  4, /* CX_TEXTURE_FORMAT_RGBA_PVR_4BPP */
  2, /* CX_TEXTURE_FORMAT_RGBA_PVR_2BPP */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_texture_node 
{
  cx_texture texture;
  const char *filename;
  struct cx_texture_node *next;
} cx_texture_node;


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_texture *cx_texture_load_img_pvr (const char *filename, cx_file_storage_base storage);
static cx_texture *cx_texture_load_img_xxx (const char *filename, cx_file_storage_base storage);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_texture *cx_texture_load_img_xxx (const char *filename, cx_file_storage_base storage)
{
  cx_texture *texture = NULL;
  
  char path [512];
  cx_file_storage_path (path, 512, filename, storage);
  
  int w, h, ch;
  cxu8 *data = stbi_load (path, &w, &h, &ch, STBI_default);
  
  if (data)
  {
    texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
    memset (texture, 0, sizeof (cx_texture));
    
    switch (ch)
    {
      case STBI_grey:       { texture->format = CX_TEXTURE_FORMAT_LUMINANCE; break; }
      case STBI_grey_alpha: { texture->format = CX_TEXTURE_FORMAT_LUMINANCE_ALPHA; break; }
      case STBI_rgb:        { texture->format = CX_TEXTURE_FORMAT_RGB; break; }
      case STBI_rgb_alpha:  { texture->format = CX_TEXTURE_FORMAT_RGBA; break; }
      default:              { CX_FATAL_ERROR ("Invalid texture format"); break; }
    }
    
    texture->data = data;
    texture->dataSize = w * h * sizeof (cxu8) * g_texture_format_pixel_size [texture->format];
    texture->width = (cxu32) w;
    texture->height = (cxu32) h;
    texture->npot = (!cx_util_is_power_of_2 (texture->width)) || (!cx_util_is_power_of_2 (texture->height));
    
    texture->imageData [0] = texture->data;
    texture->imageDataSize [0] = texture->dataSize;
  }
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PVR_TEXTURE_FLAG_TYPE_MASK  0xff
static cx_texture *cx_texture_load_img_pvr (const char *filename, cx_file_storage_base storage)
{
  cx_texture *texture = NULL;

  cxu8 *data = NULL;
  cxu32 dataSize = 0;
  
  if (cx_file_storage_load_contents (&data, &dataSize, filename, storage))
  {    
    enum
    {
      kPVRTextureFlagTypePVRTC_2 = 24,
      kPVRTextureFlagTypePVRTC_4
    };
    
    typedef struct _PVRTexHeader
    {
      uint32_t headerLength;
      uint32_t height;
      uint32_t width;
      uint32_t numMipmaps;
      uint32_t flags;
      uint32_t dataLength;
      uint32_t bpp;
      uint32_t bitmaskRed;
      uint32_t bitmaskGreen;
      uint32_t bitmaskBlue;
      uint32_t bitmaskAlpha;
      uint32_t pvrTag;
      uint32_t numSurfs;
    } PVRTexHeader;
    
    char PVRTexIdentifier [4] = "PVR!";
    
    PVRTexHeader *header = (PVRTexHeader *) data;
    
    bool validPVR = true;
    
    if (PVRTexIdentifier[0] != (((header->pvrTag) >>  0) & 0xff) ||
        PVRTexIdentifier[1] != (((header->pvrTag) >>  8) & 0xff) ||
        PVRTexIdentifier[2] != (((header->pvrTag) >> 16) & 0xff) ||
        PVRTexIdentifier[3] != (((header->pvrTag) >> 24) & 0xff))
    {
      validPVR = false;
    }
    
    if (validPVR)
    {
      cxu32 formatFlags = (header->flags) & PVR_TEXTURE_FLAG_TYPE_MASK;
      CX_REF_UNUSED (formatFlags);
      CX_ASSERT ((formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2));
      
      texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
      memset (texture, 0, sizeof (cx_texture));
      
      texture->data       = data;
      texture->dataSize   = dataSize;
      texture->compressed = 1;
      texture->mipmapCount = (header->numMipmaps);
      texture->width = (header->width);
      texture->height = (header->height);
      texture->npot = (!cx_util_is_power_of_2 (texture->width)) || (!cx_util_is_power_of_2 (texture->height));
      texture->format = (formatFlags == kPVRTextureFlagTypePVRTC_4) ? CX_TEXTURE_FORMAT_RGBA_PVR_4BPP : CX_TEXTURE_FORMAT_RGBA_PVR_2BPP;
      
      bool hasAlpha = (header->bitmaskAlpha) > 0;
      CX_REF_UNUSED (hasAlpha);
      
      cxu8 *bytes = texture->data + sizeof (PVRTexHeader);
      
      cxu32 c = 0;
      cxu32 w = texture->width;
      cxu32 h = texture->height;

      cxu32 bpp = 4;
      cxu32 dataLength = 0, dataOffset = 0, dataSize = 0;
      cxu32 blockSize = 0, widthBlocks = 0, heightBlocks = 0;
      
      dataLength = (header->dataLength);
      
      // Calculate the data size for each texture level and respect the minimum number of blocks
      while (dataOffset < dataLength)
      {
        if (formatFlags == kPVRTextureFlagTypePVRTC_4)
        {
          blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
          widthBlocks = texture->width / 4;
          heightBlocks = texture->height / 4;
          bpp = 4;
        }
        else
        {
          blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
          widthBlocks = texture->width / 8;
          heightBlocks = texture->height / 4;
          bpp = 2;
        }
        
        widthBlocks = cx_max (widthBlocks, 2);
        heightBlocks = cx_max (heightBlocks, 2);
        
        dataSize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
      
        texture->imageData [c] = bytes + dataOffset;
        texture->imageDataSize [c++] = dataSize;
        
        dataOffset += dataSize;
        
        w = cx_max (w >> 1, 1);
        h = cx_max (h >> 1, 1);
      }
      
      CX_ASSERT (texture->mipmapCount == (c - 1));
    }
    else
    {
      cx_free (data);
    }
  }

  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create (cxu32 width, cxu32 height, cx_texture_format format)
{
  CX_ASSERT ((format > CX_TEXTURE_FORMAT_INVALID) && (format < CX_TEXTURE_NUM_FORMATS));
  
  cx_texture *texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
  memset (texture, 0, sizeof (cx_texture));

  texture->id                = 0;
  texture->width             = width;
  texture->height            = height;
  texture->format            = format;
  texture->compressed        = (format <= CX_TEXTURE_FORMAT_RGBA) ? 0 : 1;
  texture->dataSize          = width * height * sizeof (cxu8) * g_texture_format_pixel_size [format];
  texture->data              = (cxu8 *) cx_malloc (texture->dataSize);
  texture->imageData [0]     = texture->data;
  texture->imageDataSize [0] = texture->dataSize;
  texture->npot              = (!cx_util_is_power_of_2 (width)) || (!cx_util_is_power_of_2 (height));
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create_from_file (const char *filename, cx_file_storage_base storage)
{
  CX_ASSERT (filename);
  
  cx_texture *texture = cx_texture_load_img_xxx (filename, storage);
  
  if (texture == NULL)
  {
    texture = cx_texture_load_img_pvr (filename, storage);
  }
  
  if (texture)
  {
    cx_texture_gpu_init (texture);
    cx_texture_data_destroy (texture);
  }

  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE && !texture, "cx_texture_create: failed to load [%s]", filename);
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_data_destroy (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  if (texture->data)
  {
    cx_free (texture->data);
    texture->data = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_destroy (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  cx_texture_gpu_deinit (texture);
  cx_texture_data_destroy (texture);
  
  cx_free (texture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_gpu_init (cx_texture *texture)
{
  CX_ASSERT (texture);
  CX_ASSERT (texture->data);
  CX_ASSERT (texture->id == 0);
  CX_ASSERT (texture->width > 0);
  CX_ASSERT (texture->height > 0);
  
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->width  [%d]", texture->width);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->height [%d]", texture->height);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->format [%d]", texture->format);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->compressed [%s]", texture->compressed ? "true" : "false");
  
  glGenTextures (1, &texture->id);
  cx_gdi_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_gdi_assert_no_errors ();

  cxu32 imageCount = texture->mipmapCount + 1;
  CX_ASSERT (imageCount <= CX_TEXTURE_MAX_MIPMAP_COUNT);
  
  cxu32 w = texture->width;
  cxu32 h = texture->height;
  
  for (cxu32 i = 0; i < imageCount; ++i)
  {
    cxu8 *imageData = texture->imageData [i];
    cxu32 imageDataSize = texture->imageDataSize [i];
    
    CX_ASSERT (imageData);
    CX_ASSERT (imageDataSize > 0);
    
    if (texture->compressed)
    {
      switch (texture->format)
      {
        case CX_TEXTURE_FORMAT_RGBA_PVR_4BPP:
        {
          glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG, w, h, 0, imageDataSize, imageData);
          break;
        }
          
        case CX_TEXTURE_FORMAT_RGBA_PVR_2BPP:
        {
          glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG, w, h, 0, imageDataSize, imageData);
          break;
        }
          
        default:
        {
          CX_FATAL_ERROR ("cx_texture_gpu_init: Invalid texture format");
          break;
        }
      }
    }
    else
    {
      switch (texture->format)
      {
        case CX_TEXTURE_FORMAT_RGB:
        {
          glTexImage2D (GL_TEXTURE_2D, i, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->data);
          break;
        }
          
        case CX_TEXTURE_FORMAT_RGBA:
        {
          glTexImage2D (GL_TEXTURE_2D, i, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
          break;
        }
          
        case CX_TEXTURE_FORMAT_ALPHA:
        {
          glTexImage2D (GL_TEXTURE_2D, i, GL_ALPHA, w, h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture->data);
          break;
        }
          
        case CX_TEXTURE_FORMAT_LUMINANCE:
        {
          glTexImage2D (GL_TEXTURE_2D, i, GL_LUMINANCE, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture->data);
          break;
        }
          
        case CX_TEXTURE_FORMAT_LUMINANCE_ALPHA:
        {
          glTexImage2D (GL_TEXTURE_2D, i, GL_LUMINANCE_ALPHA, w, h, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, texture->data);
          break;
        }
          
        default:
        {
          CX_FATAL_ERROR ("cx_texture_gpu_init: Invalid texture format");
          break;
        }
      }
    }
    cx_gdi_assert_no_errors ();
    
    w = cx_max (w >> 1, 1);
    h = cx_max (h >> 1, 1);
  }
  
  bool npotExtensionSupported = false; // may be supported via gl extensions
  
  if (!npotExtensionSupported && texture->npot)
  {
    // set up filters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cx_gdi_assert_no_errors ();
    
    // set up coordinate wrapping
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    cx_gdi_assert_no_errors ();
  }
  else if (texture->mipmapCount > 0)
  {
    // set up filters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cx_gdi_assert_no_errors ();
    
    // set up coordinate wrapping
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    cx_gdi_assert_no_errors ();
  }
  else
  {
    // auto-generate mipmaps
    glGenerateMipmap (GL_TEXTURE_2D);
    cx_gdi_assert_no_errors ();
    
    // set up filters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); //, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cx_gdi_assert_no_errors ();
    
    // set up coordinate wrapping
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    cx_gdi_assert_no_errors ();
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_gpu_deinit (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  glDeleteTextures (1, &texture->id);
  cx_gdi_assert_no_errors ();
  
  texture->id = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_set_wrap_mode (cx_texture *texture, cx_texture_wrap_mode mode)
{
  CX_ASSERT (texture);
  
  switch (mode) 
  {
    case CX_TEXTURE_WRAP_MODE_CLAMP:  
    { 
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      break; 
    }
      
    case CX_TEXTURE_WRAP_MODE_REPEAT: 
    {
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      break; 
    }
      
    default: 
    { 
      break; 
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
