//
//  cx_gdi.h
//
//  Created by Ubaka Onyechi on 01/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_GDI_H
#define CX_GDI_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../system/cx_system.h"
#include "../system/cx_matrix4x4.h"
#include "cx_colour.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_GDI_DEBUG                      (CX_DEBUG && 1)
#define CX_GDI_DEBUG_LOG_ENABLED          (CX_DEBUG && 1)

#if CX_GDI_DEBUG
#define cx_gdi_assert_no_errors()         _cx_gdi_assert_no_errors ()
#else
#define cx_gdi_assert_no_errors()
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_GDI_RENDER_STATE_NONE          0x0
#define CX_GDI_RENDER_STATE_CULL          0x1
#define CX_GDI_RENDER_STATE_DEPTH_TEST    0x2
#define CX_GDI_RENDER_STATE_BLEND         0x4

typedef cxu32 cx_gdi_renderstate;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  CX_GDI_BLEND_MODE_INVALID = -1,
  CX_GDI_BLEND_MODE_SRC_ALPHA,
  CX_GDI_BLEND_MODE_ONE_MINUS_SRC_ALPHA,
  CX_NUM_GDI_BLEND_MODES
} cx_gdi_blend_mode;

typedef enum
{
  CX_GDI_TRANSFORM_P,
  CX_GDI_TRANSFORM_MV,
  CX_GDI_TRANSFORM_MVP,
  CX_NUM_GDI_TRANSFORMS,
} cx_gdi_transform;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_gdi_init (void);
bool _cx_gdi_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf32 cx_gdi_get_aspect_ratio (void);
cxf32 cx_gdi_get_screen_width (void);
cxf32 cx_gdi_get_screen_height (void);
void cx_gdi_set_screen_dimensions (cxi32 width, cxi32 height);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_clear (const cx_colour *colour);
void cx_gdi_set_viewport (cxi32 width, cxi32 height);
void cx_gdi_enable_z_write (bool enable);
void cx_gdi_set_blend_mode (cx_gdi_blend_mode src, cx_gdi_blend_mode dest);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_get_renderstate (cx_gdi_renderstate *renderstate);
void cx_gdi_set_renderstate (cx_gdi_renderstate renderstate);
void cx_gdi_unbind_all_buffers (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_set_transform (cx_gdi_transform transform, const cx_mat4x4 *matrix);
void cx_gdi_get_transform (cx_gdi_transform transform, cx_mat4x4 *matrix);
//cx_mat4x4 *cx_gdi_get_transform (cx_gdi_transform transform);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_gdi_print_info (void);
void _cx_gdi_assert_no_errors (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
