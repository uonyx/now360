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

#define CX_PVRTC_LEGACY 0

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
  4, /* CX_TEXTURE_FORMAT_RGB_PVR_4BPP */
  2, /* CX_TEXTURE_FORMAT_RGB_PVR_2BPP */
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
      default:              { CX_ERROR ("Invalid texture format"); break; }
    }
    
    texture->data = data;
    texture->dataSize = w * h * sizeof (cxu8) * g_texture_format_pixel_size [texture->format];
    texture->width = (cxu32) w;
    texture->height = (cxu32) h;
    texture->npot = (!cx_util_is_power_of_2 (texture->width)) || (!cx_util_is_power_of_2 (texture->height));
    
    texture->imageData [0] = texture->data;
    texture->imageDataSize [0] = texture->dataSize;
    texture->mipmapCount = 1;
  }
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_PVRTC_LEGACY

#define PVR_TEXTURE_FLAG_TYPE_MASK  0xff
static cx_texture *cx_texture_load_img_pvr (const char *filename, cx_file_storage_base storage)
{
  CX_ASSERT (filename);
  
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
    
    typedef struct pvr_header_v2_t
    {
      cxu32 headerLength;
      cxu32 height;
      cxu32 width;
      cxu32 numMipmaps;
      cxu32 flags;
      cxu32 dataLength;
      cxu32 bpp;
      cxu32 bitmaskRed;
      cxu32 bitmaskGreen;
      cxu32 bitmaskBlue;
      cxu32 bitmaskAlpha;
      cxu32 pvrTag;
      cxu32 numSurfs;
    } pvr_header_v2_t;
    
    char PVRTexIdentifier [4] = "PVR!";
    
    pvr_header_v2_t *pvrHeader = (pvr_header_v2_t *) data;
    
    bool validPVR = true;
    
    if (PVRTexIdentifier[0] != (((pvrHeader->pvrTag) >>  0) & 0xff) ||
        PVRTexIdentifier[1] != (((pvrHeader->pvrTag) >>  8) & 0xff) ||
        PVRTexIdentifier[2] != (((pvrHeader->pvrTag) >> 16) & 0xff) ||
        PVRTexIdentifier[3] != (((pvrHeader->pvrTag) >> 24) & 0xff))
    {
      validPVR = false;
    }
    
    if (validPVR)
    {
      cxu32 formatFlags = (pvrHeader->flags) & PVR_TEXTURE_FLAG_TYPE_MASK;
      CX_REF_UNUSED (formatFlags);
      CX_ASSERT ((formatFlags == kPVRTextureFlagTypePVRTC_4 || formatFlags == kPVRTextureFlagTypePVRTC_2));
      
      texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
      memset (texture, 0, sizeof (cx_texture));
      
      texture->data       = data;
      texture->dataSize   = dataSize;
      texture->compressed = 1;
      texture->mipmapCount = (pvrHeader->numMipmaps) + 1; // plus base
      texture->width = (pvrHeader->width);
      texture->height = (pvrHeader->height);
      texture->npot = (!cx_util_is_power_of_2 (texture->width)) || (!cx_util_is_power_of_2 (texture->height));
      texture->format = (formatFlags == kPVRTextureFlagTypePVRTC_4) ? CX_TEXTURE_FORMAT_RGBA_PVR_4BPP : CX_TEXTURE_FORMAT_RGBA_PVR_2BPP;
      
      bool hasAlpha = (pvrHeader->bitmaskAlpha) > 0;
      CX_REF_UNUSED (hasAlpha);
      
      cxu8 *bytes = texture->data + sizeof (pvr_header_v2_t);
      
      cxu32 mc = 0;
      cxu32 w = texture->width;
      cxu32 h = texture->height;
      
      cxu32 dataOffset = 0;
      cxu32 dataLength = (pvrHeader->dataLength);
      
      // Calculate the data size for each texture level and respect the minimum number of blocks
      while (dataOffset < dataLength)
      {
        cxu32 bpp = 4;
        cxu32 blockSize = 0, widthBlocks = 0, heightBlocks = 0;
        
        if (formatFlags == kPVRTextureFlagTypePVRTC_4)
        {
          blockSize = 4 * 4; // Pixel by pixel block size for 4bpp
          widthBlocks = w / 4;
          heightBlocks = h / 4;
          bpp = 4;
        }
        else
        {
          blockSize = 8 * 4; // Pixel by pixel block size for 2bpp
          widthBlocks = w / 8;
          heightBlocks = h / 4;
          bpp = 2;
        }
        
        widthBlocks = cx_max (widthBlocks, 2);
        heightBlocks = cx_max (heightBlocks, 2);
        
        cxu32 msize = widthBlocks * heightBlocks * ((blockSize  * bpp) / 8);
        
        texture->imageData [mc] = bytes + dataOffset;
        texture->imageDataSize [mc++] = msize;
        
        dataOffset += msize;
        
        w = cx_max (w >> 1, 1);
        h = cx_max (h >> 1, 1);
      }
      
      CX_ASSERT (texture->mipmapCount == mc);
    }
    else
    {
      cx_free (data);
    }
  }
  
  return texture;
}

