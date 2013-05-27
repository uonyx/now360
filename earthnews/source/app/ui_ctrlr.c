//
//  ui_controller.c
//  earthnews
//
//  Created by Ubaka Onyechi on 27/12/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "ui_ctrlr.h"
#include "util.h"
#include "webview.h"
#include "audio.h"
#include "settings.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UI_CTRLR_DEBUG 0
#define UI_CTRLR_DEBUG_NEW_TICKER 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

enum e_uid
{
  UI_ID_TWITTER_VIEW,
  UI_ID_TWITTER_TOGGLE,
  UI_ID_SETTINGS_BUTTON,
  UI_ID_MUSIC_TOGGLE,
  UI_ID_MUSIC_VIEW,
  UI_ID_MUSIC_QUEUE,
  UI_ID_MUSIC_PLAY,
  UI_ID_MUSIC_NEXT,
  UI_ID_MUSIC_PREV,
  UI_ID_NEWS_BUTTON, // THIS HAS TO BE LAST (not elegant, I know)
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static ui_context_t *s_uicontext = NULL;

#define NEWS_MAX_ENTRIES  5
#define NEWS_LIST_HEIGHT  120.0f
#define NEWS_LIST_XOFFSET 36.0f //12.0f

const float height = 180.0f;
const float posX = 12.0f;
const float posY = 582.0f;

typedef struct ui_news_t
{
  ui_custom_t *buttons [NEWS_MAX_ENTRIES];
  float opacity;
} ui_news_t;

static ui_news_t s_uinews;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

#define TWITTER_MAX_ENTRIES                 (15)
#define TWITTER_MAX_TWEET_LEN               FEED_TWITTER_TWEET_MESSAGE_MAX_LEN
#define TWITTER_TWEET_VIS_LINK_URL_BUF_SIZE (64)
#define TWITTER_TWEET_VIS_LINK_MAX_COUNT    (8)
#define TWITTER_TICKER_MAX_ITEM_COUNT       (64)
#define TWITTER_TICKER_TWEET_MAX_ENTRIES    (16)
#define TWITTER_TICKER_TWEET_ELEM_TYPE_TEXT (0)
#define TWITTER_TICKER_TWEET_ELEM_TYPE_LINK (1)

typedef struct ticker_tweet_t
{
  struct 
  {
    cxu32 loc; //  start pos
    cxu32 charCount;
    cxu32 type; // text or link
  } elems [TWITTER_TICKER_TWEET_MAX_ENTRIES];
  
  char buffer [TWITTER_MAX_TWEET_LEN]; // max tweet - 140 chars
  cxu32 elemCount;
} ticker_tweet_t;

typedef struct
{
  struct 
  {
    void *data;
    float width;
    float posx;
  } items [TWITTER_TICKER_MAX_ITEM_COUNT];
  int itemCount;
  float width;
} ticker_t;

typedef struct ui_twitter_t 
{
  ui_custom_t *view;
  ui_custom_t *toggle;
  cx_texture *birdicon;
  ticker_t ticker;
  animdata_t fade;
  
} ui_twitter_t;

typedef struct
{
  char url [TWITTER_TWEET_VIS_LINK_URL_BUF_SIZE];
  float x, w;
} tweet_visible_link_t;

static tweet_visible_link_t s_visLinks [TWITTER_TWEET_VIS_LINK_MAX_COUNT];
static unsigned int s_visLinkCount = 0;
static ui_twitter_t s_uitwitter;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ui_music_t
{
  ui_custom_t *view;
  ui_custom_t *toggle;
  ui_custom_t *queue;
  ui_custom_t *play; // pause
  ui_custom_t *prev;
  ui_custom_t *next;
  
  cx_texture *iconNote;
  cx_texture *iconPlay;
  cx_texture *iconPause;
  cx_texture *iconPrev;
  cx_texture *iconQueue;
  
  // queue
  // toggle
  // pause
  // next
  // prev
  animdata_t fade;
} ui_music_t;

static ui_music_t s_uimusic;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ui_settings_t
{
  ui_custom_t *button;
  cx_texture *icon;
} ui_settings_t;

static ui_settings_t s_uisettings;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_init_create_widgets (void);
static void ui_ctrlr_deinit_destroy_widgets (void);
static void ui_ctrlr_screen_resize_widgets (void);

static void ui_ctrlr_news_create (void);
static void ui_ctrlr_news_destroy (void);
static void ui_ctrlr_news_position_setup (void);
static void ui_ctrlr_news_button_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_news_button_render (ui_custom_t *custom);
static void ui_ctrlr_news_populate (feed_news_t *feed);

static void ui_ctrlr_twitter_create (void);
static void ui_ctrlr_twitter_destroy (void);
static void ui_ctrlr_twitter_position_setup (void);
static void ui_ctrlr_twitter_ticker_render (ui_custom_t *custom);
static void ui_ctrlr_twitter_button_render (ui_custom_t *custom);
static void ui_ctrlr_twitter_ticker_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_twitter_button_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_twitter_populate (feed_twitter_t *feed);
static void ui_ctrlr_twitter_ticker_fade_begin (int type, float duration);
static void ui_ctrlr_twitter_ticker_fade_end (void);
static void ui_ctrlr_twitter_ticker_anim_update (void);

static void ui_ctrlr_music_create (void);
static void ui_ctrlr_music_destroy (void);
static void ui_ctrlr_music_position_setup (void);
static void ui_ctrlr_music_view_render (ui_custom_t *custom);
static void ui_ctrlr_music_view_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_toggle_render (ui_custom_t *custom);
static void ui_ctrlr_music_toggle_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_play_render (ui_custom_t *custom);
static void ui_ctrlr_music_play_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_queue_render (ui_custom_t *custom);
static void ui_ctrlr_music_queue_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_prev_render (ui_custom_t *custom);
static void ui_ctrlr_music_prev_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_next_render (ui_custom_t *custom);
static void ui_ctrlr_music_next_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_music_bar_fade_begin (int type, float duration);
static void ui_ctrlr_music_bar_fade_end (void);
static void ui_ctrlr_music_bar_anim_update (void);
static void ui_ctrlr_music_picked_callback (void);
static void ui_ctrlr_music_notification_callback (audio_music_notification_t n);

static void ui_ctrlr_settings_create (void);
static void ui_ctrlr_settings_destroy (void);
static void ui_ctrlr_settings_position_setup (void);
static void ui_ctrlr_settings_button_pressed (ui_custom_t *custom, const cx_vec2 *point);
static void ui_ctrlr_settings_button_render (ui_custom_t *custom);

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
  ui_ctrlr_twitter_ticker_anim_update ();
  
