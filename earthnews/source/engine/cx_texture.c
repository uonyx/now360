//
//  cx_texture.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 19/02/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
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

typedef struct cx_texture_node 
{
  cx_texture texture;
  struct cx_texture_node *next;
} cx_texture_node;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static int s_texture_count = 0;
static cx_texture_node *s_texture_db = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_db_add (cx_texture_node *texture);
void cx_texture_db_remove (cx_texture_node *texture);
cx_texture_node *cx_texture_db_get (cx_texture *texture);
#if CX_TEXTURE_DEBUG
cx_texture *cx_texture_db_exists (const char *filename);
#endif

void cx_texture_gpu_init (cx_texture *texture);
void cx_texture_gpu_deinit (cx_texture *texture);

static bool cx_texture_load_png (cx_texture *texture, const char *filename);
extern bool cx_native_load_png (const char *filename, cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#if CX_TEXTURE_DEBUG
cx_texture * cx_texture_db_exists (const char *filename)
{
  CX_ASSERT (filename);
  
  cx_texture_node *node = s_texture_db;
  
  while (node)
  {
    if (strcmp (node->texture.filename, filename) == 0)
    {
      return &node->texture;
    }
    
    node = node->next;
  }
  
  return NULL;
}
#endif

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

cx_texture *cx_texture_create (const char *filename)
{
  CX_ASSERT (filename);
  
  // determine type and then load. 
  // only png supported 
  
  cx_texture *texture = NULL;
  
  bool isPNG = true;
  
  if (isPNG)
  {
#if CX_TEXTURE_DEBUG
    texture = cx_texture_db_exists (filename);
    
    if (texture)
    {
      CX_OUTPUTLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG, "cx_texture_create: [%s] already loaded", filename);
      return texture;
    }
#endif

    CX_OUTPUTLOG_CONSOLE (CX_TEXTURE_DEBUG_LOG, "cx_texture_create: loading [%s]", filename);
    
    cx_texture_node *textureNode;
    
    textureNode = (cx_texture_node *) cx_malloc (sizeof (cx_texture_node));
    textureNode->next = NULL;
    
#if CX_TEXTURE_DEBUG
    textureNode->texture.filename = cx_strdup (filename);
#endif
    textureNode->texture.id = 0;
    textureNode->texture.width = 0;
    textureNode->texture.height = 0;
    textureNode->texture.bpp = 0;
    textureNode->texture.format = CX_TEXTURE_FORMAT_INVALID;
    textureNode->texture.dataSize = 0;
    textureNode->texture.data = NULL;
    
    cx_texture_load_png (&textureNode->texture, filename);
    cx_texture_db_add (textureNode);
    cx_texture_gpu_init (&textureNode->texture);
    
    texture = &textureNode->texture;
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
    cx_texture_node *node = cx_texture_db_get (texture);
    CX_ASSERT (node);
    
    cx_texture_db_remove (node);
    
#if CX_TEXTURE_DEBUG
    cx_free (texture->filename);
#endif
    cx_free (node);
  }
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
    cx_free (node->texture.filename);
#endif
    cx_free (node);
    
    node = nextNode;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_texture_gpu_init (cx_texture *texture)
{
  CX_ASSERT (texture);
  
  glGenTextures (1, &texture->id);
  cx_graphics_assert_no_errors ();
  
  glBindTexture (GL_TEXTURE_2D, texture->id);
  cx_graphics_assert_no_errors ();
  
  if ((texture->format == CX_TEXTURE_FORMAT_RGB) || (texture->format == CX_TEXTURE_FORMAT_RGBA))
  {
    glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->data);
    cx_graphics_assert_no_errors ();
  }
  else
  {
    CX_FATAL_ERROR ("cx_texture_gpu_init: Invalid Texture Format");
  }
  
  // set up mipmaps
  glGenerateMipmap (GL_TEXTURE_2D);
  cx_graphics_assert_no_errors ();
  
  // set up filters
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  cx_graphics_assert_no_errors ();
  
  // set up coordinate wrapping
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  cx_graphics_assert_no_errors ();
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
