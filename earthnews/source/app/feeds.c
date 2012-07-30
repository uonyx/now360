//
//  feeds.c
//  earthnews
//
//  Created by Ubaka Onyechi on 14/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "feeds.h"
#include "http.h"
#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG 0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const int NEWS_HTTP_REQUEST_TIMEOUT     = 30;
const int TWITTER_HTTP_REQUEST_TIMEOUT  = 10;
const int WEATHER_HTTP_REQUEST_TIMEOUT  = 30;
const int TWITTER_API_SEARCH_RPP        = 15;

const char *NEWS_SEARCH_API_URL         = "https://news.google.com/news/feeds";
const char *TWITTER_SEARCH_API_URL      = "http://search.twitter.com/search.json";
const char *WEATHER_SEARCH_API_URL      = "http://rss.weather.com/weather/local/";

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_clean (news_feed_t *feed);
void feeds_twitter_clean (twitter_feed_t *feed);
void feeds_weather_clean (weather_feed_t *feed);

void news_http_callback (http_transaction_id tId, const http_response *response, void *userdata);
void twitter_http_callback (http_transaction_id tId, const http_response *response, void *userdata);
void weather_http_callback (http_transaction_id tId, const http_response *response, void *userdata);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void news_http_callback (http_transaction_id tId, const http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  news_feed_t *feed = (news_feed_t *) userdata;
  
  if (response->error == HTTP_CONNECTION_ERROR)
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
      feeds_news_clean (feed);
      
      bool parsed = feeds_news_parse (feed, xmldata, xmldataSize);
      CX_ASSERT (parsed);
      CX_REFERENCE_UNUSED_VARIABLE (parsed);
      
      feed->dataReady = true;
      feed->dataPending = true;
      feed->lastUpdate = cx_time_get_utc_time ();
    }
    else
    {
    }
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
  http_percent_encode (q, 256, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&output=rss", url, q);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  http_get (request, NULL, 0, NEWS_HTTP_REQUEST_TIMEOUT, news_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_clean (news_feed_t *feed)
{
  CX_ASSERT (feed);
  
  news_feed_item_t *item = feed->items;
  news_feed_item_t *next = NULL;
  
  while (item)
  {
    next = item->next;
    
    cx_free ((void *)item->date);
    cx_free ((void *)item->link);
    cx_free ((void *)item->title);
    cx_free (item);
    
    item = next;
  }
  
  feed->items = NULL;
  
  if (feed->link)
  {
    cx_free ((void *)feed->link);
    feed->link = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_news_parse (news_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  
  bool success = true;
  
  xmlDocPtr doc; /* the resulting document tree */
  
  doc = xmlReadMemory (data, dataSize, "noname.xml", NULL, 0);
  
  if (doc) 
  {
    xmlNode *rootnode = xmlDocGetRootElement (doc);
    
    CX_ASSERT (xmlStrcmp (rootnode->name, (xmlChar *) "rss") == 0);
    
    xmlNode *channelNode = rootnode->children;
    
    if (channelNode)
    {
      xmlNode *child1 = channelNode->children;
      
      while (child1)
      {
        if (xmlStrcmp (child1->name, (xmlChar *) "item") == 0)
        {
          news_feed_item_t *rssItem = cx_malloc (sizeof (news_feed_item_t));
          memset (rssItem, 0, sizeof (news_feed_item_t));
          
          xmlNode *child2 = child1->children;
          
          while (child2)
          {
            xmlChar *content = xmlNodeGetContent (child2);
            const char *c = (const char *) content;
            
            if (xmlStrcmp (child2->name, (xmlChar *) "title") == 0)
            {
              CX_DEBUGLOG_CONSOLE (c, c);
              rssItem->title = cx_strdup (c, strlen (c));
            }
            else if (xmlStrcmp (child2->name, (xmlChar *) "link") == 0)
            {
              // strip out redirect url
              const char *http = "http://";
              const char *o = c;    // original
              const char *s = c;    // save
              
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
                s = o;
              }
              
              CX_DEBUGLOG_CONSOLE (s, s);
              
              rssItem->link = cx_strdup (s, strlen (s));
            }
            else if (xmlStrcmp (child2->name, (xmlChar *) "pubDate") == 0)
            {
              CX_DEBUGLOG_CONSOLE (c, c);
              rssItem->date = cx_strdup (c, strlen (c));
            }
            
            xmlFree (content);
            
            child2 = child2->next;
          }
          
          rssItem->next = feed->items;
          feed->items = rssItem;
        }
        else if (xmlStrcmp (child1->name, (xmlChar *) "link") == 0)
        {
          xmlChar *content = xmlNodeGetContent (child1);
          const char *c = (const char *) content;
          CX_DEBUGLOG_CONSOLE (c, c);
          
          feed->link = cx_strdup (c, strlen (c));
          
          xmlFree (content);
        }
        
        child1 = child1->next;
      }
    }
    
    xmlFreeDoc (doc);
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "Failed to parse document\n");
    success = false;
  }
  
  //xmlCleanupParser ();
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
// Twitter
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void twitter_http_callback (http_transaction_id tId, const http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  twitter_feed_t *feed = (twitter_feed_t *) userdata;
  
  if (response->error == HTTP_CONNECTION_ERROR)
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
        memcpy (jsondata, response->data, jsondataSize);
        jsondata [jsondataSize] = 0;
        
        // send jsondata to new thread to parse
        CX_DEBUGLOG_CONSOLE (1, jsondata);
        
        feeds_twitter_clean (feed);
        
        bool parsed = feeds_twitter_parse (feed, jsondata, jsondataSize);
        CX_ASSERT (parsed);
        CX_REFERENCE_UNUSED_VARIABLE (parsed);
        
        feed->dataReady = true;
        feed->dataPending = false;
        feed->lastUpdate = cx_time_get_utc_time ();
        
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
  http_percent_encode (q, 256, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&result_type=mixed&include_entities=1&since_id=%d", url, q, feed->maxId);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  http_get (request, NULL, 0, TWITTER_HTTP_REQUEST_TIMEOUT, twitter_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_clean (twitter_feed_t *feed)
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
          memset (tweetItem, 0, sizeof (twitter_tweet_t));
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

void weather_http_callback (http_transaction_id tId, const http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  weather_feed_t *feed = (weather_feed_t *) userdata;
  
  if (response->error == HTTP_CONNECTION_ERROR)
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
      feeds_weather_clean (feed);
      
      bool parsed = feeds_weather_parse (feed, xmldata, xmldataSize);
      CX_ASSERT (parsed);
      CX_REFERENCE_UNUSED_VARIABLE (parsed);
      
      feed->dataReady = true;
      feed->dataPending = false;
      feed->lastUpdate = cx_time_get_utc_time ();
    }
    else
    {
    }
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
  
  // url
  
  const char *url = WEATHER_SEARCH_API_URL;
  
  // request
  
  char request [512];
  
  cx_sprintf (request, 512, "%s%s?cm_ven=LWO&cm_cat=rss&par=LWO_rss", url, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  http_get (request, NULL, 0, WEATHER_HTTP_REQUEST_TIMEOUT, weather_http_callback, feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_weather_clean (weather_feed_t *feed)
{
  CX_ASSERT (feed);
  
  if (feed->imageUrl)
  {
    cx_free ((void *)feed->imageUrl);
    feed->imageUrl = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_weather_info_parse (void);
void feeds_weather_info_parse ()
{
  // find > read temperature
  // find http:
  
}

bool feeds_weather_parse (weather_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  
  bool success = true;
  
  cx_xml_doc doc;
  
  cx_xml_doc_create (&doc, data, dataSize);
  
  cx_xml_node rootNode = cx_xml_doc_get_root_node (doc);
  
  CX_ASSERT (rootNode);
  
  cx_xml_node channelNode = cx_xml_node_get_child (rootNode, "channel");
  
  CX_ASSERT (channelNode);
  
  cx_xml_node itemNode = cx_xml_node_get_child (channelNode, "item");
  
  bool found = false;
  
  while (itemNode && !found)
  {
    cx_xml_node titleNode = cx_xml_node_get_child (itemNode, "title");
    
    const char *title = cx_xml_node_get_content (titleNode);
        
    if (strstr (title, "Current Weather Conditions In"))
    {
      cx_xml_node descNode = cx_xml_node_get_child (itemNode, "description");

      const char *description = cx_xml_node_get_content (descNode);
      
      // get image URL
      char imageURL [512];
      int imageURLLength = 0;
      
      const char *s = "src=\"";
      const char *k = strstr (description, s);
      
      if (k)
      {
        k += strlen (s);
        
        while (*k != '\"')
        {
          CX_ASSERT (imageURLLength < 512);
          imageURL [imageURLLength++] = *k++;
        }
      }
      
      imageURL [imageURLLength] = 0;
      
      CX_DEBUG_BREAK_ABLE;
      
      
      // get temperature
    }
        
        
    found = true;
    
    itemNode = cx_xml_node_get_next_sibling (itemNode);
    
  }
  
  cx_xml_doc_destroy (&doc);
  
  
  
#if 0
  xmlDocPtr doc; /* the resulting document tree */
  
  doc = xmlReadMemory (data, dataSize, "noname.xml", NULL, 0);
  
  if (doc) 
  {
    xmlNode *rootnode = xmlDocGetRootElement (doc);
    
    CX_ASSERT (xmlStrcmp (rootnode->name, (xmlChar *) "rss") == 0);
    
    xmlNode *channelNode = rootnode->children;
    
    if (channelNode)
    {
      xmlNode *child1 = channelNode->children;
      
      while (child1)
      {
        if (xmlStrcmp (child1->name, (xmlChar *) "item") == 0)
        {
          xmlNode *child2 = child1->children;
          
          while (child2)
          {
            xmlChar *content = xmlNodeGetContent (child2);
            const char *c = (const char *) content;
            
            
            if (xmlStrcmp (child2->name, (xmlChar *) "title") == 0)
            {
              CX_DEBUGLOG_CONSOLE (c, c);
              rssItem->title = cx_strdup (c, strlen (c));
            }
            else if (xmlStrcmp (child2->name, (xmlChar *) "pubDate") == 0)
            {
              CX_DEBUGLOG_CONSOLE (c, c);
              rssItem->date = cx_strdup (c, strlen (c));
            }
            
            xmlFree (content);
            
            child2 = child2->next;
          }
          
        }
        
        child1 = child1->next;
      }
    }
    
    xmlFreeDoc (doc);
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "Failed to parse document\n");
    success = false;
  }
  
  //xmlCleanupParser ();
#endif
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
