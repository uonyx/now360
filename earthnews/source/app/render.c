//
//  ui.c
//  earthnews
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "render.h"
#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_font *s_font [NUM_UI_FONTS];
static cx_texture *s_quadTexture = NULL;
static float s_screenWidth = 0.0f;
static float s_screenHeight = 0.0f;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const cx_font *render_get_ui_font (ui_font_size_t fontSize)
{
  CX_ASSERT ((fontSize > UI_FONT_SIZE_INVALID) && (fontSize < NUM_UI_FONTS));
  
  return s_font [fontSize];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_init (float screenWidth, float screenHeight)
{
  s_screenWidth = screenWidth;
  s_screenHeight = screenHeight;
  
  //const char *g_fontname = "data/fonts/courier_new.ttf";
  const char *fontname = "data/fonts/verdana.ttf";
  
  s_font [UI_FONT_SIZE_10] = cx_font_create (fontname, 10.0f);
  s_font [UI_FONT_SIZE_12] = cx_font_create (fontname, 12.0f);
  s_font [UI_FONT_SIZE_14] = cx_font_create (fontname, 14.0f);
  s_font [UI_FONT_SIZE_18] = cx_font_create (fontname, 18.0f);
  s_font [UI_FONT_SIZE_20] = cx_font_create (fontname, 20.0f);
  s_font [UI_FONT_SIZE_24] = cx_font_create (fontname, 24.0f);
  
  //s_quadTexture = cx_texture_create_from_file ("data/textures/avatar.png");
  s_quadTexture = cx_texture_create_from_file ("data/textures/notallcaps0.png");
  //s_quadTexture = cx_texture_create_from_file ("data/browser_icons/icon066w.png");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_deinit (void)
{
  for (unsigned int i = 0; i < NUM_UI_FONTS; ++i)
  {
    cx_font_destroy (s_font [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_screen_reset (float screenWidth, float screenHeight)
{
  s_screenWidth = screenWidth;
  s_screenHeight = screenHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_fps (void)
{
  static float fps = 0.0f;
  static float totalTime = 0.0f;
  static unsigned int frameCount = 0;

  frameCount++;
  
  float deltaTime = (float) cx_system_time_get_delta_time ();
  
  //CX_DEBUGLOG_CONSOLE (1, "%.2f", deltaTime);
  
  totalTime += deltaTime;
  
  if (totalTime >= 1.0f)
  {
    fps = (float) frameCount / totalTime;
    frameCount = 0;
    totalTime = 0.0f;
  }
  
  fps = 1.0f/ deltaTime;
  
  char fpsStr [32];
  
  cx_sprintf (fpsStr, 32, "%.2f", fps);
  
  cx_font *font = s_font [UI_FONT_SIZE_12];
  
  cx_font_render (font, fpsStr, 2.0f, 12.0f, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_red ());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_test (void)
{
#if 1
  float x1 = 60.0f;
  float y1 = 100.0f;
  float x2 = x1 + (float) s_quadTexture->width;
  float y2 = y1 + (float) s_quadTexture->height;
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, cx_colour_blue (), NULL);
  
  x1 = x2;
  x2 = x1 + (float) s_quadTexture->width;
  
  float u1 = 0.0f;
  float v1 = 0.0f;
  float u2 = 1.0f;
  float v2 = 1.0f;
  
  cx_draw_quad_uv (x1, y1, x2, y2, -0.936f, 0.0f, u1, v1, u2, v2, cx_colour_green (), s_quadTexture);
  
  cx_font *font = s_font [UI_FONT_SIZE_18];
  cx_font_render (font, "Jack and Jill went up the ozone layer", 4.0f, 36.0f, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_green ());
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_ui_element (const ui_element_t *elem)
{
  CX_ASSERT (elem);
  
  float x1 = elem->position.x;
  float y1 = elem->position.y;
  float x2 = x1 + elem->dimension.x;
  float y2 = y1 + elem->dimension.y;
  
  if (elem->bgTexture)
  {
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &elem->bgColour, elem->bgTexture);
  }
  else
  {
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &elem->bgColour, NULL);
  }
  
  if (elem->text && *elem->text)
  {
    float tx = 4.0f;
    float ty = 36.0f;
    
    cx_font_render (s_font[0], elem->text, tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &elem->fgColour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