  ui_ctrlr_music_bar_anim_update ();
  
  ui_render (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_screen_resize (float width, float height)
{
  ui_canvas_resize (s_uicontext, width, height);
  
  ui_ctrlr_screen_resize_widgets ();
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

bool ui_ctrlr_settings_get_active (void)
{
  return settings_ui_active ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_settings_set_active (bool active)
{
  if (active)
  {
    settings_ui_show ();
    s_uisettings.button->userdata = NULL;
  }
  else
  {
    settings_ui_hide ();
    s_uisettings.button->userdata = (void *) 0xffff;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_init_create_widgets (void)
{
  ui_ctrlr_news_create ();  
  ui_ctrlr_twitter_create ();
  ui_ctrlr_music_create ();
  ui_ctrlr_settings_create ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_deinit_destroy_widgets (void)
{
  ui_ctrlr_news_destroy ();
  ui_ctrlr_twitter_destroy ();
  ui_ctrlr_music_destroy ();
  ui_ctrlr_settings_destroy ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_screen_resize_widgets (void)
{
  ui_ctrlr_news_position_setup ();
  ui_ctrlr_twitter_position_setup ();
  ui_ctrlr_music_position_setup ();
  ui_ctrlr_settings_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_button_render_with_image (ui_custom_t *custom, cx_colour* col1, cx_colour *col2, 
                                                cx_texture *image, bool hflip, bool toggle)
{
  CX_ASSERT (custom);
  CX_ASSERT (col1);
  CX_ASSERT (col2);
  CX_ASSERT (image);
  
  ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
  const cx_colour *colour = ui_widget_get_colour (custom, wstate);
  const cx_vec2 *pos = ui_widget_get_position (custom);
  const cx_vec2 *dim = ui_widget_get_dimension (custom);
  float opacity = ui_widget_get_opacity (custom);
  
  // bg
  
  float w = dim->x;
  float h = dim->y;
  
  float x1 = pos->x;
  float y1 = pos->y;
  float x2 = x1 + w;
  float y2 = y1 + h;
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, colour, NULL);
  
  // image
  
  float bw = (float) image->width;
  float bh = (float) image->height;
  
  float bx1 = x1 + ((w - bw) * 0.5f);
  float by1 = y1 + ((h - bh) * 0.5f);
  float bx2 = bx1 + bw;
  float by2 = by1 + bh;
  
  cx_colour *col = NULL;
  
  if (toggle)
  {  
    col = custom->userdata ? col2 : col1;
  }
  else
  {
    col = (wstate == UI_WIDGET_STATE_HOVER) ? col2 : col1;
  }
  
  col->a *= opacity;
  
  if (hflip)
  {
    float u1 = 1.0f;
    float v1 = 0.0f;
    float u2 = 0.0f;
    float v2 = 1.0f;
    
    cx_draw_quad_uv (bx1, by1, bx2, by2, 0.0f, 0.0f, u1, v1, u2, v2, col, image);
  }
  else
  {
    cx_draw_quad (bx1, by1, bx2, by2, 0.0f, 0.0f, col, image);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_create (void)
{
  memset (&s_uinews, 0, sizeof (s_uinews));
  
  s_uinews.opacity = 1.0f;
  
  ui_custom_callbacks_t newsCallbacks;
  memset (&newsCallbacks, 0, sizeof (ui_custom_callbacks_t));
  newsCallbacks.pressFn = ui_ctrlr_news_button_pressed;
  newsCallbacks.renderFn = ui_ctrlr_news_button_render;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = ui_custom_create (s_uicontext, UI_ID_NEWS_BUTTON + i);
    
    ui_custom_set_callbacks (custom, &newsCallbacks);
    ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, cx_colour_white ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, cx_colour_grey ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_white ());
    
    s_uinews.buttons [i] = custom;
  }
  
  ui_ctrlr_news_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_destroy (void)
{
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_destroy (s_uicontext, s_uinews.buttons [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_position_setup (void)
{
  //const float botMargin = 12.0f;
  const float botMargin = 12.0f + 36.0f;
  const float height = NEWS_LIST_HEIGHT;
  const float posX = NEWS_LIST_XOFFSET;
  const float posY = s_uicontext->canvasHeight - height - botMargin;
  const float itemSpacingY = height / (float) NEWS_MAX_ENTRIES;
  
  float x = posX;
  float y = posY;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = s_uinews.buttons [i];
    
    ui_widget_set_position (custom, x, y);
    
    y += itemSpacingY;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 1
static int cmp_feed_news_item (feed_news_item_t *item0, feed_news_item_t *item1)
{
  CX_ASSERT (item0);
  CX_ASSERT (item1);
  
  //  1: 0 > 1
  //  0: 0 = 1
  // -1: 0 < 1
  
  if (item0->pubDateInfo.mon == item1->pubDateInfo.mon)
  {
    if (item0->pubDateInfo.mday == item1->pubDateInfo.mday)
    {
      if (item0->pubDateInfo.secs == item1->pubDateInfo.secs)
      {
        return 0;
      }
      else 
      {
        return (item0->pubDateInfo.secs > item1->pubDateInfo.secs) ? 1 : -1;
      }
    }
    else
    {
      return (item0->pubDateInfo.mday > item1->pubDateInfo.mday) ? 1 : -1;
    }
  }
  else
  {
    return (item0->pubDateInfo.mon > item1->pubDateInfo.mon) ? 1 : -1;
  }
}

static void ui_ctrlr_news_populate (feed_news_t *feed)
{
  CX_ASSERT (feed);
  CX_ASSERT (feed->reqStatus != FEED_REQ_STATUS_IN_PROGRESS);
    
  feed_news_item_t *entry = feed->items;
  
  // to array
  feed_news_item_t *entryArray [32] = {0};
  
  int entryCount = 0;
  
  while (entry && (entryCount < 32)) 
  {
    entryArray [entryCount++] = entry;
    
    entry = entry->next;
  }
  
  // (selection) sort array according to entry pubDate
  
  for (int si = 0; si < (entryCount - 1); ++si)
  {
    int maxIdx = si;
    
    feed_news_item_t *maxEntry = entryArray [maxIdx];
    
    for (int sj = si + 1; sj < entryCount; ++sj)
    {
      feed_news_item_t *sjEntry = entryArray [sj];
      
      if (cmp_feed_news_item (sjEntry, maxEntry) == 1)
      {
        maxIdx = sj;
        maxEntry = sjEntry;
      }
    }
    
    feed_news_item_t *tmpEntry = entryArray [maxIdx];
    entryArray [maxIdx] = entryArray [si];
    entryArray [si] = tmpEntry;
  }
  
  
  const cx_font *font = util_get_font (FONT_SIZE_18);
  
  float fh = 24.0f; //cx_font_get_height (font);
  
  int displayCount = NEWS_MAX_ENTRIES - 1;
  
  ui_custom_t **buttons = s_uinews.buttons;
  
  CX_ASSERT (displayCount < entryCount);
  
  int i = 0, c = displayCount;
  
  for (i = 0; i < c; ++i)
  {
    feed_news_item_t *e = entryArray [i];
    
    float fw = cx_font_get_text_width (font, e->title);
    
    ui_custom_t *custom = buttons [i];
    
    custom->userdata = e;
    
    ui_widget_set_dimension (custom, fw, fh);
  }
  
  const char *morenews = "More news...";
  ui_custom_t *custom = buttons [c];
  
  custom->userdata = feed->link;
  
  ui_widget_set_dimension (custom, cx_font_get_text_width (font, morenews), fh);
}

#else
static void ui_ctrlr_news_populate (feed_news_t *feed)
{
  CX_ASSERT (feed);
  CX_ASSERT (feed->reqStatus != FEED_REQ_STATUS_IN_PROGRESS);

  int count = NEWS_MAX_ENTRIES - 1;
  
  ui_custom_t **buttons = s_uinews.buttons;
  
  feed_news_item_t *entry = feed->items;
  
  const cx_font *font = util_get_font (FONT_SIZE_18);
  
  float fh = 24.0f; //cx_font_get_height (font);
  
  int i = 0;
  
  while (entry && count--)
  {
    float fw = cx_font_get_text_width (font, entry->title);
    
    ui_custom_t *custom = buttons [i++];
    
    custom->userdata = entry;
    ui_widget_set_dimension (custom, fw, fh);
    
    entry = entry->next;
  }
  
  const char *morenews = "More news...";
  ui_custom_t *custom = buttons [i];
  
  custom->userdata = feed->link;    
  ui_widget_set_dimension (custom, cx_font_get_text_width (font, morenews), fh);
}
#endif

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

  const cx_font *font = util_get_font (FONT_SIZE_18);
  CX_ASSERT (font);
  
  float opacity = s_uinews.opacity;
  float x1 = custom->intr.position.x;
  float y1 = custom->intr.position.y;

  ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
  cx_colour colour = *cx_colour_white (); //custom->intr.colour [wstate];
  colour.a *= opacity;
  
#if UI_CTRLR_DEBUG
  float x2 = x1 + custom->intr.dimension.x;
  float y2 = y1 + custom->intr.dimension.y;
  
  cx_colour colours [3];    
  cx_colour_set (&colours [0], 1.0f, 0.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [1], 0.0f, 1.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [2], 0.0f, 0.0f, 1.0f, 0.45f);
  
  int uid = custom->intr.uid;
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colours [uid % 3], NULL);
#endif
  
  if (wstate == UI_WIDGET_STATE_HOVER)
  {
    float x2 = x1 + custom->intr.dimension.x;
    float y2 = y1 + custom->intr.dimension.y;
    
    cx_colour colHlght;
    cx_colour_set (&colHlght, 0.2f, 0.2f, 0.2f, 0.7f);
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colHlght, NULL);
  }
  
  if (title)
  {
    char titleText [512] = {0};
    
#if CX_DEBUG
    unsigned int titleLen = strlen (title);
    CX_ASSERT (titleLen < 512);
#endif
    
    cx_str_html_unescape (titleText, 512, title);
    cx_font_render (font, titleText, x1, y1, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &colour);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
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
  
  if (link)
  {
    webview_show (link, title);
  }
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
  
  ui_clear_focus (s_uicontext);
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
  memset (&s_uitwitter, 0, sizeof (s_uitwitter));
  
  s_uitwitter.birdicon = cx_texture_create_from_file ("data/icons/twbird-16z.png");
  
  ui_custom_callbacks_t twViewCallbacks, twToggleCallbacks;
  
  memset (&twViewCallbacks, 0, sizeof (ui_custom_callbacks_t));
  memset (&twToggleCallbacks, 0, sizeof (ui_custom_callbacks_t));
  
  twViewCallbacks.renderFn = ui_ctrlr_twitter_ticker_render;
  twViewCallbacks.pressFn = ui_ctrlr_twitter_ticker_pressed;
  twToggleCallbacks.renderFn = ui_ctrlr_twitter_button_render;
  twToggleCallbacks.pressFn = ui_ctrlr_twitter_button_pressed;
  
  s_uitwitter.toggle = ui_custom_create (s_uicontext, UI_ID_TWITTER_TOGGLE);
  ui_custom_set_callbacks (s_uitwitter.toggle, &twToggleCallbacks);
  
  cx_colour darkgrey;
  cx_colour_set (&darkgrey, 0.2f, 0.2f, 0.2f, 1.0f);
  
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_NORMAL, cx_colour_black ());
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_HOVER, &darkgrey);
  ui_widget_set_colour (s_uitwitter.toggle, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  
  cx_colour viewCol;
  cx_colour_set (&viewCol, 0.3f, 0.3f, 0.3f, 0.4f);
  
  s_uitwitter.view = ui_custom_create (s_uicontext, UI_ID_TWITTER_VIEW);
  ui_custom_set_callbacks (s_uitwitter.view, &twViewCallbacks);

  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_NORMAL, &viewCol);
  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uitwitter.view, UI_WIDGET_STATE_FOCUS, &viewCol);
  
  ui_ctrlr_twitter_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_destroy (void)
{
  cx_texture_destroy (s_uitwitter.birdicon);
  ui_custom_destroy (s_uicontext, s_uitwitter.view);
  ui_custom_destroy (s_uicontext, s_uitwitter.toggle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_position_setup (void)
{
  //float posY = 72.0f - 30.0f;
  float posY = s_uicontext->canvasHeight - 36.0f;
  
  ui_widget_set_position (s_uitwitter.toggle, 0.0f, posY);
  ui_widget_set_dimension (s_uitwitter.toggle, 36.0f, 24.0f);
  ui_widget_set_visible (s_uitwitter.toggle, false);
  
  ui_widget_set_position (s_uitwitter.view, 0.0f, posY);
  ui_widget_set_dimension (s_uitwitter.view, s_uicontext->canvasWidth, 24.0f);
  ui_widget_set_visible (s_uitwitter.view, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_populate (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  CX_ASSERT (feed->reqStatus != FEED_REQ_STATUS_IN_PROGRESS);
  
  ui_custom_t *custom = s_uitwitter.view;
  CX_ASSERT (custom);
  
  ticker_t *ticker = &s_uitwitter.ticker;
  memset (ticker, 0, sizeof (ticker_t));
  
  feed_twitter_tweet_t *tweet = feed->items;
  
  float pixelwidthAvgChar = 6.0f;
  float pixelwidthTotal = 0.0f;
  
  const cx_font *font = util_get_font (FONT_SIZE_16);
  
  while (tweet && (ticker->itemCount < TWITTER_TICKER_MAX_ITEM_COUNT))
  {
    float pixelwidth = 0.0f;
    
    pixelwidth += cx_font_get_text_width (font, tweet->username);
    pixelwidth += (pixelwidthAvgChar + pixelwidthAvgChar); // space + @
    
    pixelwidth += cx_font_get_text_width (font, tweet->text);
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
  
  ui_widget_set_visible (s_uitwitter.view, true);
  ui_widget_set_visible (s_uitwitter.toggle, true);
  
  if (!s_uitwitter.toggle->userdata)
  {
    ui_ctrlr_twitter_ticker_fade_begin (ANIM_FADE_IN, 0.3f);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
#if UI_CTRLR_DEBUG_NEW_TICKER
static void ui_ctrlr_twitter_get_ticker_tweet (ticker_tweet_t *dest, const char *tweet)
{
  CX_ASSERT (tweet);
  
  cx_strcpy (dest->buffer, TWITTER_MAX_TWEET_LEN, tweet);
  
  const char *start = dest->buffer;
  const char *curr = dest->buffer;
  

  cxu32 i = 0;
  cxu32 currLoc = 0;
  
  dest->elems [i].loc = currLoc;
  dest->elems [i++].type = TWITTER_TICKER_TWEET_ELEM_TYPE_TEXT;
  
  const char *http = strstr (curr, "http://"); 
  
  if (http == NULL)
  {
    http = strstr (curr, "https://");
  }
  
  while (http)
  {
    cxu32 p0 = http - start;
    
    CX_ASSERT (i > 0);
    
    if (p0 == dest->elems [i-1].loc)
    {
      i = i -1;
    }

    dest->elems [i].loc = p0;
    dest->elems [i++].type = TWITTER_TICKER_TWEET_ELEM_TYPE_LINK;
    
    char c = 0;
    char *t = (char *) http;
    
    while ((c = *t++))
    {
      if ((c == ' ') || (c == '#') || (c == '@') || (c > 127))
      {
        cxu32 p = t - start;
        
        dest->elems [i].loc = p;
        dest->elems [i++].type = TWITTER_TICKER_TWEET_ELEM_TYPE_TEXT;
        
        currLoc = p;
        
        if (c != ' ')
        {
          --t;
        }
        
        break;
      }
    }
    
    curr = t;
    
    http = strstr (curr, "http://");
    
    if (http == NULL)
    {
      http = strstr (curr, "https://");
    }
  }
  
  dest->elemCount = i;

  for (cxu32 j = 0; j < dest->elemCount; ++j)
  {
    cxu32 loc = dest->elems [j].loc;
    
    if (loc > 0)
    {
      dest->buffer [loc - 1] = 0;
    }
  }
  
#if 0
  for (cxu32 j = 0; j < dest->elemCount; ++j)
  {
    cxu32 loc = dest->elems [j].loc;
    
    const char *str = &dest->buffer [loc];
    
    CX_DEBUGLOG_CONSOLE (1, "%s", str);
    
    CX_DEBUG_BREAKABLE_EXPR;
  }
#endif
  
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);

  memset (s_visLinks, 0, sizeof (s_visLinks));
  s_visLinkCount = 0;  
  
  ticker_t *ticker = &s_uitwitter.ticker;
  
  if (ticker)
  {
    ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
    const cx_colour *colour = ui_widget_get_colour (custom, wstate);
    const cx_vec2 *pos = ui_widget_get_position (custom);
    const cx_vec2 *dim = ui_widget_get_dimension (custom);
    float opacity = ui_widget_get_opacity (custom);
    
    cx_colour colbg = *colour;
    cx_colour colname = *cx_colour_grey ();
    cx_colour coltweet = *cx_colour_white ();
    cx_colour collink = *cx_colour_cyan ();
    
    colbg.a *= opacity;
    colname.a *= opacity;
    coltweet.a *= opacity;
    collink.a *= opacity;
    
    float x1 = pos->x;
    float y1 = pos->y;
    float x2 = x1 + dim->x;
    float y2 = y1 + dim->y;
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colbg, NULL);
    
    const cx_font *font = util_get_font (FONT_SIZE_16);
    float scrollx = 60.0f * (float) cx_system_time_get_delta_time ();
    char unescapedText [256] = {0};
    
    for (int i = 0, c = ticker->itemCount; i < c; ++i)
    {
      CX_ASSERT (i < TWITTER_TICKER_MAX_ITEM_COUNT);
      
      float ix = x1 + ticker->items [i].posx;
      float iy = pos->y;
      float ix2 = ix + ticker->items [i].width;
      
      feed_twitter_tweet_t *tweet = ticker->items [i].data;
      CX_ASSERT (tweet);
    
      bool outOfBounds = (((ix <= 0.0f) && (ix2 <= 0.0f)) ||
                          ((ix >= s_uicontext->canvasWidth) && (ix2 >= s_uicontext->canvasWidth)));
    
      if (!outOfBounds)
      {
#if UI_CTRLR_DEBUG_NEW_TICKER
      if (tweet->username && tweet->text)
      {
        ticker_tweet_t tickerTweet;
        memset (&tickerTweet, 0, sizeof (ticker_tweet_t));
        
        cx_str_html_unescape (unescapedText, 256, tweet->text);
        ui_ctrlr_twitter_get_ticker_tweet (&tickerTweet, unescapedText);
        
        char username [32];
        cx_sprintf (username, 32, "@%s: ", tweet->username);
        cx_font_render (font, username, ix, iy, 0.0f, 0, &colname);
        
        float rx = ix + cx_font_get_text_width (font, username);
        
        char tickerTweetText [256];
        for (cxu32 j = 0; j < tickerTweet.elemCount; ++j)
        {
          cxu32 loc = tickerTweet.elems [j].loc;
          const char *s = &tickerTweet.buffer [loc];
          cx_sprintf (tickerTweetText, 256, "%s ", s);
          
          bool isLink = (tickerTweet.elems [j].type == TWITTER_TICKER_TWEET_ELEM_TYPE_LINK);
      
          const cx_colour *col = isLink ? &collink : &coltweet;
          
          cx_font_render (font, tickerTweetText, rx, iy, 0.0f, 0, col);
          
          // collect visible link elems for ui input check - funky hack works!
          if (isLink && (s_visLinkCount < TWITTER_TWEET_VIS_LINK_MAX_COUNT))
          {
            //if ((rx >= 0.0f) && (rx <= s_uicontext->canvasWidth))
            {
              unsigned int idx = s_visLinkCount++;
              CX_ASSERT (idx < TWITTER_TWEET_VIS_LINK_MAX_COUNT);
              
              s_visLinks [idx].x = rx;
              s_visLinks [idx].w = cx_font_get_text_width (font, s);
              cx_strcpy (s_visLinks [idx].url, TWITTER_TWEET_VIS_LINK_URL_BUF_SIZE, s);
            }
          }
          
          rx = rx + cx_font_get_text_width (font, tickerTweetText);
        }
      }
#else
      if (tweet->username && tweet->text)
      {
        char namehandle [32];
        cx_sprintf (namehandle, 32, "@%s: ", tweet->username);
        cx_font_render (font, namehandle, x, y, 0.0f, 0, &colname);
        
        float x2 = x + cx_font_get_text_width (font, namehandle);
        
        char tweetText [256] = {0};
        cx_str_html_unescape (tweetText, 256, tweet->text);
        cx_font_render (font, tweetText, x2, y, 0.0f, 0, &coltweet);
      }
#endif
      }
      
      bool activeSystemUI = webview_active () || audio_music_picker_active () || settings_ui_active ();
      
      if ((wstate != UI_WIDGET_STATE_HOVER) && !activeSystemUI)
      {
        if ((ix + ticker->items [i].width) < 0.0f)
        {
          ticker->items [i].posx = ix + ticker->width + custom->intr.dimension.x;
        }
        else
        {
          ticker->items [i].posx = ix - scrollx;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  // get x,y press position
  // get positions of all currently visible ticker_tweets
  // find visible link element
  // go to browser
  
  CX_ASSERT (point);
  
  float touchX = point->x;
  
  for (unsigned int i = 0; i < s_visLinkCount; ++i) 
  {
    float x1 = s_visLinks [i].x;
    float x2 = s_visLinks [i].w + x1;
    
    if ((touchX > x1) && (touchX < x2))
    {
      const char *url = s_visLinks [i].url;
      webview_show (url, NULL);
      
      audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
    }
  }
  
  ui_clear_focus (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uitwitter.birdicon;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  CX_ASSERT (custom);
  
  if (!s_uitwitter.fade.on)
  {
    if (custom->userdata)
    {
      // show 
      custom->userdata = NULL;
      ui_ctrlr_twitter_ticker_fade_begin (ANIM_FADE_IN, 0.3f);
    }
    else
    {
      // hide
      custom->userdata = (void *) 0xffff;
      ui_ctrlr_twitter_ticker_fade_begin (ANIM_FADE_OUT, 0.3f);
    }
  }
  
  ui_clear_focus (s_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade_begin (int type, float duration)
{
  s_uitwitter.fade.type = type;
  s_uitwitter.fade.duration = duration;
  s_uitwitter.fade.t = 0.0f;
  s_uitwitter.fade.now = 0.0f;
  s_uitwitter.fade.on = true;

  ui_widget_set_enabled (s_uitwitter.view, false);
  ui_widget_set_visible (s_uitwitter.view, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade_end (void)
{
  s_uitwitter.fade.on = false;
  
  bool b = (s_uitwitter.fade.type == ANIM_FADE_IN);
  
  ui_widget_set_enabled (s_uitwitter.view, b);
  ui_widget_set_visible (s_uitwitter.view, b);
 }

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_anim_update (void)
{
  if (s_uitwitter.fade.on)
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    s_uitwitter.fade.now += deltaTime;
    s_uitwitter.fade.t = s_uitwitter.fade.now / s_uitwitter.fade.duration;
    s_uitwitter.fade.t = cx_clamp (s_uitwitter.fade.t, 0.0f, 1.0f);
    
    float opacity = (s_uitwitter.fade.type == ANIM_FADE_IN) ? s_uitwitter.fade.t : (1.0f - s_uitwitter.fade.t);
    
    ui_widget_set_opacity (s_uitwitter.view, opacity);
    
    if (s_uitwitter.fade.now > s_uitwitter.fade.duration)
    {
      ui_ctrlr_twitter_ticker_fade_end ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_create (void)
{
  memset (&s_uimusic, 0, sizeof (s_uimusic));
    
  s_uimusic.iconNote = cx_texture_create_from_file ("data/icons/mnote-16z.png");
  s_uimusic.iconPlay = cx_texture_create_from_file ("data/icons/play-12z.png");
  s_uimusic.iconPause = cx_texture_create_from_file ("data/icons/pause-12z.png");
  s_uimusic.iconPrev = cx_texture_create_from_file ("data/icons/prev-12z.png");
  s_uimusic.iconQueue = cx_texture_create_from_file ("data/icons/eject-12z.png");
  
  audio_music_notification_register (ui_ctrlr_music_notification_callback);
  
  ui_custom_callbacks_t cbtoggle, cbplay, cbview, cbqueue, cbprev, cbnext;
  
  memset (&cbtoggle, 0, sizeof (ui_custom_callbacks_t));
  memset (&cbplay, 0, sizeof (ui_custom_callbacks_t));
  memset (&cbview, 0, sizeof (ui_custom_callbacks_t));
  memset (&cbqueue, 0, sizeof (ui_custom_callbacks_t));
  memset (&cbprev, 0, sizeof (ui_custom_callbacks_t));
  memset (&cbnext, 0, sizeof (ui_custom_callbacks_t));

  cbtoggle.renderFn = ui_ctrlr_music_toggle_render;
  cbtoggle.pressFn = ui_ctrlr_music_toggle_pressed;
  
  cbview.renderFn = ui_ctrlr_music_view_render;
  cbview.pressFn = ui_ctrlr_music_view_pressed;
  
  cbplay.renderFn = ui_ctrlr_music_play_render;;
  cbplay.pressFn = ui_ctrlr_music_play_pressed;
  
  cbqueue.renderFn = ui_ctrlr_music_queue_render;
  cbqueue.pressFn = ui_ctrlr_music_queue_pressed;
  
  cbprev.renderFn = ui_ctrlr_music_prev_render;
  cbprev.pressFn = ui_ctrlr_music_prev_pressed;
  
  cbnext.renderFn = ui_ctrlr_music_next_render;
  cbnext.pressFn = ui_ctrlr_music_next_pressed;

  s_uimusic.toggle = ui_custom_create (s_uicontext, UI_ID_MUSIC_TOGGLE);
  s_uimusic.queue = ui_custom_create (s_uicontext, UI_ID_MUSIC_QUEUE);
  s_uimusic.play = ui_custom_create (s_uicontext, UI_ID_MUSIC_PLAY);
  s_uimusic.prev = ui_custom_create (s_uicontext, UI_ID_MUSIC_PREV);
  s_uimusic.next = ui_custom_create (s_uicontext, UI_ID_MUSIC_NEXT);
  s_uimusic.view = ui_custom_create (s_uicontext, UI_ID_MUSIC_VIEW);
  
  cx_colour darkgrey;
  cx_colour_set (&darkgrey, 0.2f, 0.2f, 0.2f, 1.0f);
  
  ui_custom_set_callbacks (s_uimusic.toggle, &cbtoggle);
  ui_widget_set_colour (s_uimusic.toggle, UI_WIDGET_STATE_NORMAL, cx_colour_black ());
  ui_widget_set_colour (s_uimusic.toggle, UI_WIDGET_STATE_HOVER, &darkgrey);
  ui_widget_set_colour (s_uimusic.toggle, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  ui_widget_set_visible (s_uimusic.toggle, true);
  s_uimusic.toggle->userdata = (void *) 0xffff; // using userdata as toggle switch -_-
  
  cx_colour viewCol;
  cx_colour_set (&viewCol, 0.3f, 0.3f, 0.3f, 0.4f);
  
  ui_custom_set_callbacks (s_uimusic.view, &cbview);
  ui_widget_set_colour (s_uimusic.view, UI_WIDGET_STATE_NORMAL, &viewCol);
  ui_widget_set_colour (s_uimusic.view, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uimusic.view, UI_WIDGET_STATE_FOCUS, &viewCol);
  ui_widget_set_visible (s_uimusic.view, false);
  
  cx_colour nullCol;
  cx_colour_set (&nullCol, 0.0f, 0.0f, 0.0f, 0.0f);
  
  ui_custom_set_callbacks (s_uimusic.queue, &cbqueue);
  ui_widget_set_colour (s_uimusic.queue, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (s_uimusic.queue, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uimusic.queue, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (s_uimusic.queue, false);
  
  ui_custom_set_callbacks (s_uimusic.next, &cbnext);
  ui_widget_set_colour (s_uimusic.next, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (s_uimusic.next, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uimusic.next, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (s_uimusic.next, false);
  
  ui_custom_set_callbacks (s_uimusic.prev, &cbprev);
  ui_widget_set_colour (s_uimusic.prev, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (s_uimusic.prev, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uimusic.prev, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (s_uimusic.prev, false);
  
  ui_custom_set_callbacks (s_uimusic.play, &cbplay);
  ui_widget_set_colour (s_uimusic.play, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (s_uimusic.play, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (s_uimusic.play, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (s_uimusic.play, false);
  
  ui_ctrlr_music_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_destroy (void)
{
  ui_custom_destroy (s_uicontext, s_uimusic.toggle);
  ui_custom_destroy (s_uicontext, s_uimusic.queue);
  ui_custom_destroy (s_uicontext, s_uimusic.play);
  ui_custom_destroy (s_uicontext, s_uimusic.prev);
  ui_custom_destroy (s_uicontext, s_uimusic.next);
  ui_custom_destroy (s_uicontext, s_uimusic.view);
  
  cx_texture_destroy (s_uimusic.iconNote);
  cx_texture_destroy (s_uimusic.iconPlay); 
  cx_texture_destroy (s_uimusic.iconPause);
  cx_texture_destroy (s_uimusic.iconPrev);
  cx_texture_destroy (s_uimusic.iconQueue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_position_setup (void)
{
  float posY = 36.0f - 24.0f; // 12.0f;
  posY += 10.0f;
  
  ui_widget_set_position (s_uimusic.toggle, 0.0f, posY);
  ui_widget_set_dimension (s_uimusic.toggle, 36.0f, 24.0f);

  ui_widget_set_position (s_uimusic.view, 36.0f, posY);
  ui_widget_set_dimension (s_uimusic.view, s_uicontext->canvasWidth - 36.0f, 24.0f);
  
  ui_widget_set_position (s_uimusic.queue, s_uicontext->canvasWidth - 36.0f, posY);
  ui_widget_set_dimension (s_uimusic.queue, 36.0f, 24.0f);

  ui_widget_set_position (s_uimusic.next, s_uicontext->canvasWidth - (36.0f * 2.0f), posY);
  ui_widget_set_dimension (s_uimusic.next, 36.0f, 24.0f);

  ui_widget_set_position (s_uimusic.prev, s_uicontext->canvasWidth - (36.0f * 3.0f), posY);
  ui_widget_set_dimension (s_uimusic.prev, 36.0f, 24.0f);

  ui_widget_set_position (s_uimusic.play, s_uicontext->canvasWidth - (36.0f * 4.0f), posY);
  ui_widget_set_dimension (s_uimusic.play, 36.0f, 24.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_toggle_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uimusic.iconNote;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_toggle_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  if (!audio_music_queued ())
  {
    audio_music_pick (ui_ctrlr_music_picked_callback);
    custom->userdata = NULL;
    audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
  }
  else
  {
    if (!s_uimusic.fade.on)
    {
      if (custom->userdata)
      {
        // show 
        custom->userdata = NULL;
        ui_ctrlr_music_bar_fade_begin (ANIM_FADE_IN, 0.3f);
      }
      else
      {
        // hide
        custom->userdata = (void *) 0xffff;
        ui_ctrlr_music_bar_fade_begin (ANIM_FADE_OUT, 0.3f);
      }
    }
    
    audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
  }
  
  ui_clear_focus (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_picked_callback (void)
{
  CX_ASSERT (!s_uimusic.fade.on);

  if (audio_music_queued ())
  {
    s_uimusic.toggle->userdata = NULL;
    ui_ctrlr_music_bar_fade_begin (ANIM_FADE_IN, 0.3f);
  }
  else
  {
    s_uimusic.toggle->userdata = (void *) 0xffff;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_play_render (ui_custom_t *custom)
{  
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = audio_music_playing () ? s_uimusic.iconPause : s_uimusic.iconPlay;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_play_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  if (audio_music_playing ())
  {
    audio_music_pause ();
  }
  else
  {
    audio_music_play ();
  }
  
  ui_clear_focus (s_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_queue_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uimusic.iconQueue;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_queue_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_pick (NULL);
  
  ui_clear_focus (s_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_view_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  ui_widget_state_t wstate = ui_widget_get_state (s_uicontext, custom);
  const cx_colour *colour = ui_widget_get_colour (custom, wstate);
  const cx_vec2 *pos = ui_widget_get_position (custom);
  const cx_vec2 *dim = ui_widget_get_dimension (custom);
  float opacity = ui_widget_get_opacity (custom);
  
  cx_colour col = *colour;
  col.a *= opacity;
  
  // bg
  
  float w = dim->x;
  float h = dim->y;
  
  float x1 = pos->x;
  float y1 = pos->y;
  float x2 = x1 + w;
  float y2 = y1 + h;
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &col, NULL);
  
  char trackId [128];
  
  int trackIdLen = audio_music_get_track_id (trackId, 128);
  
  if (trackIdLen > 0)
  {
    float xoffset = 12.0f;
    float yoffset = 1.0f;
    
    float tx = x1 + xoffset;
    float ty = y1 + yoffset;
    
    col = *cx_colour_white ();
    col.a *= opacity;
    
    const cx_font *font = util_get_font (FONT_SIZE_14);
    
    cx_font_render (font, trackId, tx, ty, 0.0f, 0, &col);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_view_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  ui_clear_focus (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_prev_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uimusic.iconPrev;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_prev_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_prev ();
  
  ui_clear_focus (s_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_next_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uimusic.iconPrev;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_next_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_next ();
  
  ui_clear_focus (s_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_fade_begin (int type, float duration)
{
  s_uimusic.fade.type = type;
  s_uimusic.fade.duration = duration;
  s_uimusic.fade.t = 0.0f;
  s_uimusic.fade.now = 0.0f;
  s_uimusic.fade.on = true;
  
  ui_widget_set_visible (s_uimusic.view, true);
  ui_widget_set_visible (s_uimusic.play, true);
  ui_widget_set_visible (s_uimusic.prev, true);
  ui_widget_set_visible (s_uimusic.next, true);
  ui_widget_set_visible (s_uimusic.queue, true);
  
  ui_widget_set_enabled (s_uimusic.view, false);
  ui_widget_set_enabled (s_uimusic.play, false);
  ui_widget_set_enabled (s_uimusic.prev, false);
  ui_widget_set_enabled (s_uimusic.next, false);
  ui_widget_set_enabled (s_uimusic.queue, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_fade_end (void)
{
  s_uimusic.fade.on = false;
  
  bool b = (s_uimusic.fade.type == ANIM_FADE_IN);
  
  ui_widget_set_enabled (s_uimusic.view, b);
  ui_widget_set_enabled (s_uimusic.play, b);
  ui_widget_set_enabled (s_uimusic.prev, b);
  ui_widget_set_enabled (s_uimusic.next, b);
  ui_widget_set_enabled (s_uimusic.queue, b);
  
  ui_widget_set_visible (s_uimusic.view, b);
  ui_widget_set_visible (s_uimusic.play, b);
  ui_widget_set_visible (s_uimusic.prev, b);
  ui_widget_set_visible (s_uimusic.next, b);
  ui_widget_set_visible (s_uimusic.queue, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_anim_update (void)
{
  if (s_uimusic.fade.on)
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    s_uimusic.fade.now += deltaTime;
    s_uimusic.fade.t = s_uimusic.fade.now / s_uimusic.fade.duration;
    s_uimusic.fade.t = cx_clamp (s_uimusic.fade.t, 0.0f, 1.0f);
    
    float opacity = (s_uimusic.fade.type == ANIM_FADE_IN) ? s_uimusic.fade.t : (1.0f - s_uimusic.fade.t);
    
    ui_widget_set_opacity (s_uimusic.view, opacity);
    ui_widget_set_opacity (s_uimusic.play, opacity);
    ui_widget_set_opacity (s_uimusic.prev, opacity);
    ui_widget_set_opacity (s_uimusic.next, opacity);
    ui_widget_set_opacity (s_uimusic.queue, opacity);
    
    if (s_uimusic.fade.now > s_uimusic.fade.duration)
    {
      ui_ctrlr_music_bar_fade_end ();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_notification_callback (audio_music_notification_t n)
{
  if ((n == AUDIO_MUSIC_NOTIFICATION_INTERRUPTED) ||
      (n == AUDIO_MUSIC_NOTIFICATION_STOPPED))
  {
    // set pause icon to play
    
    // also do this if app is backgrounded
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_create (void)
{
  memset (&s_uisettings, 0, sizeof (s_uisettings));
  
  ui_custom_t *custom = ui_custom_create (s_uicontext, UI_ID_SETTINGS_BUTTON);
  
  ui_custom_callbacks_t callbacks;
  memset (&callbacks, 0, sizeof (ui_custom_callbacks_t));
  
  callbacks.pressFn = ui_ctrlr_settings_button_pressed;
  callbacks.renderFn = ui_ctrlr_settings_button_render;
  ui_custom_set_callbacks (custom, &callbacks);
  
  cx_colour darkgrey;
  cx_colour_set (&darkgrey, 0.2f, 0.2f, 0.2f, 1.0f);
  
  ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, cx_colour_black ());
  ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, &darkgrey);
  ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  
  s_uisettings.button = custom;
  s_uisettings.button->userdata = (void *) 0xffff;
  s_uisettings.icon = cx_texture_create_from_file ("data/icons/gears-18.png");
  
  ui_ctrlr_settings_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_destroy (void)
{
  ui_custom_destroy (s_uicontext, s_uisettings.button);
  
  cx_texture_destroy (s_uisettings.icon);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_position_setup (void)
{
  float posY = 72.0f - 30.0f; // 42.0f;
  posY += 8.0f;
  
  ui_widget_set_position (s_uisettings.button, 0.0f, posY);
  ui_widget_set_dimension (s_uisettings.button, 36.0f, 24.0f);
  ui_widget_set_visible (s_uisettings.button, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  ui_ctrlr_settings_set_active (true);
 
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
  
  ui_clear_focus (s_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_button_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = s_uisettings.icon;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

