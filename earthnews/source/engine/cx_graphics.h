//
//  cx_graphics.h
//
//  Created by Ubaka Onyechi on 01/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_GRAPHICS_H
#define CX_GRAPHICS_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"
#include "cx_matrix4x4.h"
#include "cx_colour.h"
#include "cx_shader.h"
#include "cx_material.h"
#include "cx_mesh.h"
#include "cx_draw.h"
#include "cx_font.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_GRAPHICS_DEBUG                     (CX_DEBUG && 1)
#define CX_GRAPHICS_DEBUG_LOG_ENABLED         (CX_DEBUG && 1)

#if CX_GRAPHICS_DEBUG
#define cx_graphics_assert_no_errors()        _cx_graphics_assert_no_errors ()
#else
#define cx_graphics_assert_no_errors()
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_GRAPHICS_RENDER_STATE_NONE         0x0
#define CX_GRAPHICS_RENDER_STATE_CULL         0x1
#define CX_GRAPHICS_RENDER_STATE_DEPTH_TEST   0x2
#define CX_GRAPHICS_RENDER_STATE_BLEND        0x4

typedef cxu32 cx_graphics_renderstate;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  CX_GRAPHICS_BLEND_MODE_INVALID = -1,
  CX_GRAPHICS_BLEND_MODE_SRC_ALPHA,
  CX_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA,
  
  CX_NUM_GRAPHICS_BLEND_MODES
} cx_graphics_blend_mode;

typedef enum
{
  CX_GRAPHICS_TRANSFORM_P,
  CX_GRAPHICS_TRANSFORM_MV,
  CX_GRAPHICS_TRANSFORM_MVP,
  CX_NUM_GRAPHICS_TRANSFORMS,
} cx_graphics_transform;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_graphics_init (void);
bool cx_graphics_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_graphics_get_aspect_ratio (void);
cxf32 cx_graphics_get_screen_width (void);
cxf32 cx_graphics_get_screen_height (void);
void cx_graphics_set_screen_dimensions (cxi32 width, cxi32 height);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_clear (const cx_colour *colour);
void cx_graphics_set_viewport (cxi32 width, cxi32 height);
void cx_graphics_enable_z_buffer (bool enable);
void cx_graphics_set_blend_mode (cx_graphics_blend_mode src, cx_graphics_blend_mode dest);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_get_renderstate (cx_graphics_renderstate *renderstate);
void cx_graphics_activate_renderstate (cx_graphics_renderstate renderstate);
void cx_graphics_unbind_all_buffers (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_set_transform (cx_graphics_transform transform, const cx_mat4x4 *matrix);
void cx_graphics_get_transform (cx_graphics_transform transform, cx_mat4x4 *matrix);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_graphics_print_info (void);
void _cx_graphics_assert_no_errors (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
