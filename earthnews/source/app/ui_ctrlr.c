//
//  ui_controller.c
//  earthnews
//
//  Created by Ubaka Onyechi on 27/12/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "ui_ctrlr.h"
#include "browser.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

enum e_uid
{
  UI_ID_TWITTER_VIEW,
  UI_ID_TWITTER_TOGGLE,
  UI_ID_NEWS_BUTTON
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static ui_context_t *s_uicontext = NULL;

#define NEWS_MAX_ENTRIES 7

typedef struct ui_news_t
{
  cx_font *font;
  ui_custom_t *buttons [NEWS_MAX_ENTRIES];
  float opacity;
} ui_news_t;

static ui_news_t s_uinews;

#define TWITTER_MAX_ENTRIES 15
#define TICKER_MAX_ENTRIES 64

#define ANIM_FADE_IN  1
#define ANIM_FADE_OUT 0

typedef struct
{
  float t;
  float duration;
  float now;
  bool on;
  unsigned int type : 1;
} animdata_t;

typedef struct
{
  struct 
  {
    void *data;
    float width;
    float posx;
  } items [TICKER_MAX_ENTRIES];
  int itemCount;
  float width;
} ticker_t;

typedef struct ui_twitter_t 
{
  cx_font *font;
  ui_custom_t *view;
  ui_custom_t *toggle;
  cx_texture *bird;
  ticker_t ticker;
  animdata_t fade;
  
} ui_twitter_t;

static ui_twitter_t s_uitwitter;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_init_create_widgets (void);
static void ui_ctrlr_deinit_destroy_widgets (void);

static void ui_ctrlr_news_create (void);
static void ui_ctrlr_news_button_pressed (ui_custom_t *custom);
static void ui_ctrlr_news_button_render (ui_custom_t *custom);
static void ui_ctrlr_news_populate (feed_news_t *feed);

static void ui_ctrlr_twitter_create (void);
static void ui_ctrlr_twitter_ticker_render (ui_custom_t *custom);
static void ui_ctrlr_twitter_button_render (ui_custom_t *custom);
static void ui_ctrlr_twitter_button_pressed (ui_custom_t *custom);
static void ui_ctrlr_twitter_populate (feed_twitter_t *feed);

static void ui_ctrlr_twitter_ticker_fade (int type, float duration);
static void ui_ctrlr_twitter_ticker_fade_update (float *opacity);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_init (float width, float height)
{
  s_uicontext = ui_init (width, height);
  
  ui_ctrlr_init_create_widgets ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_deinit (void)
{
  ui_ctrlr_deinit_destroy_widgets ();
  
  ui_deinit (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_render (void)
{
  ui_render (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_screen_resize (float width, float height)
{
  ui_canvas_resize (s_uicontext, width, height);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ui_ctrlr_handle_input (const input_touch_event *event)
{
  return ui_input (s_uicontext, event);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_set_news_feed_visible (bool visible)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_set_news_feed (feed_news_t *feed)
{
  CX_ASSERT (feed);
  
  ui_ctrlr_news_populate (feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_set_news_feed_opacity (float opacity)
{
  s_uinews.opacity = opacity;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_init_create_widgets (void)
{
#if 0
  s_uibutton = ui_button_create (s_uicontext, -1);
  s_uibutton->text = "button-1";
  s_uibutton->intr.colour [UI_WIDGET_STATE_NORMAL] = *cx_colour_red ();
  s_uibutton->intr.colour [UI_WIDGET_STATE_HOVER] = *cx_colour_green ();
  s_uibutton->intr.colour [UI_WIDGET_STATE_FOCUS] = *cx_colour_blue ();
  cx_vec2_set (&s_uibutton->intr.position, 0.0f, 0.0f);
  cx_vec2_set (&s_uibutton->intr.dimension, 200.0f, 40.0f);
#endif
  
  ui_ctrlr_news_create ();  
  ui_ctrlr_twitter_create ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_deinit_destroy_widgets (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_create (void)
{
  memset (&s_uinews, 0, sizeof (s_uinews));
  
  const float height = 180.0f;
  const float posX = 12.0f;
  const float posY = 582.0f;
  const float itemSpacingY = height / (float) NEWS_MAX_ENTRIES;
  
  ui_custom_callbacks_t newsCallbacks;
  memset (&newsCallbacks, 0, sizeof (ui_custom_callbacks_t));
  newsCallbacks.pressFn = ui_ctrlr_news_button_pressed;
  newsCallbacks.renderFn = ui_ctrlr_news_button_render;
  
  float x = posX;
  float y = posY;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = ui_custom_create (s_uicontext, UI_ID_NEWS_BUTTON + i);
    ui_custom_set_callbacks (custom, &newsCallbacks);
    
    ui_widget_set_position (custom, x, y);
    ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, cx_colour_white ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, cx_colour_grey ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_blue ());
    
    s_uinews.buttons [i] = custom;
    
    y += itemSpacingY;
  }
  
  s_uinews.opacity = 1.0f;
  s_uinews.font = cx_font_create ("data/fonts/verdana.ttf", 14);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_populate (feed_news_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    int count = NEWS_MAX_ENTRIES - 1;
    
    ui_custom_t **buttons = s_uinews.buttons;
    
    feed_news_item_t *entry = feed->items;
    
    cx_font *font = s_uinews.font;
    
    float fh = cx_font_get_height (font);
    
    int i = 0;
    
    while (entry && count--)
    {
      float fw = cx_font_get_text_width (font, entry->title);
      
      ui_custom_t *custom = buttons [i++];
      
      custom->userdata = entry;
      custom->intr.dimension.x = fw;
      custom->intr.dimension.y = fh;
      
      entry = entry->next;
    }
    
    const char *morenews = "More news...";
    ui_custom_t *custom = buttons [i];
    
    custom->userdata = feed->link;
    custom->intr.dimension.x = cx_font_get_text_width (font, morenews);
    custom->intr.dimension.y = fh;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_button_render (ui_custom_t *custom)
{
  const char *title = NULL;
  
  ui_custom_t *moreNewsButton = s_uinews.buttons [NEWS_MAX_ENTRIES - 1];
  
  if (moreNewsButton == custom)
  {
    title = custom->userdata ? "More news..." : NULL;
  }
  else
  {
    feed_news_item_t *entry = (feed_news_item_t *) custom->userdata;
    
    title = entry ? entry->title : NULL;
  }

  cx_font *font = s_uinews.font;
  CX_ASSERT (font);
  
  float opacity = s_uinews.opacity;
  float x1 = custom->intr.position.x;
  float y1 = custom->intr.position.y;

  ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
  cx_colour colour = custom->intr.colour [wstate];
  colour.a *= opacity;
  
#if 1
  float x2 = x1 + custom->intr.dimension.x;
  float y2 = y1 + custom->intr.dimension.y;
  
  cx_colour colours [3];    
  cx_colour_set (&colours [0], 1.0f, 0.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [1], 0.0f, 1.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [2], 0.0f, 0.0f, 1.0f, 0.45f);
  
  int uid = custom->intr.uid;
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colours [uid % 3], NULL);
#endif
  
  if (title)
  {
    cx_font_render (font, title, x1, y1, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_button_pressed (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  const char *link = NULL, *title = NULL;
  
  ui_custom_t *moreNewsButton = s_uinews.buttons [NEWS_MAX_ENTRIES - 1];
  
  if (moreNewsButton == custom)
  {
    title = "More news...";
    link = (const char *) custom->userdata;
  }
  else
  {
    feed_news_item_t *entry = (feed_news_item_t *) custom->userdata;
    
    title = entry ? entry->title : NULL;
    link = entry ? entry->link : NULL;
  }

  
  if (link && title)
  {
    bool open = browser_open (link, title);
    CX_REFERENCE_UNUSED_VARIABLE (open);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_set_twitter_feed (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  ui_ctrlr_twitter_populate (feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_create (void)
{
#if 0
  const float height = 180.0f;
  const float posX = 12.0f;
  const float posY = 582.0f;
  const float itemSpacingY = height / (float) NEWS_MAX_ENTRIES;
  
  ui_custom_callbacks_t newsCallbacks;
  newsCallbacks.pressFn = ui_ctrlr_news_button_pressed;
  newsCallbacks.renderFn = ui_ctrlr_news_button_render;
  
  float x = posX;
  float y = posY;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = ui_custom_create (s_uicontext, UI_ID_NEWS_BUTTON + i);
    ui_custom_set_callbacks (custom, &newsCallbacks);
    
    ui_widget_set_position (custom, x, y);
    ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, cx_colour_white ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, cx_colour_grey ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_blue ());
    
    s_uinews.buttons [i] = custom;
    
    y += itemSpacingY;
  }
  
#endif
  
  memset (&s_uitwitter, 0, sizeof (s_uitwitter));
  
  s_uitwitter.font = cx_font_create ("data/fonts/verdana.ttf", 14);
  s_uitwitter.bird = cx_texture_create_from_file ("data/icons/twitter-bird-16.png");
  
  ui_custom_callbacks_t twViewCallbacks, twToggleCallbacks;
  
  memset (&twViewCallbacks, 0, sizeof (ui_custom_callbacks_t));
  memset (&twToggleCallbacks, 0, sizeof (ui_custom_callbacks_t));
  
  twViewCallbacks.renderFn = ui_ctrlr_twitter_ticker_render;
  twToggleCallbacks.renderFn = ui_ctrlr_twitter_button_render;
  twToggleCallbacks.pressFn = ui_ctrlr_twitter_button_pressed;
  
  s_uitwitter.toggle = ui_custom_create (s_uicontext, UI_ID_TWITTER_TOGGLE);
  ui_custom_set_callbacks (s_uitwitter.toggle, &twToggleCallbacks);
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_NORMAL, cx_colour_black ());
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_HOVER, cx_colour_grey ());
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_FOCUS, cx_colour_cyan ());
  ui_widget_set_position (s_uitwitter.toggle, 0.0f, 36.0f);
  ui_widget_set_dimension (s_uitwitter.toggle, (float) s_uitwitter.bird->width, (float) s_uitwitter.bird->height);
  ui_widget_set_dimension (s_uitwitter.toggle, 36.0f, 24.0f);
  
  s_uitwitter.view = ui_custom_create (s_uicontext, UI_ID_TWITTER_VIEW);
  ui_custom_set_callbacks (s_uitwitter.view, &twViewCallbacks);
  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_NORMAL, cx_colour_indigo ());
  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_HOVER, cx_colour_yellow ());
  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_FOCUS, cx_colour_orange ());
  ui_widget_set_position (s_uitwitter.view, 0.0f, 36.0f);
  ui_widget_set_dimension (s_uitwitter.view, 1024.0f, 24.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_populate (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    ui_custom_t *custom = s_uitwitter.view;
    
    if (custom->userdata != feed)
    {
      custom->userdata = feed;
      
      ticker_t *ticker = &s_uitwitter.ticker;
      
      memset (ticker, 0, sizeof (ticker_t));
      
      feed_twitter_tweet_t *tweet = feed->items;
      
      float pixelwidthAvgChar = 6.0f;
      float pixelwidthTotal = 0.0f;
      
      while (tweet && (ticker->itemCount < TICKER_MAX_ENTRIES))
      {
        float pixelwidth = 0.0f;
        
        pixelwidth += cx_font_get_text_width (s_uitwitter.font, tweet->userhandle);
        pixelwidth += (pixelwidthAvgChar + pixelwidthAvgChar); // space + @
        
        pixelwidth += cx_font_get_text_width (s_uitwitter.font, tweet->text);
        pixelwidth += (pixelwidthAvgChar + pixelwidthAvgChar); // padding
        
        ticker->items [ticker->itemCount].data = tweet;
        ticker->items [ticker->itemCount].width = pixelwidth;
        ticker->items [ticker->itemCount++].posx = custom->intr.position.x + pixelwidthTotal;
        
        pixelwidthTotal += pixelwidth;
        
        tweet = tweet->next;
      }
      
      ticker->width = pixelwidthTotal;
      
      
      for (int i = 0; i < ticker->itemCount; ++i)
      {
        ticker->items [i].posx += custom->intr.dimension.x;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  ticker_t *ticker = &s_uitwitter.ticker;
  
  if (ticker)
  {
    float opacity = ui_widget_get_opacity (custom);
    ui_ctrlr_twitter_ticker_fade_update (&opacity);
    
    ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
    const cx_colour *colour = ui_widget_get_colour (custom, wstate);
    const cx_vec2 *pos = ui_widget_get_position (custom);
    const cx_vec2 *dim = ui_widget_get_dimension (custom);
    
    cx_colour colbg = *colour;
    cx_colour colname = *cx_colour_green ();
    cx_colour coltweet = *cx_colour_white ();
    
    colbg.a *= opacity;
    colname.a *= opacity;
    coltweet.a *= opacity;
    
    float x1 = pos->x;
    float y1 = pos->y;
    float x2 = x1 + dim->x;
    float y2 = y1 + dim->y;
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colbg, NULL);
    
    cx_font *font = s_uitwitter.font;
    float scrollx = 60.0f * (float) cx_system_time_get_delta_time ();
    
    for (int i = 0, c = ticker->itemCount; i < c; ++i)
    {
      float x = ticker->items [i].posx;
      float y = pos->y;
      
      feed_twitter_tweet_t *tweet = ticker->items [i].data;
      
      if (tweet->userhandle && tweet->text)
      {
        char namehandle [256];
        
        cx_sprintf (namehandle, 256, "@%s: ", tweet->userhandle);
        
        cx_font_render (font, namehandle, x, y, 0.0f, 0, &colname);
        
        float x2 = x + cx_font_get_text_width (font, namehandle);
        
        cx_font_render (font, tweet->text, x2, y, 0.0f, 0, &coltweet);
      }

      if (wstate != UI_WIDGET_STATE_HOVER)
      {
        if ((x + ticker->items [i].width) < 0.0f)
        {
          ticker->items [i].posx = x + ticker->width + custom->intr.dimension.x;
        }
        else
        {
          ticker->items [i].posx = x - scrollx;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
  const cx_colour *colour = ui_widget_get_colour (custom, wstate);
  const cx_vec2 *pos = ui_widget_get_position (custom);
  const cx_vec2 *dim = ui_widget_get_dimension (custom);
  float opacity = ui_widget_get_opacity (custom);
  const cx_texture *image = s_uitwitter.bird;
  CX_ASSERT (image);
  
  // bg
  
  float w = dim->x;
  float h = dim->y;
  
  float x1 = pos->x;
  float y1 = pos->y;
  float x2 = x1 + w;
  float y2 = y1 + h;
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, colour, NULL);
  
  // bird
  
  float bw = (float) s_uitwitter.bird->width;
  float bh = (float) s_uitwitter.bird->height;
  
  float bx1 = x1 + ((w - bw) * 0.5f);
  float by1 = y1 + ((h - bh) * 0.5f);
  float bx2 = bx1 + bw;
  float by2 = by1 + bh;
  
  cx_colour col = custom->userdata ? *cx_colour_grey () : *cx_colour_white ();
  col.a *= opacity;
  
  cx_draw_quad (bx1, by1, bx2, by2, 0.0f, 0.0f, &col, image);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_pressed (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  if (!s_uitwitter.fade.on)
  {
    if (custom->userdata)
    {
      // show twitter
      custom->userdata = NULL;
      ui_ctrlr_twitter_ticker_fade (ANIM_FADE_IN, 1.0f);
    }
    else
    {
      // hide twitter
      custom->userdata = (void *) 0xffff;
      ui_ctrlr_twitter_ticker_fade (ANIM_FADE_OUT, 1.0f);
    }
  }
  
  ui_clear_focus (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade (int type, float duration)
{
  s_uitwitter.fade.type = type;
  s_uitwitter.fade.duration = duration;
  s_uitwitter.fade.t = 0.0f;
  s_uitwitter.fade.now = 0.0f;
  s_uitwitter.fade.on = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade_update (float *opacity)
{
  CX_ASSERT (opacity);
  
  if (s_uitwitter.fade.on)
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    s_uitwitter.fade.now += deltaTime;
    s_uitwitter.fade.t = s_uitwitter.fade.now / s_uitwitter.fade.duration;
    s_uitwitter.fade.t = cx_clamp (s_uitwitter.fade.t, 0.0f, 1.0f);
    
    if (s_uitwitter.fade.now > s_uitwitter.fade.duration)
    {
      s_uitwitter.fade.on = false;
    }
  }
  
  float a = *opacity;
  
  a *= (s_uitwitter.fade.type == ANIM_FADE_IN) ? s_uitwitter.fade.t : (1.0f - s_uitwitter.fade.t);
  
  *opacity = a;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
