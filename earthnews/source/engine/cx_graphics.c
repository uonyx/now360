//
//  cx_graphics.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 01/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_graphics.h"
#include "cx_math.h"
#include "cx_file.h"
#include "cx_string.h"

#include "yajl_tree.h"
#include "yajl_parse.h"
#include "yajl_gen.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cxi32 s_screenWidth = 0;
static cxi32 s_screenHeight = 0;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_graphics_initialise (cxi32 width, cxi32 height)
{
  cx_graphics_print_info ();
  
  s_screenWidth = width;
  s_screenHeight = height;
  
  CX_OUTPUTLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "cx_graphics_initialise: width [%d], height [%d]", width, height);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL cx_graphics_deinitialise (void)
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_update (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_graphics_get_aspect_ratio (void)
{
  cxf32 aspectRatio = (cxf32) s_screenWidth / (cxf32) s_screenHeight;
  
  return aspectRatio;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_get_screen_dimensions (cxi32 *width, cxi32 *height)
{
  *width = s_screenWidth;
  *height = s_screenHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_enable_depth_test (bool enable)
{
  if (enable)
  {
    glEnable (GL_DEPTH_TEST);
  }
  else
  {
    glDisable (GL_DEPTH_TEST);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_enable_culling (bool enable, int face)
{
  CX_REFERENCE_UNUSED_VARIABLE(face);
  
  if (enable)
  {
    glCullFace (GL_FRONT);
    glEnable (GL_CULL_FACE);
  }
  else
  {
    glDisable (GL_CULL_FACE);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_set_viewport (cxi32 width, cxi32 height)
{
  glViewport (0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_clear (const cx_colour *colour)
{
  CX_ASSERT (colour);
  
  float r, g, b, a;
  cx_colour_get_f32 (colour, &r, &g, &b, &a);
  
  glClearColor (r, g, b, a);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _cx_graphics_assert_no_errors (void)
{
#if CX_GRAPHICS_DEBUG
  GLenum error = glGetError ();
  
  switch (error) 
  {
    case GL_NO_ERROR:           { break; }
    case GL_INVALID_ENUM:       { CX_OUTPUTLOG_CONSOLE (1, "GL_INVALID_ENUM"); break; }
    case GL_INVALID_VALUE:      { CX_OUTPUTLOG_CONSOLE (1, "GL_INVALID_VALUE"); break; }
    case GL_INVALID_OPERATION:  { CX_OUTPUTLOG_CONSOLE (1, "GL_INVALID_OPERATION"); break; }
    case GL_OUT_OF_MEMORY:      { CX_OUTPUTLOG_CONSOLE (1, "GL_OUT_OF_MEMORY"); break; }
    default:                    { CX_OUTPUTLOG_CONSOLE (1, "UNKNOWN GL ERROR"); break; }
  }
 
  CX_ASSERT (error == GL_NO_ERROR);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_print_info (void)
{
#if CX_GRAPHICS_DEBUG_LOG
  struct graphics_info
  {
    GLenum type;
    const char *name;
  };
  
  struct graphics_info info [] =
  {
    { GL_MAX_VERTEX_ATTRIBS, "GL_MAX_VERTEX_ATTRIBS" },
    { GL_MAX_VERTEX_UNIFORM_VECTORS, "GL_MAX_VERTEX_UNIFORM_VECTORS" },
    { GL_MAX_VARYING_VECTORS, "GL_MAX_VARYING_VECTORS" },
    { GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS" },
    { GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS"},
    { GL_MAX_TEXTURE_IMAGE_UNITS, "GL_MAX_TEXTURE_IMAGE_UNITS" },
    { GL_MAX_TEXTURE_SIZE, "GL_MAX_TEXTURE_SIZE" },
    { GL_DEPTH_BITS, "GL_DEPTH_BITS" },
    { GL_STENCIL_BITS, "GL_STENCIL_BITS"},
  };
  
  int numInfo = sizeof (info) / sizeof (struct graphics_info);
  
  int i;
  for (i = 0; i < numInfo; i++)
  {
    const char *str = info [i].name;
    GLenum type = info [i].type;
    
    GLint val;
    glGetIntegerv (type, &val);
   
    CX_OUTPUTLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "%s: %d", str, val);
  }
  
  const char *supportedExtensions = (const char *) glGetString (GL_EXTENSIONS);
  CX_OUTPUTLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG, "%s", supportedExtensions);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

