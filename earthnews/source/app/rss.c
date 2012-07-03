//
//  rss.c
//
//  Created by Ubaka Onyechi on 02/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "rss.h"
#include "http.h"
#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const int RSS_HTTP_REQUEST_TIMEOUT = 30;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void rss_http_callback (http_transaction_id tId, const http_response *response, void *userdata);
bool rss_parse_xml (rss_feed_t *feed, const char *data, int dataSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void rss_get_feed (rss_feed_t *feed, const char *url)
{
  CX_ASSERT (feed);
  //const char *url = "https://news.google.com/news/feeds?q=nigeria&output=rss";
  
  http_transaction_id id = http_get (url, NULL, 0, RSS_HTTP_REQUEST_TIMEOUT, rss_http_callback, feed);
  CX_REFERENCE_UNUSED_VARIABLE (id);
  
  feed->dataReady = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void rss_release_feed (rss_feed_t *feed)
{
  CX_ASSERT (feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void rss_http_callback (http_transaction_id tId, const http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  rss_feed_t *feed = (rss_feed_t *) userdata;
  
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
      
      // send xmldata to new thread to parse
      rss_parse_xml (feed, xmldata, xmldataSize);
      
      feed->dataReady = true;
    }
    else
    {
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool rss_parse_xml (rss_feed_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  
  bool success = true;
  
  xmlDocPtr doc; /* the resulting document tree */
  
  doc = xmlReadMemory (data, dataSize, "noname.xml", NULL, 0);
  
  if (doc == NULL) 
  {
    CX_DEBUGLOG_CONSOLE (1, "Failed to parse document\n");
    success = false;
  }
  
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
        rss_feed_item_t *rssItem = cx_malloc (sizeof (rss_feed_item_t));
        memset (rssItem, 0, sizeof (rss_feed_item_t));
        
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
            const char *http = "http://";
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
            
            if (s)
            {
              CX_DEBUGLOG_CONSOLE (s, s);
            }
            else 
            {
              s = c;
            }
            
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
      
      child1 = child1->next;
    }
  }
  
  xmlFreeDoc (doc);
  
  //xmlCleanupParser ();
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
