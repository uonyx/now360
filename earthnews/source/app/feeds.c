//
//  feeds.c
//  earthnews
//
//  Created by Ubaka Onyechi on 14/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "../engine/cx_engine.h"
#include "feeds.h"
#include "render.h"
#include "worker.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG 0

#define WEATHER_YAHOO  1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const int NEWS_HTTP_REQUEST_TIMEOUT     = 30;
const int TWITTER_HTTP_REQUEST_TIMEOUT  = 10;
const int WEATHER_HTTP_REQUEST_TIMEOUT  = 30;
const int TWITTER_API_SEARCH_RPP        = 15;

const char *NEWS_SEARCH_API_URL         = "https://news.google.com/news/feeds";
const char *TWITTER_SEARCH_API_URL      = "http://search.twitter.com/search.json";
#if WEATHER_YAHOO
const char *WEATHER_SEARCH_API_URL      = "http://weather.yahooapis.com/forecastrss";
#else
const char *WEATHER_SEARCH_API_URL      = "http://rss.weather.com/weather/local";
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

enum weather_image_code
{
  // tick
  
  WEATHER_IMAGE_SUNNY_DAY,
  WEATHER_IMAGE_SUNNY_NIGHT,
  WEATHER_IMAGE_CLOUDY_DAY,
  WEATHER_IMAGE_CLOUDY_NIGHT,
  WEATHER_IMAGE_PARTLY_CLOUDY_DAY,
  WEATHER_IMAGE_PARTLY_CLOUDY_NIGHT,
  WEATHER_IMAGE_RAIN_DAY,
  WEATHER_IMAGE_RAIN_NIGHT,
  WEATHER_IMAGE_SNOW_DAY,
  WEATHER_IMAGE_SNOW_NIGHT,
  WEATHER_IMAGE_THUNDERSTORM_DAY,
  WEATHER_IMAGE_THUNDERSTORM_NIGHT,
  WEATHER_IMAGE_FOG_DAY,
  WEATHER_IMAGE_FOG_NIGHT,
  NUM_WEATHER_IMAGES
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_texture *s_weatherIcons [NUM_WEATHER_CONDITION_CODES];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_news_parse (news_feed_t *feed, const char *data, int dataSize);
bool feeds_twitter_parse (twitter_feed_t *feed, const char *data, int dataSize);
bool feeds_weather_parse (weather_feed_t *feed, const char *data, int dataSize);

void feeds_news_clear (news_feed_t *feed);
void feeds_twitter_clear (twitter_feed_t *feed);
void feeds_weather_clear (weather_feed_t *feed);

void news_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata);
void twitter_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata);
void weather_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_init (void)
{
  char weatherFilename [512];
  
  for (int i = 0; i < NUM_WEATHER_CONDITION_CODES; ++i)
  {
    cx_sprintf (weatherFilename, 512, "data/weather_icons/a%d.png", i);
    
    const char *f = weatherFilename;
    
    s_weatherIcons [i] = cx_texture_create_from_file (f);
    
    CX_ASSERT (s_weatherIcons [i]);
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_deinit (void)
{
  for (unsigned int i = 0; i < NUM_WEATHER_CONDITION_CODES; ++i)
  {
    cx_texture_destroy (s_weatherIcons [i]);
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void news_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  news_feed_t *feed = (news_feed_t *) userdata;
  
  if (response->error == CX_HTTP_CONNECTION_ERROR)
  {
    // no internet connection ?
  }
  else
  {
    if (response->statusCode == 200)
    {
      int xmldataSize = response->dataSize;
      char *xmldata = cx_malloc (sizeof (char) * (xmldataSize + 1));
      memcpy (xmldata, response->data, xmldataSize);
      xmldata [xmldataSize] = 0;
      
      // send xmldata to new thread to parse
      feeds_news_clear (feed);
      
      bool parsed = feeds_news_parse (feed, xmldata, xmldataSize);
      CX_ASSERT (parsed);
      CX_REFERENCE_UNUSED_VARIABLE (parsed);
      
      feed->dataReady = true;
      feed->lastUpdate = cx_time_get_utc_time ();
    }
    else
    {
    }
  }
  
  feed->dataPending = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_clear (news_feed_t *feed)
{
  CX_ASSERT (feed);
  
  news_feed_item_t *item = feed->items;
  news_feed_item_t *next = NULL;
  
  while (item)
  {
    next = item->next;
    
    cx_free ((void *) item->date);
    cx_free ((void *) item->link);
    cx_free ((void *) item->title);
    cx_free (item);
    
    item = next;
  }
  
  feed->items = NULL;
  
  if (feed->link)
  {
    cx_free ((void *) feed->link);
    feed->link = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_search (news_feed_t *feed, const char *query)
{
  CX_ASSERT (feed);
  CX_ASSERT (query);
  
  // google: "https://news.google.com/news/feeds?q=lagos&output=rss"
  
  if (feed->dataPending)
  {
    return;
  }
  
  feed->dataReady = false;
  feed->dataPending = true;
  
  // url
  
  const char *url = NEWS_SEARCH_API_URL;
  
  // parameters
  
  // q
  char q [256];
  cx_http_percent_encode (q, 256, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&output=rss", url, q);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  cx_http_get (request, NULL, 0, NEWS_HTTP_REQUEST_TIMEOUT, news_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_render (news_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    const cx_font *font = render_get_ui_font (UI_FONT_SIZE_12);
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

bool feeds_news_get_item (news_feed_item_t *feedItem, news_feed_t *feed, float touchX, float touchY)
{
  CX_ASSERT (feed);
  CX_ASSERT (feedItem);
  
  bool found = false;
  
  if (feed->dataReady)
  {
    memset (feedItem, 0, sizeof (news_feed_item_t));
    
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
    
    const cx_font *font = render_get_ui_font (UI_FONT_SIZE_12);
    
    while (news && count--)
    {
      float w = cx_font_get_text_width (font, news->title);
      
      if ((touchX >= tx) && (touchX <= (tx + w)))
      {
        if ((touchY >= ty) && (touchY <= (ty + itemSpacingY)))
        {
          found = true;
          
          *feedItem = *news;
          
          break;
        }
      }
      
      ty += itemSpacingY;
      
      news = news->next;
    }
    
    if (!found)
    {
      const char *moreNews = "More news...";
      
      float w = cx_font_get_text_width (font, moreNews);
      
      if ((touchX >= tx) && (touchX <= (tx + w)))
      {
        if ((touchY >= ty) && (touchY <= (ty + itemSpacingY)))
        {
          feedItem->link = feed->link;
          feedItem->title = moreNews;
          
          found = true;
        }
      }
    }
  }
  
  return found;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_news_parse (news_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  
  bool success = false;
  
  cx_xml_doc doc;

  cx_xml_doc_create (&doc, data, dataSize);

  if (doc)
  {
    cx_xml_node rootNode = cx_xml_doc_get_root_node (doc);
    cx_xml_node channelNode = cx_xml_node_get_child (rootNode, "channel", NULL);
    cx_xml_node child = cx_xml_node_get_first_child (channelNode);
    
    while (child)
    {
      const char *name = cx_xml_node_get_name (child);
      
      if (strcmp (name, "item") == 0)
      {
        cx_xml_node title = cx_xml_node_get_child (child, "title", NULL);
        cx_xml_node link = cx_xml_node_get_child (child, "link", NULL);
        cx_xml_node pubDate = cx_xml_node_get_child (child, "pubDate", NULL);
        
        news_feed_item_t *rssItem = cx_malloc (sizeof (news_feed_item_t));
        
        rssItem->title = cx_xml_node_get_content (title);
        rssItem->date = cx_xml_node_get_content (pubDate);
        
        // strip out redirect url
        const char *linkContent = cx_xml_node_get_content (link);
        const char *http = "http://";
        const char *c = linkContent;    // original
        const char *s = linkContent;    // save
        
        while (c)
        {
          c = strstr (c, http);
          
          if (c)
          {
            s = c;
            c += strlen (http);
          }
        }
        
        CX_ASSERT (s);
        
        if (!s)
        {
          s = linkContent;
        }
        
        // ***************************** //
        // ESCAPE URLS WITH '%' IN THEM! //
        // ***************************** //
        
        //CX_DEBUGLOG_CONSOLE (s, s);
        
        rssItem->link = cx_strdup (s, strlen (s));
      
        cx_free ((void *) linkContent);
        
        rssItem->next = feed->items;
        feed->items = rssItem;
        
      }
      else if (strcmp (name, "link") == 0)
      {
        feed->link  = cx_xml_node_get_content (child);
      }
      
      child = cx_xml_node_get_next_sibling (child);
    }
    
    success = true;
    
    cx_xml_doc_destroy (&doc);
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Twitter
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void twitter_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  twitter_feed_t *feed = (twitter_feed_t *) userdata;
  
  if (response->error == CX_HTTP_CONNECTION_ERROR)
  {
    // no internet connection ?
  }
  else
  {
    switch (response->statusCode) 
    {
      case 200:
      {
        int jsondataSize = response->dataSize;
        char *jsondata = cx_malloc (sizeof (char) * (jsondataSize + 1));
        
        if (jsondataSize && (jsondataSize > 0))
        {
          memcpy (jsondata, response->data, jsondataSize);
          jsondata [jsondataSize] = 0;
          
          // send jsondata to new thread to parse
          CX_DEBUGLOG_CONSOLE (1, jsondata);
          
          feeds_twitter_clear (feed);
          
          bool parsed = feeds_twitter_parse (feed, jsondata, jsondataSize);
          CX_ASSERT (parsed);
          CX_REFERENCE_UNUSED_VARIABLE (parsed);
          
          feed->dataReady = true;
          feed->lastUpdate = cx_time_get_utc_time ();
        }
        else
        {
          CX_DEBUGLOG_CONSOLE (1, "twitter_http_callback: Warning: JSON data size [%d]", jsondataSize);
        }
        
        break;
      }
        
      case 403:
      {
        break;
      }
        
      case 420:
      {
        break;
      }
        
      default:
      {
        break;
      }
    }
  }
  
  feed->dataPending = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_search (twitter_feed_t *feed, const char *query)
{
  CX_ASSERT (feed);
  CX_ASSERT (query);
  
  if (feed->dataPending)
  {
    return;
  }
  
  feed->dataReady = false;
  feed->dataPending = true;
  
  // url
  
  const char *url = TWITTER_SEARCH_API_URL;
  
  // parameters
  
  // q
  char q [256];
  cx_http_percent_encode (q, 256, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&result_type=mixed&include_entities=1&since_id=%d", url, q, feed->maxId);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  cx_http_get (request, NULL, 0, TWITTER_HTTP_REQUEST_TIMEOUT, twitter_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_clear (twitter_feed_t *feed)
{
  CX_ASSERT (feed);
  
  twitter_tweet_t *tweet = feed->items;
  twitter_tweet_t *next = NULL;
  
  while (tweet)
  {
    next = tweet->next;
    
    cx_free ((void *)tweet->date);
    cx_free ((void *)tweet->userhandle);
    cx_free ((void *)tweet->username);
    cx_free ((void *)tweet->text);
    cx_free (tweet);
    
    tweet = next;
  }
  
  feed->items = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_render (twitter_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady)
  {
    const float width = 360.0f;
    const float height = cx_gdi_get_screen_height ();
    
    float x1 = cx_gdi_get_screen_width() - width;
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
    
    const cx_font *font0 = render_get_ui_font (UI_FONT_SIZE_14);
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

bool feeds_twitter_parse (twitter_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  
  bool success = false;
  
  json_settings settings;
  memset (&settings, 0, sizeof(settings));
  
  char errorBuffer [512];
  errorBuffer [0] = '\0';
  
  json_value *root = json_parse_ex (&settings, data, dataSize, errorBuffer, 512);
  
  if (root)
  {
    CX_ASSERT (root->type == json_object);
    
    unsigned int i, c;
    for (i = 0, c = root->u.object.length; i < c; ++i)
    {
      const char *name = root->u.object.values [i].name;
      json_value *value = root->u.object.values [i].value;
      
      if (strcmp (name, "max_id") == 0)
      {
        CX_ASSERT (value->type == json_integer);
        feed->maxId = value->u.integer;
      }
      else if (strcmp (name, "results") == 0)
      {
        CX_ASSERT (value->type == json_array);
        
        json_value **avalues = value->u.array.values;
        unsigned int alength = value->u.array.length;
        unsigned int j;
        for (j = 0; j < alength; ++j)
        {
          json_value *avalue = avalues [j];
          CX_ASSERT (avalue->type == json_object);
          
          //
          // for memory optimisation, do a pre-pass for one-time malloc of stringdatabuffer for all strings for tweet_item/all tweets
          //
          
          twitter_tweet_t *tweetItem = (twitter_tweet_t *) cx_malloc (sizeof (twitter_tweet_t));
          CX_DEBUGLOG_CONSOLE (DEBUG_LOG, "=====entry=====");
          
          int done = 0;
          unsigned int k, n;
          for (k = 0, n = avalue->u.object.length; k < n; ++k)
          {
            const char *pname = avalue->u.object.values [k].name; // created at
            json_value *pvalue = avalue->u.object.values [k].value; // "date";
            
            if (strcmp (pname, "created_at") == 0)
            {
              CX_ASSERT (pvalue->type == json_string);
              tweetItem->date = cx_strdup (pvalue->u.string.ptr, pvalue->u.string.length);
              CX_DEBUGLOG_CONSOLE (DEBUG_LOG, tweetItem->date);
              done++;
            }
            else if (strcmp (pname, "from_user") == 0)
            {
              CX_ASSERT (pvalue->type == json_string);
              tweetItem->userhandle = cx_strdup (pvalue->u.string.ptr, pvalue->u.string.length);
              CX_DEBUGLOG_CONSOLE (DEBUG_LOG, tweetItem->userhandle);
              done++;
            }
            else if (strcmp (pname, "from_user_name") == 0)
            {
              CX_ASSERT (pvalue->type == json_string);
              tweetItem->username = cx_strdup (pvalue->u.string.ptr, pvalue->u.string.length);
              CX_DEBUGLOG_CONSOLE (DEBUG_LOG, tweetItem->username);
              done++;
            }
            else if (strcmp (pname, "text") == 0)
            {
              CX_ASSERT (pvalue->type == json_string);
              tweetItem->text = cx_strdup (pvalue->u.string.ptr, pvalue->u.string.length);
              CX_DEBUGLOG_CONSOLE (DEBUG_LOG, tweetItem->text);
              done++;
            }
            
            if (done == 4)
            {
              break;
            }
          }
          
          tweetItem->next = feed->items;
          feed->items = tweetItem;
        }
      }
    }
    
    json_value_free (root);
    success = true;
  }
  else 
  {
    CX_DEBUGLOG_CONSOLE (1, errorBuffer);
    CX_FATAL_ERROR ("JSON Parse Error");
    
    success = false;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// weather
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0
struct temp
{
  char *data;
  int dataSize;
  weather_feed_t *feed;
};

static void async_task (void *data)
{
  struct temp *t = (struct temp *) data;
  weather_feed_t *feed = t->feed;
  char *xmldata = t->data;
  int xmldataSize = t->dataSize;
  
  // send xmldata to new thread to parse
  feeds_weather_clear (feed);
  
  bool parsed = feeds_weather_parse (feed, xmldata, xmldataSize);
  CX_ASSERT (parsed);
  CX_REFERENCE_UNUSED_VARIABLE (parsed);
  
  feed->dataReady = true;
  feed->dataPending = false;
  feed->lastUpdate = cx_time_get_utc_time ();
}
#endif

void weather_http_callback (cx_http_request_id tId, const cx_http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  weather_feed_t *feed = (weather_feed_t *) userdata;
  
  if (response->error == CX_HTTP_CONNECTION_ERROR)
  {
    // no internet connection ?
  }
  else
  {
    if (response->statusCode == 200)
    {
      int xmldataSize = response->dataSize;
      char *xmldata = cx_malloc (sizeof (char) * (xmldataSize + 1));
      memcpy (xmldata, response->data, xmldataSize);
      xmldata [xmldataSize] = 0;
      
      // send xmldata to new thread to parse
      feeds_weather_clear (feed);
      
      bool parsed = feeds_weather_parse (feed, xmldata, xmldataSize);
      CX_ASSERT (parsed);
      CX_REFERENCE_UNUSED_VARIABLE (parsed);
      
      feed->dataReady = true;
      feed->dataPending = false;
      feed->lastUpdate = cx_time_get_utc_time ();
#if 0
      struct temp t;
      t.data = xmldata;
      t.dataSize = xmldataSize;
      t.feed = feed;
      worker_add_task (async_task, (void*) &t, NULL);
#endif
    }
    else
    {
    }
  }
  
  feed->dataPending = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_weather_render (weather_feed_t *feed, float x, float y, float z)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady && 
      (feed->conditionCode > WEATHER_CONDITION_CODE_INVALID) && 
      (feed->conditionCode < NUM_WEATHER_CONDITION_CODES))
  {
    cx_texture *image = s_weatherIcons [feed->conditionCode];
    CX_ASSERT (image);
    
    float w = (float) image->width;
    float h = (float) image->height;
    
    float x1 = x - (w * 0.5f);
    float y1 = y - (h * 0.5f);
    float x2 = x1 + w;
    float y2 = y1 + h;
    
    cx_draw_quad (x1, y1, x2, y2, z, 0.0f, cx_colour_white (), image);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_weather_search (weather_feed_t *feed, const char *query)
{
  CX_ASSERT (feed);
  CX_ASSERT (query);
  
  if (feed->dataPending)
  {
    return;
  }
  
  // weather.com: "http://rss.weather.com/weather/local/IVXX0001?cm_cat=rss"
  
  feed->dataReady = false;
  feed->dataPending = true;
  feed->q = query;
  
  // url
  
  const char *url = WEATHER_SEARCH_API_URL;
  
  // request
  
  char request [512];
  
#if WEATHER_YAHOO
  cx_sprintf (request, 512, "%s?w=%s&u=c", url, query);
#else
  cx_sprintf (request, 512, "%s/%s?cm_ven=LWO&cm_cat=rss&par=LWO_rss", url, query);
#endif
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  cx_http_get (request, NULL, 0, WEATHER_HTTP_REQUEST_TIMEOUT, weather_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_weather_clear (weather_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->date)
  {
    cx_free ((void *)feed->date);
    feed->date = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_weather_parse (weather_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  CX_ASSERT (dataSize > 0);
  
  bool success = false;
  
  // find > read temperature
  // find http:
  
  cx_xml_doc doc;
  
  cx_xml_doc_create (&doc, data, dataSize);
  
  if (doc)
  {  
    cx_xml_node rootNode = cx_xml_doc_get_root_node (doc);
    cx_xml_node channelNode = cx_xml_node_get_child (rootNode, "channel", NULL);
    cx_xml_node itemNode = cx_xml_node_get_child (channelNode, "item", NULL);
    cx_xml_node conditionNode = cx_xml_node_get_child (itemNode, "condition", "yweather");
    
    const char *temp = cx_xml_node_get_attr (conditionNode, "temp");
    const char *code = cx_xml_node_get_attr (conditionNode, "code");
    const char *date = cx_xml_node_get_attr (conditionNode, "date");
    
    int tempCelsius = atoi (temp);
    int conditionCode = atoi (code);

    char oclock [8];
    int hour, minute;
    
    //  "Wed, 30 Nov 2005 1:56 pm PST" (RFC822 Section 5 format)
    sscanf (date, "%*s %*d %*s %*d %d:%d %s %*s", &hour, &minute, oclock);
    
    if (strcmp (oclock, "pm") == 0)
    {
      hour += (hour == 12) ? 0 : 12;
    }
    else if ((strcmp (oclock, "am") == 0) && (hour == 12))
    {
      hour = 0;
    }
    
    cx_free ((void *) temp);
    cx_free ((void *) code);
    cx_free ((void *) date);
    
    feed->celsius = tempCelsius;
    feed->conditionCode = conditionCode;
    cx_sprintf (feed->time, 8, "%02d:%02d", hour, minute);
    
    cx_xml_doc_destroy (&doc);
    
    success = true;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
