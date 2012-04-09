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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static GLenum s_openglBlendModes [CX_NUM_GRAPHICS_BLEND_MODES] =
{
  GL_SRC_ALPHA,
  GL_ONE_MINUS_SRC_ALPHA,
};

static cxi32 s_screenWidth = 0;
static cxi32 s_screenHeight = 0;
static cx_mat4x4 s_modelViewProj;
static bool s_initialised = false;
static cx_graphics_renderstate s_activeRenderstate = CX_GRAPHICS_RENDER_STATE_NONE;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_graphics_initialise (cxi32 width, cxi32 height)
{
  CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG_ENABLED, "cx_graphics_initialise: width [%d], height [%d]", width, height);
  
  cx_graphics_print_info ();
  
  cx_mat4x4_identity (&s_modelViewProj);
  
  s_screenWidth = width;
  s_screenHeight = height;
  
  s_initialised = true;
  
  cx_graphics_set_viewport (width, height);
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_graphics_deinitialise (void)
{
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_set_mvp_matrix (const cx_mat4x4 *matrix)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT (matrix);
  
  s_modelViewProj = *matrix;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_get_mvp_matrix (cx_mat4x4 *matrix)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT (matrix);
  
  *matrix = s_modelViewProj;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_graphics_get_aspect_ratio (void)
{
  CX_ASSERT (s_initialised);
  
  cxf32 aspectRatio = (cxf32) s_screenWidth / (cxf32) s_screenHeight;
  
  return aspectRatio;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_graphics_get_screen_width (void)
{
  CX_ASSERT (s_initialised);
  return (float) s_screenWidth;
}

cxf32 cx_graphics_get_screen_height (void)
{
  CX_ASSERT (s_initialised);
  return (float) s_screenHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_set_viewport (cxi32 width, cxi32 height)
{
  CX_ASSERT (s_initialised);
  
  glViewport (0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_clear (const cx_colour *colour)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT (colour);
  
  glClearColor (colour->r, colour->g, colour->b, colour->a);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_enable_z_buffer (bool enable)
{
  glDepthMask (enable);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_set_blend_mode (cx_graphics_blend_mode src, cx_graphics_blend_mode dest)
{
  CX_ASSERT ((src > CX_GRAPHICS_BLEND_MODE_INVALID) && (src < CX_NUM_GRAPHICS_BLEND_MODES));
  CX_ASSERT ((dest > CX_GRAPHICS_BLEND_MODE_INVALID) && (dest < CX_NUM_GRAPHICS_BLEND_MODES));
  
  GLenum srcMode = s_openglBlendModes [src];
  GLenum destMode = s_openglBlendModes [dest];
  glBlendFunc (srcMode, destMode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_activate_renderstate (cx_graphics_renderstate renderstate)
{
  //CX_ASSERT (renderstate != CX_GRAPHICS_RENDER_STATE_NONE);
  
  s_activeRenderstate = renderstate;
  
  if (renderstate & CX_GRAPHICS_RENDER_STATE_CULL)
  {
    glCullFace (GL_FRONT);
    glEnable (GL_CULL_FACE);
  }
  else 
  {
    glDisable (GL_CULL_FACE);
  }

  if (renderstate & CX_GRAPHICS_RENDER_STATE_DEPTH_TEST)
  {
    glEnable (GL_DEPTH_TEST);
  }
  else 
  {
    glDisable (GL_DEPTH_TEST);
  }
  
  if (renderstate & CX_GRAPHICS_RENDER_STATE_BLEND)
  {
    glEnable (GL_BLEND);
  }
  else 
  {
    glDisable (GL_BLEND);
  } 
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_get_renderstate (cx_graphics_renderstate *renderstate)
{
  *renderstate = s_activeRenderstate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_unbind_all_buffers (void)
{
  glBindVertexArrayOES (0);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
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
    case GL_INVALID_ENUM:       { CX_DEBUGLOG_CONSOLE (1, "GL_INVALID_ENUM"); break; }
    case GL_INVALID_VALUE:      { CX_DEBUGLOG_CONSOLE (1, "GL_INVALID_VALUE"); break; }
    case GL_INVALID_OPERATION:  { CX_DEBUGLOG_CONSOLE (1, "GL_INVALID_OPERATION"); break; }
    case GL_OUT_OF_MEMORY:      { CX_DEBUGLOG_CONSOLE (1, "GL_OUT_OF_MEMORY"); break; }
    default:                    { CX_DEBUGLOG_CONSOLE (1, "UNKNOWN GL ERROR"); break; }
  }
 
  CX_ASSERT (error == GL_NO_ERROR);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_print_info (void)
{
#if CX_GRAPHICS_DEBUG_LOG_ENABLED
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
   
    CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG_ENABLED, "%s: %d", str, val);
  }
  
  const char *supportedExtensions = (const char *) glGetString (GL_EXTENSIONS);
  CX_DEBUGLOG_CONSOLE (CX_GRAPHICS_DEBUG_LOG_ENABLED, "%s", supportedExtensions);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

