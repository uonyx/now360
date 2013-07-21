//
//  ui_controller.c
//  earthnews
//
//  Created by Ubaka Onyechi on 27/12/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "ui_ctrlr.h"
#include "feeds.h"
#include "util.h"
#include "webview.h"
#include "audio.h"
#include "settings.h"
#include "metrics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define UI_CTRLR_DEBUG                (0)
#define UI_CTRLR_DEBUG_NEWS_LOCALISED (0)

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

static ui_context_t *g_uicontext = NULL;

#define NEWS_MAX_ENTRIES  5
#define NEWS_LIST_HEIGHT  120.0f
#define NEWS_LIST_XOFFSET 36.0f //12.0f

#if 0
const float height = 180.0f;
const float posX = 12.0f;
const float posY = 582.0f;
#endif
typedef struct ui_news_t
{
  ui_custom_t *buttons [NEWS_MAX_ENTRIES];
  float opacity;
} ui_news_t;

static ui_news_t g_uinews;

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
  
  char buffer [TWITTER_MAX_TWEET_LEN];
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

static tweet_visible_link_t g_visLinks [TWITTER_TWEET_VIS_LINK_MAX_COUNT];
static unsigned int g_visLinkCount = 0;
static ui_twitter_t g_uitwitter;

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

static ui_music_t g_uimusic;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct ui_settings_t
{
  ui_custom_t *button;
  cx_texture *icon;
} ui_settings_t;

