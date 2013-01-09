//
//  cx_font.h
//
//  Created by Ubaka Onyechi on 21/03/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_FONT_H
#define CX_FONT_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_colour.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_FONT_ALIGNMENT_LEFT_X          0x0
#define CX_FONT_ALIGNMENT_CENTRE_X        0x1
#define CX_FONT_ALIGNMENT_RIGHT_X         0x2
#define CX_FONT_ALIGNMENT_CENTRE_Y        0x4
#define CX_FONT_ALIGNMENT_DEFAULT         CX_FONT_ALIGNMENT_LEFT_X

typedef unsigned int cx_font_alignment;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct cx_font
{
  void *fontdata;
} cx_font;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font *cx_font_create (const char *filename, cxf32 fontsize);
void cx_font_destroy (cx_font *font);
void cx_font_set_scale (const cx_font *font, cxf32 x, cxf32 y);
void cx_font_render (const cx_font *font, const char *text, cxf32 x, cxf32 y, cxf32 z, cx_font_alignment alignment, const cx_colour *colour);
cxi32 cx_font_render_word_wrap (const cx_font *font, const char *text, cxf32 x, cxf32 y, cxf32 bx, cxf32 by, cxf32 z, cx_font_alignment alignment, const cx_colour *colour);
cxf32 cx_font_get_text_width (const cx_font *font, const char *text);
cxf32 cx_font_get_height (const cx_font *font);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
