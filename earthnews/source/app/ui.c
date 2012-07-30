//
//  ui.c
//  earthnews
//
//  Created by Ubaka Onyechi on 09/04/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "ui.h"
#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

enum 
{
  UI_FONT_SIZE_10,
  UI_FONT_SIZE_12,
  UI_FONT_SIZE_14,
  UI_FONT_SIZE_18,
  UI_FONT_SIZE_20,
  UI_FONT_SIZE_24,
  NUM_UI_FONTS
};

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

void ui_render_test (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_init (float screenWidth, float screenHeight)
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
  
  s_quadTexture = cx_texture_create_from_file ("data/textures/avatar.png");
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_deinit (void)
{
  for (unsigned int i = 0; i < NUM_UI_FONTS; ++i)
  {
    cx_font_destroy (s_font [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_screen_reset (float screenWidth, float screenHeight)
{
  s_screenWidth = screenWidth;
  s_screenHeight = screenHeight;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render (void)
{
  ui_render_test ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render_test (void)
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
  
  cx_draw_quad_texture2 (x1, y1, x2, y2, u1, v1, u2, v2, cx_colour_white(), s_quadTexture);
  
  cx_font *font = s_font [UI_FONT_SIZE_18];
  cx_font_render (font, "Jack and Jill went up the ozone layer", 4.0f, 36.0f, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_green ());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render_twitter_feed (twitter_feed_t *feed)
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
    colour.a = 0.5f;
    
    cx_draw_quad_colour (x1, y1, x2, y2, &colour);
    
    int max_rpp = 15;
    
    float itemHeight = height / (float) max_rpp;
    (void)itemHeight;
    
    float itemTextSpacingY = 10.0f; //(itemHeight * 0.2f);
    
    float tx = x1 + 10.0f;
    float ty = y1 + 12.0f;
    
    cx_font *font0 = s_font [UI_FONT_SIZE_14];
    cx_font *font1 = s_font [UI_FONT_SIZE_18];
    //cx_font_set_scale (s_fontui, 1.0f, 1.0f);
    
    char name [128];
    twitter_tweet_t *tweet = feed->items;
    
    while (tweet)
    {
      cx_sprintf (name, 128, "%s @%s", tweet->username, tweet->userhandle);
      cx_font_render (font1, name, tx, ty, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_green ());
      
      float tty = ty + itemTextSpacingY;
      
      int lines = cx_font_render_word_wrap (font0, tweet->text, tx, tty, x2, y2, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
      
      float fontHeight = cx_font_get_height (font0) * (lines + 3);
      
      ty += fontHeight;
      
      //ty += itemHeight;
      
      tweet = tweet->next;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_render_rss_feed (news_feed_t *feed)
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
    
    cx_colour_set (&colours [0], 1.0f, 0.0f, 0.0f, 0.8f);
    cx_colour_set (&colours [1], 0.0f, 1.0f, 0.0f, 0.8f);
    cx_colour_set (&colours [2], 0.0f, 0.0f, 1.0f, 0.8f);
    
    float h = cx_font_get_height (font);
    
    while (news && count--)
    {
      float w = cx_font_get_text_width (font, news->title);
      
      cx_draw_quad_colour (tx, ty, tx + w, ty + h, &colours [count % 3]);
      
      cx_font_render (font, news->title, tx, ty, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
      
      ty += itemSpacingY;
      
      news = news->next;
    }
    
    cx_font_render (font, "More news...", tx, ty, CX_FONT_ALIGNMENT_DEFAULT, cx_colour_white ());
  }
}

const char *ui_rss_hit_test (news_feed_t *feed, float inputX, float inputY)
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

void ui_render_element (const ui_element_t *elem)
{
  CX_ASSERT (elem);
  
  float x1 = elem->position.x;
  float y1 = elem->position.y;
  float x2 = x1 + elem->dimension.x;
  float y2 = y1 + elem->dimension.y;
  
  if (elem->bgTexture)
  {
    cx_draw_quad_texture (x1, y1, x2, y2, &elem->bgColour, elem->bgTexture);
  }
  else
  {
    cx_draw_quad_colour (x1, y1, x2, y2, &elem->bgColour);
  }
  
  if (elem->text && *elem->text)
  {
    float tx = 4.0f;
    float ty = 36.0f;
    
    cx_font_render (s_font[0], elem->text, tx, ty, CX_FONT_ALIGNMENT_DEFAULT, &elem->fgColour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

