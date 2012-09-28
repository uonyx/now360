//
//  cx_draw.h
//
//  Created by Ubaka Onyechi on 08/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_DRAW_H
#define CX_DRAW_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../system/cx_system.h"
#include "cx_colour.h"
#include "cx_texture.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_draw_quad (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 rot, 
                   const cx_colour *colour, const cx_texture *texture);

void cx_draw_quad_uv (cxf32 x1, cxf32 y1, cxf32 x2, cxf32 y2, cxf32 z, cxf32 rot, cxf32 u1, cxf32 v1, cxf32 u2, cxf32 v2, 
                    const cx_colour *colour, const cx_texture *texture);

void cx_draw_points (cxi32 numPoints, const cx_vec4 *pos, const cx_colour *colour, const cx_texture *texture);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
