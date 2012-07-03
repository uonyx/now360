//
//  ui.h
//  earthnews
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef UI_H
#define UI_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_colour.h"
#include "../engine/cx_texture.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct ui_element_t
{
  cx_vec2 position;
  cx_vec2 dimension;
  cx_colour bgColour;
  cx_colour fgColour;
  cx_texture *bgTexture;
  const char *text;
  void *userdata;
  struct ui_element_t *next;
};

struct ui_twitter_element_t
{
  struct ui_element_t *elem;
};

struct ui_list_t
{
  cx_vec2 position;
  cx_vec2 dimension;
  cx_colour bgColour;
  cx_colour fgColour;
  cx_texture *bgTexture;
  
  float interspacing; // vertical list (y)
  struct ui_element_t *child;
};

typedef struct ui_element_t ui_element_t;
typedef struct ui_list_t ui_list_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render_element_twitter_entry (const ui_element_t *elem);
void ui_render_element (const ui_element_t *elem);
void ui_render_list (const ui_list_t *elem);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_init (void);
void ui_deinit (void);

void ui_update (void);
void ui_render (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
