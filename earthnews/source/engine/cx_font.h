//
//  cx_font.h
//  earthnews
//
//  Created by Ubaka  Onyechi on 21/03/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#ifndef CX_FONT_H
#define CX_FONT_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_shader.h"
#include "cx_material.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void * cx_font_data;

typedef struct cx_font
{
  cx_font_data fontImpl;
  float size;
  
} cx_font;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_font *cx_font_create (const char *filename, cxf32 fontsize, cx_shader *shader);
void cx_font_destroy (cx_font *font);
void cx_font_render (const cx_font *font, const char *text, cxf32 x, cxf32 y, const cx_colour *colour);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
