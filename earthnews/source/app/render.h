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

#include "../engine/cx_engine.h"
#include "feeds.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum 
{
  UI_FONT_SIZE_INVALID = -1,
  UI_FONT_SIZE_10,
  UI_FONT_SIZE_12,
  UI_FONT_SIZE_14,
  UI_FONT_SIZE_18,
  UI_FONT_SIZE_20,
  UI_FONT_SIZE_24,
  NUM_UI_FONTS
} ui_font_size_t;

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

void render_ui_element (const ui_element_t *elem);
void render_ui_list (const ui_list_t *elem);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_init (float screenWidth, float screenHeight);
void render_deinit (void);
void render_screen_reset (float screenWidth, float screenHeight);
void render_test (void);
void render_fps (void);

const cx_font *render_get_ui_font (ui_font_size_t fontSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_twitter_feed (twitter_feed_t *feed);
void render_news_feed (news_feed_t *feed);
void render_weather_feed (weather_feed_t *feed, cx_vec2 *pos, float posz);

const char *render_news_feed_hit_test (news_feed_t *feed, float inputX, float inputY);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
