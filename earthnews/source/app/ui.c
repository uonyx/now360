//
//  ui.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 09/04/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

#include "ui.h"
#include "../engine/cx_font.h"
#include "../engine/cx_draw.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ui_element_t
{
  cx_vec2 position;
  cx_vec2 dimension;
  cx_colour colour;
  cx_texture *texture;
};

typedef struct ui_element_t ui_element_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_shader *s_fontShader = NULL;
static cx_font *s_font = NULL;
static cx_texture *s_quadTexture = NULL;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_initialise (void)
{
  s_fontShader = cx_shader_create ("font", "data/shaders");
  s_font = cx_font_create ("data/fonts/courier_new.ttf", 24.0f, s_fontShader);
  s_quadTexture = cx_texture_create_from_file ("data/textures/avatar.png");
  
  CX_FATAL_ASSERT (s_fontShader);
  CX_FATAL_ASSERT (s_font);
  CX_FATAL_ASSERT (s_quadTexture);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_deinitialise (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_update (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render (void)
{
  float x1 = 60.0f;
  float y1 = 100.0f;
  float x2 = x1 + 100.0f;
  float y2 = y1 + 100.0f;
  
  cx_draw_quad_colour (x1, y1, x2, y2, cx_colour_blue ());
  
  x1 += 100.0f;
  x2 += 100.0f;
  
  float u1 = 0.0f;
  float v1 = 0.0f;
  float u2 = 1.0f;
  float v2 = 1.0f;
  
  cx_draw_quad_texture (x1, y1, x2, y2, u1, v1, u2, v2, cx_colour_white(), s_quadTexture);
  
  cx_font_render (s_font, "Jack and Jill went up the hill", 4.0f, 36.0f, cx_colour_green ());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
