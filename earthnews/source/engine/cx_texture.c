//
//  cx_texture.
//
//  Created by Ubaka Onyechi on 19/02/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_texture.h"
#include "cx_utility.h"
#include "cx_string.h"
#include "cx_graphics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int s_texture_format_pixel_size [CX_TEXTURE_NUM_FORMATS] = 
{
  4,  /* CX_TEXTURE_FORMAT_RGB */
  4,  /* CX_TEXTURE_FORMAT_RGBA */
  1   /* CX_TEXTURE_FORMAT_ALPHA */
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_TEXTURE_DB 0

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
  if (cx_native_load_png (filename, texture))
  {
    CX_ASSERT (cx_util_is_power_of_2 ((cxu32) texture->width));
    CX_ASSERT (cx_util_is_power_of_2 ((cxu32) texture->height));
    
    return true;
  }
  
  return false;
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
  texture->compressed = false;
  
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
  cx_graphics_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_graphics_assert_no_errors ();
  
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
  cx_graphics_assert_no_errors ();
  
  // generate mipmaps
  //glGenerateMipmap (GL_TEXTURE_2D);
  //cx_graphics_assert_no_errors ();
  
  // set up filters
  //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  cx_graphics_assert_no_errors ();
  
  // set up coordinate wrapping
  //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  //glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  //cx_graphics_assert_no_errors ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_gpu_deinit (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  glDeleteTextures (1, &texture->id);
  cx_graphics_assert_no_errors ();
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

#if CX_TEXTURE_DB
static int s_texture_count = 0;
static cx_texture_node *s_texture_db = NULL;
cx_texture_node *cx_texture_db_get (cx_texture *texture);
void cx_texture_db_add (cx_texture_node *texture);
void cx_texture_db_remove (cx_texture_node *texture);
void cx_texture_db_clean_up (void);
cx_texture *cx_texture_db_exists (const char *filename);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_TEXTURE_DB
void cx_texture_db_add (cx_texture_node *texture)
{
  CX_ASSERT (texture);
  
  cx_texture_node *t = texture;
  
  t->next = s_texture_db;
  
  s_texture_db = t;
  
  s_texture_count++;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_db_remove (cx_texture_node *texture)
{
  cx_texture_node *prev, *curr;
  
  prev = NULL;
  curr = s_texture_db;
  
  while (curr)
  {
    if (curr == texture)
    {
      if (curr == s_texture_db)
      {
        s_texture_db = curr->next;
      }
      else
      {
        prev->next = curr->next;
      }
      
      --s_texture_count;
      return;
    }
    
    prev = curr;
    curr = curr->next;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture_node *cx_texture_db_get (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  cx_texture_node *node = s_texture_db;
  
  while (node)
  {
    if (&node->texture == texture)
    {
      return node;
    }
    
    node = node->next;
  }
  
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_texture * cx_texture_db_exists (const char *filename)
{
  CX_ASSERT (filename);
  
  cx_texture_node *node = s_texture_db;
  
  while (node)
  {
    if (strcmp (node->filename, filename) == 0)
    {
      return &node->texture;
    }
    
    node = node->next;
  }
  
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_db_clean_up (void)
{
  cx_texture_node *node;
  cx_texture_node *nextNode;
  
  node = s_texture_db;
  
  while (node)
  {
    nextNode = node->next;
    
#if CX_TEXTURE_DEBUG
    cx_free (node->filename);
#endif
    cx_free (node);
    
    node = nextNode;
  }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