#else

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define PVRT_IDENTIFIER       0x50565203
#define PVRT_IDENTIFIER_REV   0x03525650
static cx_texture *cx_texture_load_img_pvr (const char *filename, cx_file_storage_base storage)
{
  CX_ASSERT (filename);
  
  cx_texture *texture = NULL;
  
  cxu8 *data = NULL;
  cxu32 dataSize = 0;
  
  if (cx_file_storage_load_contents (&data, &dataSize, filename, storage))
  {
    typedef struct pvr_header_v3_t
    {
      cxu32 version;
      cxu32 flags;
      cxu64 pixelFormat;
      cxu32 colourSpace;
      cxu32 channelType;
      cxu32 height;
      cxu32 width;
      cxu32 depth;
      cxu32 numSurfaces;
      cxu32 numFaces;
      cxu32 mipmapCount;
      cxu32 metadataSize;
    } pvr_header_t;
    
    typedef enum pvr_header_v3_channel_type_t
    {
      PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORM,
      PVR_CHANNEL_TYPE_SIGNED_BYTE_NORM,
      PVR_CHANNEL_TYPE_UNSIGNED_BYTE,
      PVR_CHANNEL_TYPE_SIGNED_BYTE,
      PVR_CHANNEL_TYPE_UNSIGNED_SHORT_NORM,
      PVR_CHANNEL_TYPE_SIGNED_SHORT_NORM,
      PVR_CHANNEL_TYPE_UNSIGNED_SHORT,
      PVR_CHANNEL_TYPE_SIGNED_SHORT,
      PVR_CHANNEL_TYPE_UNSIGNED_INT_NORM,
      PVR_CHANNEL_TYPE_SIGNED_INT_NORM,
      PVR_CHANNEL_TYPE_UNSIGNED_INT,
      PVR_CHANNEL_TYPE_SIGNED_INT,
      PVR_CHANNEL_TYPE_FLOAT,
    } pvr_header_v3_channel_type_t;
    
    typedef enum pvr_header_v3_pixel_format_type_t
    {
      PVR_PIXEL_FORMAT_PVRTC_2BPP_RGB,
      PVR_PIXEL_FORMAT_PVRTC_2BPP_RGBA,
      PVR_PIXEL_FORMAT_PVRTC_4BPP_RGB,
      PVR_PIXEL_FORMAT_PVRTC_4BPP_RGBA,
      PVR_PIXEL_FORMAT_PVRTC_II_2BPP,
      PVR_PIXEL_FORMAT_PVRTC_II_4BPP,
      PVR_PIXEL_FORMAT_ETC1,
    } pvr_header_v3_pixel_format_type_t;
    
    pvr_header_t *pvrHeader = (pvr_header_t *) data;
    
    if ((pvrHeader->version == PVRT_IDENTIFIER) || (pvrHeader->version == PVRT_IDENTIFIER_REV))
    {
      bool pvrtcHardwareSupport = cx_gdi_get_extension_supported (CX_GDI_EXTENSION_PVRTC);
      CX_FATAL_ASSERT (pvrtcHardwareSupport);
      CX_REF_UNUSED (pvrtcHardwareSupport);
      
      cxu32 headerSize = sizeof (pvr_header_t) + pvrHeader->metadataSize;
      cxu32 textureDataSize = dataSize - headerSize;
      cxu8 *textureData = data + headerSize;
      
      bool byteswap = !(pvrHeader->version == PVRT_IDENTIFIER_REV);
    
      if (byteswap)
      {
        pvrHeader->version      = cx_util_byte_swap_u32 (pvrHeader->version);
        pvrHeader->flags        = cx_util_byte_swap_u32 (pvrHeader->flags);
        pvrHeader->pixelFormat  = cx_util_byte_swap_u64 (pvrHeader->pixelFormat);
        pvrHeader->colourSpace  = cx_util_byte_swap_u32 (pvrHeader->colourSpace);
        pvrHeader->channelType  = cx_util_byte_swap_u32 (pvrHeader->channelType);
        pvrHeader->height       = cx_util_byte_swap_u32 (pvrHeader->height);
        pvrHeader->width        = cx_util_byte_swap_u32 (pvrHeader->width);
        pvrHeader->depth        = cx_util_byte_swap_u32 (pvrHeader->depth);
        pvrHeader->numSurfaces  = cx_util_byte_swap_u32 (pvrHeader->numSurfaces);
        pvrHeader->numFaces     = cx_util_byte_swap_u32 (pvrHeader->numFaces);
        pvrHeader->mipmapCount  = cx_util_byte_swap_u32 (pvrHeader->mipmapCount);
        pvrHeader->metadataSize = cx_util_byte_swap_u32 (pvrHeader->metadataSize);

        cxu32 channelblockSize = 0;
        
        switch (pvrHeader->channelType)
        {
          case PVR_CHANNEL_TYPE_UNSIGNED_BYTE_NORM:
          case PVR_CHANNEL_TYPE_SIGNED_BYTE_NORM:
          case PVR_CHANNEL_TYPE_UNSIGNED_BYTE:
          case PVR_CHANNEL_TYPE_SIGNED_BYTE:
          {
            channelblockSize = 1;
            break;
          }
            
          case PVR_CHANNEL_TYPE_UNSIGNED_SHORT_NORM:
          case PVR_CHANNEL_TYPE_SIGNED_SHORT_NORM:
          case PVR_CHANNEL_TYPE_UNSIGNED_SHORT:
          case PVR_CHANNEL_TYPE_SIGNED_SHORT:
          {
            channelblockSize = 2;
            break;
          }
            
          case PVR_CHANNEL_TYPE_UNSIGNED_INT_NORM:
          case PVR_CHANNEL_TYPE_SIGNED_INT_NORM:
          case PVR_CHANNEL_TYPE_UNSIGNED_INT:
          case PVR_CHANNEL_TYPE_SIGNED_INT:
          case PVR_CHANNEL_TYPE_FLOAT:
          {
            channelblockSize = 4;
            break;
          }
            
          default:
          {
            CX_ERROR ("Unsupported channelType");
            break;
          }
        }
        
        if (channelblockSize > 1)
        {
          for (cxu32 i = 0; i < textureDataSize; i += channelblockSize)
          {
            cx_util_byte_swap (textureData + i, channelblockSize);
          }
        }
      }
    
      CX_ASSERT (pvrHeader->numSurfaces <= 1);
      CX_ASSERT (pvrHeader->numFaces <= 1);

      cxu32 bpp = 0;
      cxu32 minw = 0, minh = 0, mind = 0;
      
      switch (pvrHeader->pixelFormat)
      {
        case PVR_PIXEL_FORMAT_PVRTC_2BPP_RGB:
        case PVR_PIXEL_FORMAT_PVRTC_2BPP_RGBA:
        {
          bpp = 2;
          minw = 16; minh = 8; mind = 1;
          break;
        }
          
        case PVR_PIXEL_FORMAT_PVRTC_4BPP_RGB:
        case PVR_PIXEL_FORMAT_PVRTC_4BPP_RGBA:
        {
          bpp = 4;
          minw = 8; minh = 8; mind = 1;
          break;
        }
          
        case PVR_PIXEL_FORMAT_PVRTC_II_2BPP:
        {
          bpp = 2;
          minw = 8; minh = 4; mind = 1;
          break;
        }
          
        case PVR_PIXEL_FORMAT_PVRTC_II_4BPP:
        {
          bpp = 4;
          minw = 4; minh = 4; mind = 1;
          break;
        }
          
        default:
        {
          CX_ERROR ("Unsupported pixelFormat");
          break;
        }
      }
      
      texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
      memset (texture, 0, sizeof (cx_texture));
      
      cxu32 totalImageDataSize = 0;
      
      for (cxu32 m = 0, mc = pvrHeader->mipmapCount; m < mc; ++m)
      {
        cxu32 mw = cx_max (1, pvrHeader->width >> m);
        cxu32 mh = cx_max (1, pvrHeader->height >> m);
        cxu32 md = cx_max (1, pvrHeader->depth >> m);
        
#if 0
        mw = mw + (minw - (mw % minw));
        mh = mh + (minh - (mh % minh));
        md = md + (mind - (md % mind));
#else
        mw = mw + (-mw % minw);
        mh = mh + (-mh % minh);
        md = md + (-md % mind);
#endif
        cxu32 msize = bpp * mw * mh * md;
        
        msize = (msize / 8) * pvrHeader->numSurfaces * pvrHeader->numFaces;
  
        texture->imageData [m] = textureData + totalImageDataSize;
        texture->imageDataSize [m] = msize;
        
        totalImageDataSize += msize;
      }
      
      CX_ASSERT (totalImageDataSize == textureDataSize);
      
      texture->data         = data;
      texture->dataSize     = dataSize;
      texture->compressed   = 1;
      texture->mipmapCount  = pvrHeader->mipmapCount;
      texture->width        = pvrHeader->width;
      texture->height       = pvrHeader->height;
      texture->npot         = (!cx_util_is_power_of_2 (texture->width)) || (!cx_util_is_power_of_2 (texture->height));
      
      switch (pvrHeader->pixelFormat)
      {
        case PVR_PIXEL_FORMAT_PVRTC_2BPP_RGB:   { texture->format  = CX_TEXTURE_FORMAT_RGB_PVR_2BPP; break; }
        case PVR_PIXEL_FORMAT_PVRTC_2BPP_RGBA:  { texture->format  = CX_TEXTURE_FORMAT_RGBA_PVR_2BPP; break; }
        case PVR_PIXEL_FORMAT_PVRTC_4BPP_RGB:   { texture->format  = CX_TEXTURE_FORMAT_RGB_PVR_2BPP; break; }
        case PVR_PIXEL_FORMAT_PVRTC_4BPP_RGBA:  { texture->format  = CX_TEXTURE_FORMAT_RGBA_PVR_4BPP; break; }
        case PVR_PIXEL_FORMAT_PVRTC_II_2BPP:    { texture->format  = CX_TEXTURE_FORMAT_RGB_PVR_2BPP; break; }
        case PVR_PIXEL_FORMAT_PVRTC_II_4BPP:    { texture->format  = CX_TEXTURE_FORMAT_RGB_PVR_4BPP; break; }
        default:                                { CX_ERROR ("Unsupported pixelFormat"); break; }
      }      
    }
    else
    {
      cx_free (data);
    }
  }
  
  return texture;
}
#endif

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
  texture->mipmapCount       = 1;
  texture->npot              = (!cx_util_is_power_of_2 (width)) || (!cx_util_is_power_of_2 (height));
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create_from_file (const char *filename, cx_file_storage_base storage, bool genMipmaps)
{
  CX_ASSERT (filename);
  
  cx_texture *texture = cx_texture_load_img_xxx (filename, storage);
  
  if (texture == NULL)
  {
    texture = cx_texture_load_img_pvr (filename, storage);
  }
  
  if (texture)
  {
    cx_texture_gpu_init (texture, genMipmaps);
    cx_texture_data_destroy (texture);
  }

  CX_LOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE && !texture, "cx_texture_create: failed to load [%s]", filename);
  
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

