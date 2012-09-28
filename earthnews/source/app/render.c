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

cx_texture *g_weatherIcons [NUM_WEATHER_CONDITION_CODES];

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
  
  // weather icons
  
  char weatherFilename [512];
  
  for (int i = 0; i < NUM_WEATHER_CONDITION_CODES; ++i)
  {
    cx_sprintf (weatherFilename, 512, "data/weather_icons/a%d.png", i);
    
    const char *f = weatherFilename;
    
    g_weatherIcons [i] = cx_texture_create_from_file (f);
    
    CX_ASSERT (g_weatherIcons [i]);
  }
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

void render_twitter_feed (twitter_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    const float width = 360.0f;
    const float height = s_screenHeight;
    
    float x1 = s_screenWidth - width;
    float y1 = 0.0f;
    float x2 = x1 + width;
    float y2 = y1 + height;
    
    cx_colour colour = *cx_colour_blue ();
    
    colour.r = 0.2f;
    colour.g = 0.2f;
    colour.b = 0.2f;
    colour.a = 0.7f;
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colour, NULL);
    
    int max_rpp = 15;
    
    float itemHeight = height / (float) max_rpp;
    (void)itemHeight;
    
    float itemTextSpacingY = 10.0f; //(itemHeight * 0.2f);
    
    float tx = x1 + 10.0f;
    float ty = y1 + 12.0f;
    
    cx_font *font0 = s_font [UI_FONT_SIZE_14];
    //cx_font *font1 = s_font [UI_FONT_SIZE_18];
    //cx_font_set_scale (s_fontui, 1.0f, 1.0f);
    
    char name [128];
    twitter_tweet_t *tweet = feed->items;
    
    while (tweet)
    {
      cx_sprintf (name, 128, "%s @%s", tweet->username, tweet->userhandle);
      cx_font_render (font0, name, tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_green ());
      
      float tty = ty + itemTextSpacingY;
      
      int lines = cx_font_render_word_wrap (font0, tweet->text, tx, tty, x2 - 24.0f, y2, 0.0f, 
                                            CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
      
      float fontHeight = cx_font_get_height (font0) * (lines + 2);
      
      ty += fontHeight;
      
      //ty += itemHeight;
      
      tweet = tweet->next;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_news_feed (news_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    const cx_font *font = s_font [UI_FONT_SIZE_12];
    
    const int maxNumDisplayItems = 8;
    /* const float width = 400.0f; */
    const float height = 180.0f;
    const float posX = 0.0f;
    const float posY = 588.0f;
    const float marginX = 12.0f;
    const float marginY = 6.0f;
    /* const float textHeight = cx_font_get_height (font); */
    const float itemSpacingY = height / (float) maxNumDisplayItems;
    
    int count = maxNumDisplayItems - 2;
    news_feed_item_t *news = feed->items;
    
    float tx = posX + marginX;
    float ty = posY + marginY;
    
    cx_colour colours [3];
    
    cx_colour_set (&colours [0], 1.0f, 0.0f, 0.0f, 0.45f);
    cx_colour_set (&colours [1], 0.0f, 1.0f, 0.0f, 0.45f);
    cx_colour_set (&colours [2], 0.0f, 0.0f, 1.0f, 0.45f);
    
    float h = cx_font_get_height (font);
    
    while (news && count--)
    {
      float w = cx_font_get_text_width (font, news->title);
      
      cx_draw_quad (tx, ty, tx + w, ty + h, 0.0f, 0.0f, &colours [count % 3], NULL);
      
      cx_font_render (font, news->title, tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
      
      ty += itemSpacingY;
      
      news = news->next;
    }
    
    cx_font_render (font, "More news...", tx, ty, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void render_weather_feed (weather_feed_t *feed, cx_vec2 *pos, float posz)
{
  CX_ASSERT (feed);
  CX_ASSERT (pos);
  
  if (feed->dataReady && 
      (feed->conditionCode > WEATHER_CONDITION_CODE_INVALID) && 
      (feed->conditionCode < NUM_WEATHER_CONDITION_CODES))
  {
    cx_texture *image = g_weatherIcons [feed->conditionCode];
    CX_ASSERT (image);
    
    float w = (float) image->width;
    float h = (float) image->height;
    
    float x1 = pos->x - (w * 0.5f);
    float y1 = pos->y - (h * 0.5f);
    float x2 = x1 + w;
    float y2 = y1 + h;
    
    cx_draw_quad (x1, y1, x2, y2, posz, 0.0f, cx_colour_white (), image);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *render_news_feed_hit_test (news_feed_t *feed, float inputX, float inputY)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    const int maxNumDisplayItems = 8;
    /* const float width = 400.0f; */
    const float height = 180.0f;
    const float posX = 0.0f;
    const float posY = 588.0f;
    const float marginX = 12.0f;
    const float marginY = 6.0f;
    const float itemSpacingY = height / (float) maxNumDisplayItems;
    
    int count = maxNumDisplayItems - 2;
    news_feed_item_t *news = feed->items;
    
    float tx = posX + marginX;
    float ty = posY + marginY;
    
    const cx_font *font = s_font [UI_FONT_SIZE_12];
    
    while (news && count--)
    {
      float w = cx_font_get_text_width (font, news->title);
      
      if ((inputX >= tx) && (inputX <= (tx + w)))
      {
        if ((inputY >= ty) && (inputY <= (ty + itemSpacingY)))
        {
          return news->link;
        }
      }
      
      ty += itemSpacingY;
      
      news = news->next;
    }
    
    const char *moreNews = "More news...";
    
    float w = cx_font_get_text_width (font, moreNews);
    
    if ((inputX >= tx) && (inputX <= (tx + w)))
    {
      if ((inputY >= ty) && (inputY <= (ty + itemSpacingY)))
      {
        return feed->link;
      }
    }
  }
  
  return NULL;
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

