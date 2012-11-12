//
//  cx_graphics.c
//
//  Created by Ubaka Onyechi on 01/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

#include "cx_gdi.h"
#include "cx_file.h"
#include "cx_string.h"
#include "cx_shader.h"
#include "cx_material.h"
#include "cx_mesh.h"
#include "cx_draw.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_GDI_DEFAULT_SCREEN_WIDTH     1280
#define CX_GDI_DEFAULT_SCREEN_HEIGHT    720

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool s_initialised = false;

static cxi32 s_screenWidth = 0;
static cxi32 s_screenHeight = 0;

static cx_gdi_renderstate s_activeRenderstate = CX_GDI_RENDER_STATE_NONE;
static GLenum s_openglBlendModes [CX_NUM_GDI_BLEND_MODES] =
{
  GL_SRC_ALPHA,
  GL_ONE_MINUS_SRC_ALPHA,
};

static cx_mat4x4 s_transforms [CX_NUM_GDI_TRANSFORMS];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_gdi_init (void)
{
  CX_ASSERT (!s_initialised);
  
  cx_gdi_print_info ();
  
  s_screenWidth = CX_GDI_DEFAULT_SCREEN_WIDTH;
  s_screenHeight = CX_GDI_DEFAULT_SCREEN_HEIGHT;
  
  cx_mat4x4_identity (&s_transforms [CX_GDI_TRANSFORM_P]);
  cx_mat4x4_identity (&s_transforms [CX_GDI_TRANSFORM_MV]);
  cx_mat4x4_identity (&s_transforms [CX_GDI_TRANSFORM_MVP]);
  
  s_initialised = true;
  
  return s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_gdi_deinit (void)
{
  if (s_initialised)
  {
    s_initialised = false;
  }
  
  return !s_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_gdi_get_aspect_ratio (void)
{
  CX_ASSERT (s_initialised);
  
  cxf32 aspectRatio = (cxf32) s_screenWidth / (cxf32) s_screenHeight;
  
  return aspectRatio;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_gdi_get_screen_width (void)
{
  CX_ASSERT (s_initialised);
  return (float) s_screenWidth;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_gdi_get_screen_height (void)
{
  CX_ASSERT (s_initialised);
  return (float) s_screenHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_transform (cx_gdi_transform transform, const cx_mat4x4 *matrix)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT ((transform >= 0) && (transform < CX_NUM_GDI_TRANSFORMS));
  CX_ASSERT (matrix);
  
  s_transforms [transform] = *matrix;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_get_transform (cx_gdi_transform transform, cx_mat4x4 *matrix)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT ((transform >= 0) && (transform < CX_NUM_GDI_TRANSFORMS));
  CX_ASSERT (matrix);
  
  *matrix = s_transforms [transform];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_screen_dimensions (cxi32 width, cxi32 height)
{
  CX_DEBUGLOG_CONSOLE (CX_GDI_DEBUG_LOG_ENABLED, "cx_gdi_set_screen_dimensions: width [%d], height [%d]", width, height);
  
  s_screenWidth = width;
  s_screenHeight = height;
  
  cx_gdi_set_viewport (width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_viewport (cxi32 width, cxi32 height)
{
  CX_ASSERT (s_initialised);
  
  glViewport (0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_clear (const cx_colour *colour)
{
  CX_ASSERT (s_initialised);
  CX_ASSERT (colour);
  
  glClearColor (colour->r, colour->g, colour->b, colour->a);
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_enable_z_buffer (bool enable)
{
  glDepthMask (enable);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_blend_mode (cx_gdi_blend_mode src, cx_gdi_blend_mode dest)
{
  CX_ASSERT ((src > CX_GDI_BLEND_MODE_INVALID) && (src < CX_NUM_GDI_BLEND_MODES));
  CX_ASSERT ((dest > CX_GDI_BLEND_MODE_INVALID) && (dest < CX_NUM_GDI_BLEND_MODES));
  
  GLenum srcMode = s_openglBlendModes [src];
  GLenum destMode = s_openglBlendModes [dest];
  glBlendFunc (srcMode, destMode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_renderstate (cx_gdi_renderstate renderstate)
{
  //CX_ASSERT (renderstate != CX_GDI_RENDER_STATE_NONE);
  
  s_activeRenderstate = renderstate;
  
  if (renderstate & CX_GDI_RENDER_STATE_CULL)
  {
    //glCullFace (GL_FRONT);
    glEnable (GL_CULL_FACE);
  }
  else 
  {
    glDisable (GL_CULL_FACE);
  }

  if (renderstate & CX_GDI_RENDER_STATE_DEPTH_TEST)
  {
    glEnable (GL_DEPTH_TEST);
  }
  else 
  {
    glDisable (GL_DEPTH_TEST);
  }
  
  if (renderstate & CX_GDI_RENDER_STATE_BLEND)
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

void cx_gdi_get_renderstate (cx_gdi_renderstate *renderstate)
{
  *renderstate = s_activeRenderstate;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_unbind_all_buffers (void)
{
  glBindVertexArrayOES (0);
  glBindBuffer (GL_ARRAY_BUFFER, 0);
  glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _cx_gdi_assert_no_errors (void)
{
#if CX_GDI_DEBUG
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

void cx_gdi_print_info (void)
{
#if CX_GDI_DEBUG_LOG_ENABLED
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
    { GL_STENCIL_BITS, "GL_STENCIL_BITS" },
    { GL_NUM_COMPRESSED_TEXTURE_FORMATS, "GL_NUM_COMPRESSED_TEXTURE_FORMATS" }
  };
  
  int numInfo = sizeof (info) / sizeof (struct graphics_info);
  
  for (int i = 0; i < numInfo; i++)
  {
    const char *str = info [i].name;
    GLenum type = info [i].type;
    
    GLint val;
    glGetIntegerv (type, &val);
   
    CX_DEBUGLOG_CONSOLE (CX_GDI_DEBUG_LOG_ENABLED, "%s: %d", str, val);
  }
  
  // compressed texture support
  
  GLint numCompressedTextures = 0;
  glGetIntegerv (GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numCompressedTextures);
  
  if (numCompressedTextures > 0)
  {
    GLint *compressedFormats = (GLint *) cx_malloc (sizeof (GLint) * numCompressedTextures);
    glGetIntegerv (GL_COMPRESSED_TEXTURE_FORMATS, compressedFormats);
    
    CX_DEBUGLOG_CONSOLE (CX_GDI_DEBUG_LOG_ENABLED, "Supported compressed texture formats:");
    
    for (int j = 0; j < numCompressedTextures; ++j)
    {
      GLenum format = (GLenum) compressedFormats [j];
    
      const char *formatStr = NULL;
      
      switch (format)
      {
        case GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG:  { formatStr = "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG"; break; }
        case GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG:  { formatStr = "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG"; break; }
        case GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG: { formatStr = "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG"; break; }
        case GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG: { formatStr = "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG"; break; }
        default:                                  { formatStr = "GL_COMPRESSED_FORMAT_UNKNOWN!"; break; }
      }
                                                    
      CX_DEBUGLOG_CONSOLE (CX_GDI_DEBUG_LOG_ENABLED, "%s", formatStr);
      CX_REFERENCE_UNUSED_VARIABLE (formatStr);
    }
    
    cx_free (compressedFormats);
  }
  
  // supported extensions
  
  const char *supportedExtensions = (const char *) glGetString (GL_EXTENSIONS);
  CX_DEBUGLOG_CONSOLE (CX_GDI_DEBUG_LOG_ENABLED, "%s", supportedExtensions);
  CX_REFERENCE_UNUSED_VARIABLE (supportedExtensions);
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