void cx_texture_gpu_init (cx_texture *texture, bool genMipmaps)
{
  CX_ASSERT (texture);
  CX_ASSERT (texture->data);
  CX_ASSERT (texture->id == 0);
  CX_ASSERT (texture->width > 0);
  CX_ASSERT (texture->height > 0);
  CX_ASSERT (texture->mipmapCount <= CX_TEXTURE_MAX_MIPMAP_COUNT);
  
  CX_LOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->width  [%d]", texture->width);
  CX_LOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->height [%d]", texture->height);
  CX_LOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->format [%d]", texture->format);
  CX_LOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->compressed [%s]", texture->compressed ? "true" : "false");
  
  if (texture->compressed)
  {
    glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
  }
  
  glGenTextures (1, &texture->id);
  cx_gdi_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_gdi_assert_no_errors ();

  if (!genMipmaps)
  {
    // override mipmap count
    texture->mipmapCount = 1;
  }
  
  for (cxu32 i = 0, c = texture->mipmapCount; i < c; ++i)
  {
    cxu8 *imageData = texture->imageData [i];
    cxu32 imageDataSize = texture->imageDataSize [i];
    
    cxu32 w = cx_max (1, texture->width >> i);
    cxu32 h = cx_max (1, texture->height >> i);
    
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
          
        case CX_TEXTURE_FORMAT_RGB_PVR_4BPP:
        {
          glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG, w, h, 0, imageDataSize, imageData);
          break;
        }
          
        case CX_TEXTURE_FORMAT_RGB_PVR_2BPP:
        {
          glCompressedTexImage2D (GL_TEXTURE_2D, i, GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG, w, h, 0, imageDataSize, imageData);
          break;
        }
          
        default:
        {
          CX_ERROR ("cx_texture_gpu_init: Invalid texture format");
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
          CX_ERROR ("cx_texture_gpu_init: Invalid texture format");
          break;
        }
      }
    }
    
    cx_gdi_assert_no_errors ();
  }
  
  if (texture->mipmapCount > 1)
  {
    CX_ASSERT (!texture->npot);
    
    // set up filters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    cx_gdi_assert_no_errors ();
    
    // set up coordinate wrapping
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  }
  else // mipmapCount == 1
  {
    CX_ASSERT (texture->mipmapCount == 1);
    
    bool npotExtensionSupported = cx_gdi_get_extension_supported (CX_GDI_EXTENSION_NPOT);
    
    if (texture->npot && !npotExtensionSupported)
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
    else
    {
      if (genMipmaps)
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
      else
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
    }
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
