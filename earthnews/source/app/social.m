//
//  social.m
//
//  Created by Ubaka Onyechi on 21/06/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "social.h"
#include "http.h"
#include "../engine/cx_engine.h"
#import <Accounts/Accounts.h>
#import <Twitter/Twitter.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEBUG_LOG 0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

const char *TWITTER_SEARCH_API_URL              = "http://search.twitter.com/search.json";
const int   TWITTER_SEARCH_API_REQUEST_TIMEOUT  = 10;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxu32 s_social_twitter_sinceId = 0;

#if 0
struct social_twitter_api_search_request_t
{
social_twitter_api_search_callback fn;
void *userdata;
};
typedef struct social_twitter_api_search_request_t social_twitter_api_search_request_t;
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


void social_twitter_search_http_callback (http_transaction_id tId, const http_response *response, void *userdata);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

@interface TwitterDelegate : NSObject <UIAlertViewDelegate>
{
  
}
@end

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool social_twitter_init (void)
{
  bool success = true;
  
  float version = [[[UIDevice currentDevice] systemVersion] floatValue];
  
  if (version >= 5.0f)
  {
    success = true;
  }
  else
  {
    CX_DEBUGLOG_CONSOLE (1, "failed due to pre-5.0 iOS version");
    success = false;
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void social_twiiter_user_login (void)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void social_twitter_api_search (social_twitter_tweets_t *tweets, const char *query)
{
  CX_ASSERT (tweets);
  CX_ASSERT (query);

  tweets->dataReady = false;
  
  //////////////////
  // url
  //////////////////
  
  const char *url = TWITTER_SEARCH_API_URL;
  
  //////////////////
  // parameters
  //////////////////
  
  // q
  char q [256];
  http_percent_encode (q, 256, query);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", q);
  
  //////////////////
  // request
  //////////////////
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&result_type=mixed&since_id=%d", url, q, s_social_twitter_sinceId);
  
  CX_DEBUGLOG_CONSOLE (1, "%s", request);
  
  http_get (request, NULL, 0, TWITTER_SEARCH_API_REQUEST_TIMEOUT, social_twitter_search_http_callback, tweets);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void social_twitter_release (social_twitter_tweets_t *tweets)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void social_twitter_search_http_callback (http_transaction_id tId, const http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  social_twitter_tweets_t *tweets = (social_twitter_tweets_t *) userdata;
  
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
        
        // send jsondata to new thread to parse
        CX_DEBUGLOG_CONSOLE (1, jsondata);
        social_twitter_search_parse (tweets, jsondata, jsondataSize);
        
        tweets->dataReady = true;
        tweets->lastUpdateTime = cx_time_get_utc_time ();
        
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

bool social_twitter_search_parse (social_twitter_tweets_t *tweets, const char *data, int dataSize)
{
  CX_ASSERT (tweets);
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
        tweets->maxId = value->u.integer;
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
          
          social_twitter_tweet_item_t *tweetItem = (social_twitter_tweet_item_t *) cx_malloc (sizeof (social_twitter_tweet_item_t));
          memset (tweetItem, 0, sizeof (social_twitter_tweet_item_t));
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
              tweetItem->tweet = cx_strdup (pvalue->u.string.ptr, pvalue->u.string.length);
              CX_DEBUGLOG_CONSOLE (DEBUG_LOG, tweetItem->tweet);
              done++;
            }
            
            if (done == 4)
            {
              break;
            }
          }
          
          tweetItem->next = tweets->items;
          tweets->items = tweetItem;
        }
      }
    }
    
    success = true;
  }
  else 
  {
    CX_DEBUGLOG_CONSOLE (1, errorBuffer);
    CX_FATAL_ERROR ("JSON Parse Error");
    
    success = false;
  }
  
  json_value_free (root);
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
