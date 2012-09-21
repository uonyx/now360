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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cxu8 s_texture_format_pixel_size [CX_TEXTURE_NUM_FORMATS] = 
{
  4,  /* CX_TEXTURE_FORMAT_RGB */
  4,  /* CX_TEXTURE_FORMAT_RGBA */
  1   /* CX_TEXTURE_FORMAT_ALPHA */
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

static bool cx_texture_load_png (cx_texture *texture, const char *filename);
extern bool cx_native_load_png (const char *filename, cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_texture_load_png (cx_texture *texture, const char *filename)
{
  bool success = false;
  
  if (cx_native_load_png (filename, texture))
  {
    texture->npot = (!cx_util_is_power_of_2 (texture->width)) || 
                    (!cx_util_is_power_of_2 (texture->height));
    
    success = true;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create (cxu32 width, cxu32 height, cx_texture_format format)
{
  CX_ASSERT ((format > CX_TEXTURE_FORMAT_INVALID) && (format < CX_TEXTURE_NUM_FORMATS));
  
  cx_texture *texture = (cx_texture *) cx_malloc (sizeof (cx_texture));

  texture->id         = 0;
  texture->width      = width;
  texture->height     = height;
  texture->format     = format;
  texture->dataSize   = width * height * sizeof (cxu8) * s_texture_format_pixel_size [format];
  texture->data       = (cxu8 *) cx_malloc (texture->dataSize);
  texture->compressed = 0;
  texture->npot       = (!cx_util_is_power_of_2 (width)) || (!cx_util_is_power_of_2 (height));
  
  //cx_texture_gpu_init (texture);
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture *cx_texture_create_from_file (const char *filename)
{
  CX_ASSERT (filename);
  
  // determine type and then load. 
  // only png supported 
  
  cx_texture *texture = NULL;
  
  bool isPNG = true;
  
  if (isPNG)
  {
    CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_create: loading [%s]", filename);
    
    texture = (cx_texture *) cx_malloc (sizeof (cx_texture));
    
    texture->id         = 0;
    texture->width      = 0;
    texture->height     = 0;
    texture->format     = CX_TEXTURE_FORMAT_INVALID;
    texture->dataSize   = 0;
    texture->data       = NULL;
    texture->compressed = false;
    
    if (cx_texture_load_png (texture, filename))
    {
      cx_texture_gpu_init (texture);
    }
    else
    {
      CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_create: failed to load [%s]", filename);
      cx_free (texture);
      texture = NULL;
    }
  }
  
  return texture;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_destroy (cx_texture *texture)
{
  if (texture)
  {
    cx_texture_gpu_deinit (texture);
    cx_free (texture);
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_gpu_init (cx_texture *texture)
{
  CX_ASSERT (texture);
   
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->width  [%d]", texture->width);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->height [%d]", texture->height);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->format [%d]", texture->format);
  CX_DEBUGLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG_ENABLE, "cx_texture_gpu_init: texture->compressed [%s]", texture->compressed ? "true" : "false");
  
  glGenTextures (1, &texture->id);
  cx_gdi_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_gdi_assert_no_errors ();
  
  if (texture->compressed)
  {
    CX_FATAL_ERROR ("cx_texture_gpu_init: Compressed Textures: Not yet implemented. See \"glCompressedTexImage2D\"");
  }
  else
  {
    switch (texture->format) 
    {
      case CX_TEXTURE_FORMAT_RGB:
      case CX_TEXTURE_FORMAT_RGBA:
      {
        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
        break;
      }
        
      case CX_TEXTURE_FORMAT_ALPHA:
      {
        glTexImage2D (GL_TEXTURE_2D, 0, GL_ALPHA, texture->width, texture->height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, texture->data);
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
  
  bool npotExtensionSupported = false;
  
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
  else
  {
    // generate mipmaps
    glGenerateMipmap (GL_TEXTURE_2D);
    cx_gdi_assert_no_errors ();
    
    // set up filters
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