static ui_settings_t g_uisettings;

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
  g_uicontext = ui_init (width, height);
  
  ui_ctrlr_init_create_widgets ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_deinit (void)
{
  ui_ctrlr_deinit_destroy_widgets ();
  
  ui_deinit (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_render (void)
{
  ui_ctrlr_twitter_ticker_anim_update ();
  
  ui_ctrlr_music_bar_anim_update ();
  
  ui_render (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void ui_ctrlr_screen_resize (float width, float height)
{
  ui_canvas_resize (g_uicontext, width, height);
  
  ui_ctrlr_screen_resize_widgets ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool ui_ctrlr_handle_input (const input_touch_event *event)
{
  return ui_input (g_uicontext, event);
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
  g_uinews.opacity = opacity;
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
    g_uisettings.button->userdata = NULL;
  }
  else
  {
    settings_ui_hide ();
    g_uisettings.button->userdata = (void *) 0xffff;
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
  
  ui_widget_state_t wstate = ui_widget_get_state (g_uicontext, custom);
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
  memset (&g_uinews, 0, sizeof (g_uinews));
  
  g_uinews.opacity = 1.0f;
  
  ui_custom_callbacks_t newsCallbacks;
  memset (&newsCallbacks, 0, sizeof (ui_custom_callbacks_t));
  newsCallbacks.pressFn = ui_ctrlr_news_button_pressed;
  newsCallbacks.renderFn = ui_ctrlr_news_button_render;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = ui_custom_create (g_uicontext, UI_ID_NEWS_BUTTON + i);
    
    ui_custom_set_callbacks (custom, &newsCallbacks);
    ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, cx_colour_white ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, cx_colour_null ());
    ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_white ());
    
    g_uinews.buttons [i] = custom;
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
    ui_custom_destroy (g_uicontext, g_uinews.buttons [i]);
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
  const float posY = g_uicontext->canvasHeight - height - botMargin;
  const float itemSpacingY = height / (float) NEWS_MAX_ENTRIES;
  
  float x = posX;
  float y = posY;
  
  for (int i = 0; i < NEWS_MAX_ENTRIES; ++i)
  {
    ui_custom_t *custom = g_uinews.buttons [i];
    
    ui_widget_set_position (custom, x, y);
    
    y += itemSpacingY;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  
  const cx_font *font = util_get_font (FONT_ID_NEWS_18);
  
  float fh = 24.0f; //cx_font_get_height (font);
  
  int displayCount = NEWS_MAX_ENTRIES - 1;
  
  ui_custom_t **buttons = g_uinews.buttons;
  
  CX_ASSERT (displayCount < entryCount);
  
  int i = 0, c = displayCount;
  
  for (i = 0; i < c; ++i)
  {
    feed_news_item_t *e = entryArray [i];
    
    if (settings_get_use_profanity_filter ())
    {
      util_profanity_filter (e->title);
    }
    
    float fw = cx_font_get_text_width (font, e->title);
    
    ui_custom_t *custom = buttons [i];
    
    custom->userdata = e;
    
    ui_widget_set_dimension (custom, fw, fh);
  }
  
#if UI_CTRLR_DEBUG_NEWS_LOCALISED
  char morenews [128];
  util_get_translation (morenews, 128, "TXT_MORE_NEWS");
  cx_strcat (morenews, 128, " ...");
#else
  const char *morenews = "More news ...";
#endif
  
  ui_custom_t *custom = buttons [c];
  
  custom->userdata = feed->link;
  
  ui_widget_set_dimension (custom, cx_font_get_text_width (font, morenews), fh);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_button_render (ui_custom_t *custom)
{
  const char *title = NULL;

#if UI_CTRLR_DEBUG_NEWS_LOCALISED
  char morenews [128];
  util_get_translation (morenews, 128, "TXT_MORE_NEWS");
  cx_strcat (morenews, 128, " ...");
#else
  const char *morenews = "More news ...";
#endif
  
  ui_custom_t *moreNewsButton = g_uinews.buttons [NEWS_MAX_ENTRIES - 1];

  if (moreNewsButton == custom)
  {
    title = custom->userdata ? morenews : NULL;
  }
  else
  {
    feed_news_item_t *entry = (feed_news_item_t *) custom->userdata;
    title = entry ? entry->title : NULL;
  }

  const cx_font *font = util_get_font (FONT_ID_NEWS_18);
  CX_ASSERT (font);
  
  float opacity = g_uinews.opacity;
  float x1 = custom->intr.position.x;
  float y1 = custom->intr.position.y;

  ui_widget_state_t wstate = ui_widget_get_state (g_uicontext, custom);

  float x2 = x1 + custom->intr.dimension.x;
  float y2 = y1 + custom->intr.dimension.y;
  
#if UI_CTRLR_DEBUG
  cx_colour colours [3];    
  cx_colour_set (&colours [0], 1.0f, 0.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [1], 0.0f, 1.0f, 0.0f, 0.45f);
  cx_colour_set (&colours [2], 0.0f, 0.0f, 1.0f, 0.45f);
  
  int uid = custom->intr.uid;
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colours [uid % 3], NULL);
#else
  cx_colour colbg;
  
  if (wstate == UI_WIDGET_STATE_HOVER)
  {
    cx_colour_set (&colbg, 0.192f, 0.05f, 0.384f, 0.65f); // opposite of cyber yellow (purple shade)
  }
  else
  {
    cx_colour_set (&colbg, 0.0f, 0.0f, 0.0f, 0.15f);
  }
  
  cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colbg, NULL);
#endif
  
  if (title)
  {
    cx_colour col;
    cx_colour_set (&col, 1.0f, 0.83f, 0.0f, opacity); // cyber yellow
    cx_font_render (font, title, x1, y1, 0.0f, CX_FONT_ALIGNMENT_DEFAULT, &col);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_news_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  CX_ASSERT (custom);
  
  const char *link = NULL, *title = NULL;
    
#if UI_CTRLR_DEBUG_NEWS_LOCALISED
  char morenews [128];
  util_get_translation (morenews, 128, "TXT_MORE_NEWS");
#else
  const char *morenews = "More news";
#endif
  
  ui_custom_t *moreNewsButton = g_uinews.buttons [NEWS_MAX_ENTRIES - 1];
  
  if (moreNewsButton == custom)
  {
    title = morenews;
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
  
  ui_clear_focus (g_uicontext);
  
  metrics_event_log (METRICS_EVENT_CLICK_NEWS, NULL);
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
  memset (&g_uitwitter, 0, sizeof (g_uitwitter));
  
  g_uitwitter.birdicon = cx_texture_create_from_file ("data/images/ui/twbird-16z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  
  ui_custom_callbacks_t twViewCallbacks, twToggleCallbacks;
  
  memset (&twViewCallbacks, 0, sizeof (ui_custom_callbacks_t));
  memset (&twToggleCallbacks, 0, sizeof (ui_custom_callbacks_t));
  
  twViewCallbacks.renderFn = ui_ctrlr_twitter_ticker_render;
  twViewCallbacks.pressFn = ui_ctrlr_twitter_ticker_pressed;
  twToggleCallbacks.renderFn = ui_ctrlr_twitter_button_render;
  twToggleCallbacks.pressFn = ui_ctrlr_twitter_button_pressed;
  
  g_uitwitter.toggle = ui_custom_create (g_uicontext, UI_ID_TWITTER_TOGGLE);
  ui_custom_set_callbacks (g_uitwitter.toggle, &twToggleCallbacks);
  
  cx_colour darkGrey;
  cx_colour_set (&darkGrey, 0.2f, 0.2f, 0.2f, 0.7f);
  
  cx_colour black;
  cx_colour_set (&black, 0.0f, 0.0f, 0.0f, 0.8f);
  
  ui_widget_set_colour (g_uitwitter.toggle, UI_WIDGET_STATE_NORMAL, &black);
  ui_widget_set_colour (g_uitwitter.toggle, UI_WIDGET_STATE_HOVER, &darkGrey);
  ui_widget_set_colour (g_uitwitter.toggle, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  
  cx_colour viewCol;
  cx_colour_set (&viewCol, 0.0f, 0.0f, 0.0f, 0.2f);
  
  g_uitwitter.view = ui_custom_create (g_uicontext, UI_ID_TWITTER_VIEW);
  ui_custom_set_callbacks (g_uitwitter.view, &twViewCallbacks);

  ui_widget_set_colour (g_uitwitter.view, UI_WIDGET_STATE_NORMAL, &viewCol);
  ui_widget_set_colour (g_uitwitter.view, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uitwitter.view, UI_WIDGET_STATE_FOCUS, &viewCol);
  
  ui_ctrlr_twitter_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_destroy (void)
{
  cx_texture_destroy (g_uitwitter.birdicon);
  ui_custom_destroy (g_uicontext, g_uitwitter.view);
  ui_custom_destroy (g_uicontext, g_uitwitter.toggle);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_position_setup (void)
{
  //float posY = 72.0f - 30.0f;
  float posY = g_uicontext->canvasHeight - 36.0f;
  
  ui_widget_set_position (g_uitwitter.toggle, 0.0f, posY);
  ui_widget_set_dimension (g_uitwitter.toggle, 36.0f, 24.0f);
  ui_widget_set_visible (g_uitwitter.toggle, false);
  
  ui_widget_set_position (g_uitwitter.view, 0.0f, posY);
  ui_widget_set_dimension (g_uitwitter.view, g_uicontext->canvasWidth, 24.0f);
  ui_widget_set_visible (g_uitwitter.view, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_populate (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  CX_ASSERT (feed->reqStatus != FEED_REQ_STATUS_IN_PROGRESS);
  
  ui_custom_t *custom = g_uitwitter.view;
  CX_ASSERT (custom);
  
  ticker_t *ticker = &g_uitwitter.ticker;
  memset (ticker, 0, sizeof (ticker_t));
  
  feed_twitter_tweet_t *tweet = feed->items;
  
  float pixelwidthAvgChar = 6.0f;
  float pixelwidthTotal = 0.0f;
  
  const cx_font *font = util_get_font (FONT_ID_TWITTER_16);
  
  while (tweet && (ticker->itemCount < TWITTER_TICKER_MAX_ITEM_COUNT))
  {
    if (settings_get_use_profanity_filter ())
    {
      util_profanity_filter (tweet->username);
      util_profanity_filter (tweet->text);
    }
    
    float pixelwidth = 0.0f;
    
    pixelwidth += cx_font_get_text_width (font, tweet->username);
    pixelwidth += (pixelwidthAvgChar + pixelwidthAvgChar); // space + @
    
    pixelwidth += cx_font_get_text_width (font, tweet->text);
    pixelwidth += (pixelwidthAvgChar + pixelwidthAvgChar + pixelwidthAvgChar); // padding
    
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
  
  ui_widget_set_visible (g_uitwitter.view, true);
  ui_widget_set_visible (g_uitwitter.toggle, true);
  
  if (!g_uitwitter.toggle->userdata)
  {
    ui_ctrlr_twitter_ticker_fade_begin (ANIM_FADE_IN, 0.3f);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    CX_ASSERT (i < TWITTER_TICKER_TWEET_MAX_ENTRIES);
    dest->elems [i].loc = p0;
    dest->elems [i++].type = TWITTER_TICKER_TWEET_ELEM_TYPE_LINK;
    
    char c = 0;
    char *t = (char *) http;
    
    while ((c = *t++))
    {
      // stride until illegal url character
      if ((c == ' ') || (c == '#') || (c == '@') || (c > 127))
      {
        cxu32 p = t - start;
        
        CX_ASSERT (i < TWITTER_TICKER_TWEET_MAX_ENTRIES);
        dest->elems [i].loc = p;
        dest->elems [i++].type = TWITTER_TICKER_TWEET_ELEM_TYPE_TEXT;
        
        currLoc = p;
        
        if (c != ' ') // uhm... wtf? must be a good reason for this lol.
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
  
  CX_ASSERT (dest->elemCount <= TWITTER_TICKER_TWEET_MAX_ENTRIES);

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
    
    CX_LOG_CONSOLE (1, "%s", str);
    
    CX_DEBUG_BREAKABLE_EXPR;
  }
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);

  g_visLinkCount = 0;
  memset (g_visLinks, 0, sizeof (g_visLinks));
  
  ticker_t *ticker = &g_uitwitter.ticker;
  
  if (ticker)
  {
    ui_widget_state_t wstate = ui_widget_get_state (g_uicontext, custom);
    const cx_colour *colour = ui_widget_get_colour (custom, wstate);
    const cx_vec2 *pos = ui_widget_get_position (custom);
    const cx_vec2 *dim = ui_widget_get_dimension (custom);
    float opacity = ui_widget_get_opacity (custom);
    
    cx_colour colbg = *colour;
    cx_colour colname = *cx_colour_cyan ();
    cx_colour_set (&colname, 0.0f, 0.6745f, 0.9294f, opacity);
    cx_colour coltweet = *cx_colour_white ();
    cx_colour collink;
    //cx_colour_set (&collink, 1.0f, 0.8f, 0.55f, opacity);
    cx_colour_set (&collink, 1.0f, 0.83f, 0.0f, opacity); // cyber yellow
    
    colbg.a *= opacity;
    coltweet.a *= opacity;
    
    float x1 = pos->x;
    float y1 = pos->y;
    float x2 = x1 + dim->x;
    float y2 = y1 + dim->y;
    
    cx_draw_quad (x1, y1, x2, y2, 0.0f, 0.0f, &colbg, NULL);
    
    const cx_font *font = util_get_font (FONT_ID_TWITTER_16);
#if 0
    float deltaTime = (1.0f / 30.0f);
    float scrollx = 60.0f * deltaTime;
#else
    float deltaTime = (float) cx_system_time_get_delta_time ();
    float scrollx = 60.0f * deltaTime;
    scrollx = (float) cx_util_roundup_int (scrollx);
#endif
    
    for (int i = 0, c = ticker->itemCount; i < c; ++i)
    {
      CX_ASSERT (i < TWITTER_TICKER_MAX_ITEM_COUNT);
      
      float ix = x1 + ticker->items [i].posx;
      float iy = pos->y;
      float ix2 = ix + ticker->items [i].width;
      
      feed_twitter_tweet_t *tweet = ticker->items [i].data;
      CX_ASSERT (tweet);
    
      bool outOfBounds = (((ix <= 0.0f) && (ix2 <= 0.0f)) ||
                          ((ix >= g_uicontext->canvasWidth) && (ix2 >= g_uicontext->canvasWidth)));
    
      if (!outOfBounds)
      {
        if (tweet->username && tweet->text)
        {
          ticker_tweet_t tickerTweet;
          memset (&tickerTweet, 0, sizeof (ticker_tweet_t));
          
          ui_ctrlr_twitter_get_ticker_tweet (&tickerTweet, tweet->text);
          
          char username [32];
          cx_sprintf (username, 32, " @%s: ", tweet->username);
          cx_font_render (font, username, ix, iy, 0.0f, 0, &colname);
          
          float rx = ix + cx_font_get_text_width (font, username);
          
          char tickerTweetText [512];
          for (cxu32 j = 0; j < tickerTweet.elemCount; ++j)
          {
            cxu32 loc = tickerTweet.elems [j].loc;
            const char *s = &tickerTweet.buffer [loc];
            cx_sprintf (tickerTweetText, 512, "%s ", s);
            
            bool isLink = (tickerTweet.elems [j].type == TWITTER_TICKER_TWEET_ELEM_TYPE_LINK);
            const cx_colour *col = isLink ? &collink : &coltweet;
            
            cx_font_render (font, tickerTweetText, rx, iy, 0.0f, 0, col);
            
            // collect visible link elems for ui input check - funky hack works just ok!
            if (isLink && (g_visLinkCount < TWITTER_TWEET_VIS_LINK_MAX_COUNT))
            {
              unsigned int idx = g_visLinkCount++;
              CX_ASSERT (idx < TWITTER_TWEET_VIS_LINK_MAX_COUNT);
              
              g_visLinks [idx].x = rx;
              g_visLinks [idx].w = cx_font_get_text_width (font, s);
              cx_strcpy (g_visLinks [idx].url, TWITTER_TWEET_VIS_LINK_URL_BUF_SIZE, s);
            }
            
            rx = rx + cx_font_get_text_width (font, tickerTweetText);
          }
        }
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
  
  for (unsigned int i = 0; i < g_visLinkCount; ++i) 
  {
    float x1 = g_visLinks [i].x;
    float x2 = g_visLinks [i].w + x1;
    
    if ((touchX > x1) && (touchX < x2))
    {
      const char *url = g_visLinks [i].url;
      
      webview_show (url, url);
      
      audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
      
      metrics_event_log (METRICS_EVENT_OPEN_TWITTER_LINK, NULL);
    }
  }
  
  ui_clear_focus (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_render (ui_custom_t *custom)
{
  cx_colour col1;
  cx_colour_set (&col1, 0.0f, 0.6745f, 0.9294f, 1.0f);
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uitwitter.birdicon;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  CX_ASSERT (custom);
  
  if (!g_uitwitter.fade.on)
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
  
  ui_clear_focus (g_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade_begin (int type, float duration)
{
  g_uitwitter.fade.type = type;
  g_uitwitter.fade.duration = duration;
  g_uitwitter.fade.t = 0.0f;
  g_uitwitter.fade.now = 0.0f;
  g_uitwitter.fade.on = true;

  ui_widget_set_enabled (g_uitwitter.view, false);
  ui_widget_set_visible (g_uitwitter.view, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_fade_end (void)
{
  g_uitwitter.fade.on = false;
  
  bool b = (g_uitwitter.fade.type == ANIM_FADE_IN);
  
  ui_widget_set_enabled (g_uitwitter.view, b);
  ui_widget_set_visible (g_uitwitter.view, b);
 }

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_twitter_ticker_anim_update (void)
{
  if (g_uitwitter.fade.on)
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    g_uitwitter.fade.now += deltaTime;
    g_uitwitter.fade.t = g_uitwitter.fade.now / g_uitwitter.fade.duration;
    g_uitwitter.fade.t = cx_clamp (g_uitwitter.fade.t, 0.0f, 1.0f);
    
    float opacity = (g_uitwitter.fade.type == ANIM_FADE_IN) ? g_uitwitter.fade.t : (1.0f - g_uitwitter.fade.t);
    
    ui_widget_set_opacity (g_uitwitter.view, opacity);
    
    if (g_uitwitter.fade.now > g_uitwitter.fade.duration)
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
  memset (&g_uimusic, 0, sizeof (g_uimusic));
    
  g_uimusic.iconNote = cx_texture_create_from_file ("data/images/ui/mnote-16z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  g_uimusic.iconPlay = cx_texture_create_from_file ("data/images/ui/play-12z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  g_uimusic.iconPause = cx_texture_create_from_file ("data/images/ui/pause-12z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  g_uimusic.iconPrev = cx_texture_create_from_file ("data/images/ui/prev-12z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  g_uimusic.iconQueue = cx_texture_create_from_file ("data/images/ui/eject-12z.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  
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

  g_uimusic.toggle = ui_custom_create (g_uicontext, UI_ID_MUSIC_TOGGLE);
  g_uimusic.queue = ui_custom_create (g_uicontext, UI_ID_MUSIC_QUEUE);
  g_uimusic.play = ui_custom_create (g_uicontext, UI_ID_MUSIC_PLAY);
  g_uimusic.prev = ui_custom_create (g_uicontext, UI_ID_MUSIC_PREV);
  g_uimusic.next = ui_custom_create (g_uicontext, UI_ID_MUSIC_NEXT);
  g_uimusic.view = ui_custom_create (g_uicontext, UI_ID_MUSIC_VIEW);
  
  cx_colour darkGrey;
  cx_colour_set (&darkGrey, 0.2f, 0.2f, 0.2f, 0.6f);
  
  cx_colour black;
  cx_colour_set (&black, 0.0f, 0.0f, 0.0f, 0.4f);
  
  ui_custom_set_callbacks (g_uimusic.toggle, &cbtoggle);
  ui_widget_set_colour (g_uimusic.toggle, UI_WIDGET_STATE_NORMAL, &black);
  ui_widget_set_colour (g_uimusic.toggle, UI_WIDGET_STATE_HOVER, &darkGrey);
  ui_widget_set_colour (g_uimusic.toggle, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  ui_widget_set_visible (g_uimusic.toggle, true);
  g_uimusic.toggle->userdata = (void *) 0xffff; // using userdata as toggle switch -_-
  
  cx_colour viewCol;
  cx_colour_set (&viewCol, 0.0f, 0.0f, 0.0f, 0.2f);
  
  ui_custom_set_callbacks (g_uimusic.view, &cbview);
  ui_widget_set_colour (g_uimusic.view, UI_WIDGET_STATE_NORMAL, &viewCol);
  ui_widget_set_colour (g_uimusic.view, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uimusic.view, UI_WIDGET_STATE_FOCUS, &viewCol);
  ui_widget_set_visible (g_uimusic.view, false);
  
  cx_colour nullCol;
  cx_colour_set (&nullCol, 0.0f, 0.0f, 0.0f, 0.0f);
  
  ui_custom_set_callbacks (g_uimusic.queue, &cbqueue);
  ui_widget_set_colour (g_uimusic.queue, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (g_uimusic.queue, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uimusic.queue, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (g_uimusic.queue, false);
  
  ui_custom_set_callbacks (g_uimusic.next, &cbnext);
  ui_widget_set_colour (g_uimusic.next, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (g_uimusic.next, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uimusic.next, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (g_uimusic.next, false);
  
  ui_custom_set_callbacks (g_uimusic.prev, &cbprev);
  ui_widget_set_colour (g_uimusic.prev, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (g_uimusic.prev, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uimusic.prev, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (g_uimusic.prev, false);
  
  ui_custom_set_callbacks (g_uimusic.play, &cbplay);
  ui_widget_set_colour (g_uimusic.play, UI_WIDGET_STATE_NORMAL, &nullCol);
  ui_widget_set_colour (g_uimusic.play, UI_WIDGET_STATE_HOVER, &viewCol);
  ui_widget_set_colour (g_uimusic.play, UI_WIDGET_STATE_FOCUS, &nullCol);
  ui_widget_set_visible (g_uimusic.play, false);
  
  ui_ctrlr_music_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_destroy (void)
{
  ui_custom_destroy (g_uicontext, g_uimusic.toggle);
  ui_custom_destroy (g_uicontext, g_uimusic.queue);
  ui_custom_destroy (g_uicontext, g_uimusic.play);
  ui_custom_destroy (g_uicontext, g_uimusic.prev);
  ui_custom_destroy (g_uicontext, g_uimusic.next);
  ui_custom_destroy (g_uicontext, g_uimusic.view);
  
  cx_texture_destroy (g_uimusic.iconNote);
  cx_texture_destroy (g_uimusic.iconPlay); 
  cx_texture_destroy (g_uimusic.iconPause);
  cx_texture_destroy (g_uimusic.iconPrev);
  cx_texture_destroy (g_uimusic.iconQueue);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_position_setup (void)
{
  float posY = 36.0f - 24.0f; // 12.0f;
  posY += 10.0f;
  
  ui_widget_set_position (g_uimusic.toggle, 0.0f, posY);
  ui_widget_set_dimension (g_uimusic.toggle, 36.0f, 24.0f);

  ui_widget_set_position (g_uimusic.view, 36.0f, posY);
  ui_widget_set_dimension (g_uimusic.view, g_uicontext->canvasWidth - 36.0f, 24.0f);
  
  ui_widget_set_position (g_uimusic.queue, g_uicontext->canvasWidth - 36.0f, posY);
  ui_widget_set_dimension (g_uimusic.queue, 36.0f, 24.0f);

  ui_widget_set_position (g_uimusic.next, g_uicontext->canvasWidth - (36.0f * 2.0f), posY);
  ui_widget_set_dimension (g_uimusic.next, 36.0f, 24.0f);

  ui_widget_set_position (g_uimusic.prev, g_uicontext->canvasWidth - (36.0f * 3.0f), posY);
  ui_widget_set_dimension (g_uimusic.prev, 36.0f, 24.0f);

  ui_widget_set_position (g_uimusic.play, g_uicontext->canvasWidth - (36.0f * 4.0f), posY);
  ui_widget_set_dimension (g_uimusic.play, 36.0f, 24.0f);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_toggle_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uimusic.iconNote;
  
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
    
    metrics_event_log (METRICS_EVENT_MUSIC_QUEUE_1, NULL);
  }
  else
  {
    if (!g_uimusic.fade.on)
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
  
  ui_clear_focus (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_picked_callback (void)
{
  CX_ASSERT (!g_uimusic.fade.on);

  if (audio_music_queued ())
  {
    g_uimusic.toggle->userdata = NULL;
    ui_ctrlr_music_bar_fade_begin (ANIM_FADE_IN, 0.3f);
  }
  else
  {
    g_uimusic.toggle->userdata = (void *) 0xffff;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_play_render (ui_custom_t *custom)
{  
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = audio_music_playing () ? g_uimusic.iconPause : g_uimusic.iconPlay;
  
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
  
  ui_clear_focus (g_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_queue_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uimusic.iconQueue;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_queue_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_pick (NULL);
  
  ui_clear_focus (g_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);

  metrics_event_log (METRICS_EVENT_MUSIC_QUEUE_2, NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_view_render (ui_custom_t *custom)
{
  CX_ASSERT (custom);
  
  ui_widget_state_t wstate = ui_widget_get_state (g_uicontext, custom);
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

#if 1
    const cx_font *font = util_get_font (FONT_ID_DEFAULT_16);
    CX_ASSERT (font);
    cx_font_render (font, trackId, tx, ty, 0.0f, 0, &col);
#else
    const cx_font *font = util_get_font (FONT_ID_NEWS_18);
    CX_ASSERT (font);
    cx_font_set_scale (font, 15.0f/18.0f, 15.0f/18.0f);
    cx_font_render (font, trackId, tx, ty, 0.0f, 0, &col);
    cx_font_set_scale (font, 1.0f, 1.0f);
#endif
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_view_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  ui_clear_focus (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_prev_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uimusic.iconPrev;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_prev_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_prev ();
  
  ui_clear_focus (g_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_next_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uimusic.iconPrev;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, true, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_next_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  audio_music_next ();
  
  ui_clear_focus (g_uicontext);
  
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_fade_begin (int type, float duration)
{
  g_uimusic.fade.type = type;
  g_uimusic.fade.duration = duration;
  g_uimusic.fade.t = 0.0f;
  g_uimusic.fade.now = 0.0f;
  g_uimusic.fade.on = true;
  
  ui_widget_set_visible (g_uimusic.view, true);
  ui_widget_set_visible (g_uimusic.play, true);
  ui_widget_set_visible (g_uimusic.prev, true);
  ui_widget_set_visible (g_uimusic.next, true);
  ui_widget_set_visible (g_uimusic.queue, true);
  
  ui_widget_set_enabled (g_uimusic.view, false);
  ui_widget_set_enabled (g_uimusic.play, false);
  ui_widget_set_enabled (g_uimusic.prev, false);
  ui_widget_set_enabled (g_uimusic.next, false);
  ui_widget_set_enabled (g_uimusic.queue, false);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_fade_end (void)
{
  g_uimusic.fade.on = false;
  
  bool b = (g_uimusic.fade.type == ANIM_FADE_IN);
  
  ui_widget_set_enabled (g_uimusic.view, b);
  ui_widget_set_enabled (g_uimusic.play, b);
  ui_widget_set_enabled (g_uimusic.prev, b);
  ui_widget_set_enabled (g_uimusic.next, b);
  ui_widget_set_enabled (g_uimusic.queue, b);
  
  ui_widget_set_visible (g_uimusic.view, b);
  ui_widget_set_visible (g_uimusic.play, b);
  ui_widget_set_visible (g_uimusic.prev, b);
  ui_widget_set_visible (g_uimusic.next, b);
  ui_widget_set_visible (g_uimusic.queue, b);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_music_bar_anim_update (void)
{
  if (g_uimusic.fade.on)
  {
    float deltaTime = (float) cx_system_time_get_delta_time ();
    
    g_uimusic.fade.now += deltaTime;
    g_uimusic.fade.t = g_uimusic.fade.now / g_uimusic.fade.duration;
    g_uimusic.fade.t = cx_clamp (g_uimusic.fade.t, 0.0f, 1.0f);
    
    float opacity = (g_uimusic.fade.type == ANIM_FADE_IN) ? g_uimusic.fade.t : (1.0f - g_uimusic.fade.t);
    
    ui_widget_set_opacity (g_uimusic.view, opacity);
    ui_widget_set_opacity (g_uimusic.play, opacity);
    ui_widget_set_opacity (g_uimusic.prev, opacity);
    ui_widget_set_opacity (g_uimusic.next, opacity);
    ui_widget_set_opacity (g_uimusic.queue, opacity);
    
    if (g_uimusic.fade.now > g_uimusic.fade.duration)
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
  memset (&g_uisettings, 0, sizeof (g_uisettings));
  
  ui_custom_t *custom = ui_custom_create (g_uicontext, UI_ID_SETTINGS_BUTTON);
  
  ui_custom_callbacks_t callbacks;
  memset (&callbacks, 0, sizeof (ui_custom_callbacks_t));
  
  callbacks.pressFn = ui_ctrlr_settings_button_pressed;
  callbacks.renderFn = ui_ctrlr_settings_button_render;
  ui_custom_set_callbacks (custom, &callbacks);
  
  cx_colour darkGrey;
  cx_colour_set (&darkGrey, 0.2f, 0.2f, 0.2f, 0.6f);
  
  cx_colour black;
  cx_colour_set (&black, 0.0f, 0.0f, 0.0f, 0.4f);
  
  ui_widget_set_colour (custom, UI_WIDGET_STATE_NORMAL, &black);
  ui_widget_set_colour (custom, UI_WIDGET_STATE_HOVER, &darkGrey);
  ui_widget_set_colour (custom, UI_WIDGET_STATE_FOCUS, cx_colour_black ());
  
  g_uisettings.button = custom;
  g_uisettings.button->userdata = (void *) 0xffff;
  g_uisettings.icon = cx_texture_create_from_file ("data/images/ui/gears-18.png", CX_FILE_STORAGE_BASE_RESOURCE, false);
  
  ui_ctrlr_settings_position_setup ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_destroy (void)
{
  ui_custom_destroy (g_uicontext, g_uisettings.button);
  
  cx_texture_destroy (g_uisettings.icon);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_position_setup (void)
{
  float posY = 72.0f - 30.0f; // 42.0f;
  posY += 8.0f;
  
  ui_widget_set_position (g_uisettings.button, 0.0f, posY);
  ui_widget_set_dimension (g_uisettings.button, 36.0f, 24.0f);
  ui_widget_set_visible (g_uisettings.button, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_button_pressed (ui_custom_t *custom, const cx_vec2 *point)
{
  ui_ctrlr_settings_set_active (true);
 
  audio_soundfx_play (AUDIO_SOUNDFX_CLICK1);
  
  ui_clear_focus (g_uicontext);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void ui_ctrlr_settings_button_render (ui_custom_t *custom)
{
  cx_colour col1 = *cx_colour_white ();
  cx_colour col2 = *cx_colour_grey ();
  cx_texture *image = g_uisettings.icon;
  
  ui_ctrlr_button_render_with_image (custom, &col1, &col2, image, false, true);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

