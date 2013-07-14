//
//  feeds.m
//  earthnews
//
//  Created by Ubaka Onyechi on 14/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#import "feeds.h"
#import "metrics.h"
#import "settings.h"
#import "worker.h"
#import <Accounts/Accounts.h>
#import <Social/Social.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define NEWS_SEARCH_API_URL           "https://news.google.com/news/feeds"
#define NEWS_HTTP_REQUEST_TIMEOUT     (30)

#if FEEDS_TWITTER_API_1_0
#define TWITTER_SEARCH_API_URL        "http://search.twitter.com/search.json"
#else
#define TWITTER_SEARCH_API_URL        "https://api.twitter.com/1.1/search/tweets.json"
#define TWITTER_TTL                   (9)

#endif
#define TWITTER_SEARCH_API_RPP        "15"
#define TWITTER_HTTP_REQUEST_TIMEOUT  (10)

#if FEEDS_WEATHER_API_YAHOO
#define WEATHER_SEARCH_API_URL        "http://weather.yahooapis.com/forecastrss"
#else
#define WEATHER_SEARCH_API_URL        "http://rss.weather.com/weather/local"
#endif
#define WEATHER_HTTP_REQUEST_TIMEOUT  (30)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

enum weather_condition_code
{
  WEATHER_CONDITION_CODE_INVALID = -1,
  
  WEATHER_CONDITION_CODE_TORNADO,
  WEATHER_CONDITION_CODE_TROPICAL_STORM,
  WEATHER_CONDITION_CODE_HURRICANE,
  WEATHER_CONDITION_CODE_SEVERE_THUNDERSTORMS,
  WEATHER_CONDITION_CODE_THUNDERSTORMS,
  WEATHER_CONDITION_CODE_MIXED_RAIN_AND_SNOW,
  WEATHER_CONDITION_CODE_MIXED_RAIN_AND_SLEET,
  WEATHER_CONDITION_CODE_MIXED_SNOW_AND_SLEET,
  WEATHER_CONDITION_CODE_FREEZING_DRIZZLE,
  WEATHER_CONDITION_CODE_DRIZZLE,
  WEATHER_CONDITION_CODE_FREEZING_RAIN,
  WEATHER_CONDITION_CODE_SHOWERS,
  WEATHER_CONDITION_CODE_SHOWERS_1,
  WEATHER_CONDITION_CODE_SNOW_FLURRIES,
  WEATHER_CONDITION_CODE_LIGHT_SNOW_SHOWERS,
  WEATHER_CONDITION_CODE_BLOWING_SNOW,
  WEATHER_CONDITION_CODE_SNOW,
  WEATHER_CONDITION_CODE_HAIL,
  WEATHER_CONDITION_CODE_SLEET,
  WEATHER_CONDITION_CODE_DUST,
  WEATHER_CONDITION_CODE_FOGGY,
  WEATHER_CONDITION_CODE_HAZE,
  WEATHER_CONDITION_CODE_SMOKY,
  WEATHER_CONDITION_CODE_BLUSTERY,
  WEATHER_CONDITION_CODE_WINDY,
  WEATHER_CONDITION_CODE_COLD,
  WEATHER_CONDITION_CODE_CLOUDY,
  WEATHER_CONDITION_CODE_MOSTLY_CLOUDY_NIGHT,
  WEATHER_CONDITION_CODE_MOSTLY_CLOUDY_DAY,
  WEATHER_CONDITION_CODE_PARTLY_CLOUDY_NIGHT,
  WEATHER_CONDITION_CODE_PARTLY_CLOUDY_DAY,
  WEATHER_CONDITION_CODE_CLEAR_NIGHT,
  WEATHER_CONDITION_CODE_SUNNY,
  WEATHER_CONDITION_CODE_FAIR_NIGHT,
  WEATHER_CONDITION_CODE_FAIR_DAY,
  WEATHER_CONDITION_CODE_MIXED_RAIN_AND_HAIL,
  WEATHER_CONDITION_CODE_HOT,
  WEATHER_CONDITION_CODE_ISOLATED_THUNDERSTORMS,
  WEATHER_CONDITION_CODE_SCATTERED_THUNDERSTORMS,
  WEATHER_CONDITION_CODE_SCATTERED_THUNDERSTORMS_1,
  WEATHER_CONDITION_CODE_SCATTERED_SHOWERS,
  WEATHER_CONDITION_CODE_HEAVY_SNOW,
  WEATHER_CONDITION_CODE_SCATTERED_SNOW_SHOWERS,
  WEATHER_CONDITION_CODE_HEAVY_SNOW_1,
  WEATHER_CONDITION_CODE_PARTLY_CLOUDY,
  WEATHER_CONDITION_CODE_THUNDERSHOWERS,
  WEATHER_CONDITION_CODE_SNOW_SHOWERS,
  WEATHER_CONDITION_CODE_ISOLATED_THUNDERSHOWERS,
  
  NUM_WEATHER_CONDITION_CODES
};

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
  NUM_WEATHER_IMAGE_CODES,
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_texture *g_weatherIcons [NUM_WEATHER_CONDITION_CODES];

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool feeds_news_parse (feed_news_t *feed, const char *data, int dataSize);
static void feeds_news_clear (feed_news_t *feed);

static bool feeds_weather_parse (feed_weather_t *feed, const char *data, int dataSize);
static void feeds_weather_clear (feed_weather_t *feed);

static bool feeds_twitter_parse (feed_twitter_t *feed, const char *data, int dataSize);
static void feeds_twitter_clear (feed_twitter_t *feed);
static void feeds_twitter_error (feed_twitter_t *feed, const char *error);

static void http_callback_news (cx_http_request_id tId, const cx_http_response *response, void *userdata);
static void http_callback_weather (cx_http_request_id tId, const cx_http_response *response, void *userdata);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_init (void)
{
  char weatherFilename [512];
  
  for (int i = 0; i < NUM_WEATHER_CONDITION_CODES; ++i)
  {
#if 1 // use png textures
    cx_sprintf (weatherFilename, 512, "data/images/weather/a%d.png", i);
#else
    cx_sprintf (weatherFilename, 512, "data/images/weather/a%d.pvr", i);
#endif
    
    const char *f = weatherFilename;
    
    g_weatherIcons [i] = cx_texture_create_from_file (f, CX_FILE_STORAGE_BASE_RESOURCE, false);
    
    CX_ASSERT (g_weatherIcons [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_deinit (void)
{
  for (unsigned int i = 0; i < NUM_WEATHER_CONDITION_CODES; ++i)
  {
    cx_texture_destroy (g_weatherIcons [i]);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void http_callback_news (cx_http_request_id tId, const cx_http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  feed_news_t *feed = (feed_news_t *) userdata;
  
  if (response->error == CX_HTTP_CONNECTION_ERROR)
  {
    // no internet connection ?
    CX_LOG_CONSOLE (1, "http_callback_news: Warning: no internet connection");
    
    feed->reqStatus = FEED_REQ_STATUS_ERROR;
  }
  else
  {
    if (response->statusCode == 200)
    {
      feeds_news_clear (feed);
      if (feeds_news_parse (feed, (const char *) response->data, response->dataSize))
      {
        feed->lastUpdate = cx_time_get_utc_epoch ();
        feed->reqStatus = FEED_REQ_STATUS_SUCCESS;
      }
      else
      {
        CX_LOG_CONSOLE (1, "http_callback_news: Warning: Parse failed for query: %s", feed->query);
        feed->reqStatus = FEED_REQ_STATUS_FAILURE;
      }
    }
    else
    {
      feeds_news_clear (feed);
      feed->reqStatus = FEED_REQ_STATUS_FAILURE;
      
      char errorCode [32];
      cx_sprintf (errorCode, 32, "%d", response->statusCode);
      metrics_event_log (METRICS_EVENT_NEWS_API_ERROR, errorCode);
    }
  }
  
  feed->httpReqId = CX_HTTP_REQUEST_ID_INVALID;
  feed->query = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void feeds_news_clear (feed_news_t *feed)
{
  CX_ASSERT (feed);
  
  feed_news_item_t *item = feed->items;
  feed_news_item_t *next = NULL;
  
  while (item)
  {
    next = item->next;
    
    cx_free (item);
    
    item = next;
  }
  
  feed->items = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_search (feed_news_t *feed, const char *query)
{
  // google: "https://news.google.com/news/feeds?q=lagos&output=rss"
  
  CX_ASSERT (feed);
  CX_ASSERT (query);
  CX_ASSERT (feed->reqStatus == FEED_REQ_STATUS_INVALID);
  
  feed->reqStatus = FEED_REQ_STATUS_IN_PROGRESS;
  
  // url
  
  const char *url = NEWS_SEARCH_API_URL;
  
  // parameters
  
  // q
  char q [256];
  cx_str_percent_encode (q, 256, query);
  
  CX_LOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&output=rss", url, q);
  
  CX_LOG_CONSOLE (1, "%s", request);
  
  feed->httpReqId = cx_http_get (request, NULL, 0, NEWS_HTTP_REQUEST_TIMEOUT, http_callback_news, feed);
  
  feed->query = query;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_cancel_search (feed_news_t *feed)
{
  CX_ASSERT (feed);
  
  cx_http_cancel (&feed->httpReqId);
  
  feed->reqStatus = FEED_REQ_STATUS_INVALID;
  feed->query = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool feeds_news_parse (feed_news_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  
  bool success = false;
  
  cx_xml_doc doc = cx_xml_doc_create (data, dataSize);

  if (doc)
  {
    cx_xml_node rootNode = cx_xml_doc_root_node (doc);
    cx_xml_node channelNode = cx_xml_node_child (rootNode, "channel", NULL);
    cx_xml_node child = cx_xml_node_first_child (channelNode);
    
    while (child)
    {
      const char *name = cx_xml_node_name (child);
            
      if (strcmp (name, "item") == 0)
      {
        cx_xml_node titleNode = cx_xml_node_child (child, "title", NULL);
        cx_xml_node linkNode = cx_xml_node_child (child, "link", NULL);
        
        if (titleNode && linkNode)
        {        
          feed_news_item_t *rssItem = (feed_news_item_t *) cx_malloc (sizeof (feed_news_item_t));
          memset (rssItem, 0, sizeof (feed_news_item_t));
          
          // title
          char *title = cx_xml_node_content (titleNode);
          cx_str_html_unescape (rssItem->title, FEED_NEWS_TITLE_MAX_LEN, title);
          cx_free (title);
      
          // link
          char *link = cx_xml_node_content (linkNode);
#if 0
          // strip out redirect url to speed up browser page loading
          const char *http = "http://";
          const char *c = link; // original
          const char *s = link; // save
          
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
            s = link;
          }
          
          cx_strcpy (rssItem->link, FEED_NEWS_LINK_MAX_LEN, s);
#else
          cx_strcpy (rssItem->link, FEED_NEWS_LINK_MAX_LEN, link);
#endif
          cx_free (link);
          
          // pubDate
          
          cx_xml_node pubDate = cx_xml_node_child (child, "pubDate", NULL);
          
          if (pubDate)
          {
            const char *d = cx_xml_node_content (pubDate);
            
            char day [32];
            char monn [32];
            char zone [32];
            int mday = 0;
            int year = 0;
            int hour = 0;
            int mins = 0;
            int secs = 0;
            
            // "Sat, 23 Mar 2013 12:17:18 GMT"
            sscanf (d, "%s %d %s %d %d:%d:%d %s", day, &mday, monn, &year, &hour, &mins, &secs, zone);
            
            static const char *monthName [12] =
            {
              "Jan",
              "Feb",
              "Mar",
              "Apr",
              "May",
              "Jun",
              "Jul",
              "Aug",
              "Sep",
              "Oct",
              "Nov",
              "Dec"
            };
            
            int month = -1;
            
            for (int i = 0; i < 12; ++i)
            {
              if (strcmp (monn, monthName [i]) == 0)
              {
                month = i;
                break;
              }
            }
            
            CX_ASSERT (month > -1);
            
            int seconds = (hour * 3600) + (mins * 60) + secs;
            
            rssItem->pubDateInfo.mday = mday;
            rssItem->pubDateInfo.mon = month;
            rssItem->pubDateInfo.secs = seconds;
          }
          
          // next
          rssItem->next = feed->items;
          feed->items = rssItem;
        }
      }
      else if (strcmp (name, "link") == 0)
      {
        // feed link
        char *link = cx_xml_node_content (child);
        cx_strcpy (feed->link, FEED_NEWS_LINK_MAX_LEN, link);
        cx_free (link);
      }
      
      child = cx_xml_node_next_sibling (child);
    }
    
    success = true;
    
    cx_xml_doc_destroy (doc);
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

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_search (feed_twitter_t *feed, const char *query, bool local, float lat, float lon)
{
  CX_ASSERT (feed);
  CX_ASSERT (query);
  CX_ASSERT (feed->reqStatus == FEED_REQ_STATUS_INVALID);
 
#if FEEDS_TWITTER_API_1_0
  feed->reqStatus = FEED_REQ_STATUS_IN_PROGRESS;
  
  // url
  
  const char *url = TWITTER_SEARCH_API_URL;
  
  // parameters
  
  // q
  char q [256];
  cx_str_percent_encode (q, 256, query);
  
  CX_LOG_CONSOLE (1, "%s", q);
  
  // request
  
  char request [512];
  cx_sprintf (request, 512, "%s?q=%s&result_type=mixed&include_entities=1&since_id=%d", url, q, feed->maxId);
  
  CX_LOG_CONSOLE (1, "%s", request);
  
  feed->httpReqId = cx_http_get (request, NULL, 0, TWITTER_HTTP_REQUEST_TIMEOUT, http_callback_twitter, feed);
  
  feed->query = query;
  
#else  
  time_t currentTime = cx_time_get_utc_epoch ();
  
  if ((currentTime - feed->lastUpdate) > TWITTER_TTL)
  {
    feed->reqStatus = FEED_REQ_STATUS_IN_PROGRESS;
    
    
    ACAccountStore *accountStore = [[[ACAccountStore alloc] init] autorelease];
    ACAccountType *accountType = [accountStore accountTypeWithAccountTypeIdentifier:ACAccountTypeIdentifierTwitter];
    
    [accountStore requestAccessToAccountsWithType:accountType options:nil completion:^(BOOL granted, NSError *error)
     {
       const char *errorNoTwitter = "There are no Twitter accounts configured. To receive live tweets, "
                                    "you can add or create a Twitter account in \"Settings\".";
       
       const char *errorConnection = "There seems to be a connection error with the Twitter service "
                                     "at the moment. Please try again later.";
       if (granted)
       {
         NSArray *accounts = [accountStore accountsWithAccountType:accountType];
         
         if ([accounts count] > 0)
         {
           NSString *nsquery = nil;
           
           const char *qsplit = strchr (query, ',');
           
           if (qsplit)
           {
             char srchQuery [128];
             char q1 [64];
             char q2 [64];
             
             int q1Len = qsplit - query;
             
             cx_strcpy (q1, q1Len + 1, query);
             cx_strcpy (q2, 64, qsplit + 1);
             cx_sprintf (srchQuery, 128, "%s OR %s", q1, q2);
             
             nsquery = [NSString stringWithUTF8String:srchQuery];
           }
           else
           {
             nsquery = [NSString stringWithUTF8String:query];
           }
                    
           NSURL *url = [NSURL URLWithString:@TWITTER_SEARCH_API_URL];
           NSString *nsSinceId = [NSString stringWithUTF8String:feed->maxIdStr];           
           NSDictionary *params = nil;
           
           if (local)
           {
             NSString *geocode = [NSString stringWithFormat:@"%.4f,%.4f,%d%s", lat, lon, 1000, "mi"];

             params = @{ @"q" : nsquery,
                         @"result_type" : @"recent",
                         @"include_entities" : @"false",
                         @"count": @TWITTER_SEARCH_API_RPP,
                         @"geocode": geocode,
#if CX_DEBUG && 0
                         @"lang": @"en",  // currently limited to 'en' due to poor font support
#endif
                         @"since_id" : nsSinceId};
           }
           else
           {
             params = @{ @"q" : nsquery,
                         @"result_type" : @"mixed",
                         @"include_entities" : @"false",
                         @"count": @TWITTER_SEARCH_API_RPP,
#if CX_DEBUG && 0
                         @"lang": @"en",  // currently limited to 'en' due to poor font support
#endif
                         @"since_id" : nsSinceId};
           }
           
           SLRequest *request = [SLRequest requestForServiceType:SLServiceTypeTwitter
                                                   requestMethod:SLRequestMethodGET
                                                             URL:url
                                                      parameters:params];
           
           [request setAccount:[accounts lastObject]];
           
           [request performRequestWithHandler:^(NSData *responseData, NSHTTPURLResponse *response, NSError *err)
            {
              if (responseData && (responseData.length > 0))
              {
                int statusCode = response.statusCode;
                int response_dataSize = responseData.length;
                char *response_data = (char *) responseData.bytes;
              
                if (statusCode == 200)
                {
                  CX_LOG_CONSOLE (CX_DEBUG && 0, "%s", response_data);
                  
                  feeds_twitter_clear (feed);
                  if (feeds_twitter_parse (feed, (const char *) response_data, response_dataSize))
                  {
                    feed->lastUpdate = cx_time_get_utc_epoch ();
                    feed->reqStatus = FEED_REQ_STATUS_SUCCESS;
                  }
                  else
                  {
                    CX_LOG_CONSOLE (1, "http_callback_twitter: Warning: Parse failed for query: %s", feed->query);
                    feeds_twitter_clear (feed);
                    feeds_twitter_error (feed, errorConnection);
                    feed->reqStatus = FEED_REQ_STATUS_FAILURE;
                  }
                }
                else
                {
                  feeds_twitter_clear (feed);
                  feeds_twitter_error (feed, errorConnection);
                  feed->reqStatus = FEED_REQ_STATUS_FAILURE;
                
                  char errorCode [32];
                  cx_sprintf (errorCode, 32, "%d", statusCode);
                  metrics_event_log (METRICS_EVENT_TWITTER_API_ERROR, errorCode);
                }
              }
              else // no response data - offline?
              {
                feed->reqStatus = FEED_REQ_STATUS_ERROR;
              }
            }
          ];
         }
         else // no twitter accounts
         {
           feeds_twitter_clear (feed);
           feeds_twitter_error (feed, errorNoTwitter);
           feed->reqStatus = FEED_REQ_STATUS_FAILURE;
         }
       }
       else // no twitter access
       {
         feeds_twitter_clear (feed);
         feeds_twitter_error (feed, errorNoTwitter);
         feed->reqStatus = FEED_REQ_STATUS_FAILURE;
       }
       
       CX_LOG_CONSOLE (1, "*** done");
     }
    ];
  }
  else
  {
    feed->reqStatus = FEED_REQ_STATUS_SUCCESS;
  }

#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_cancel_search (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  
#if 0
  cx_http_cancel (&feed->httpReqId);
  
  feed->reqStatus = FEED_REQ_STATUS_INVALID;
  feed->query = NULL;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void feeds_twitter_clear (feed_twitter_t *feed)
{
  CX_ASSERT (feed);
  
  feed_twitter_tweet_t *tweetItem = feed->items;
  feed_twitter_tweet_t *nextItem = NULL;
  
  while (tweetItem)
  {
    nextItem = tweetItem->next;
    
    cx_free (tweetItem);
    
    tweetItem = nextItem;
  }
  
  feed->items = NULL;
  feed->lastUpdate = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void feeds_twitter_error (feed_twitter_t *feed, const char *error)
{
  CX_ASSERT (feed);
  CX_ASSERT (error);
  
  const char *text = error;
  const char *username = "now360_app";
  
  if (text && username)
  {
    feed_twitter_tweet_t *tweetItem = (feed_twitter_tweet_t *) cx_malloc (sizeof (feed_twitter_tweet_t));
    memset (tweetItem, 0, sizeof (feed_twitter_tweet_t));
    
    cx_strcpy (tweetItem->text, FEED_TWITTER_TWEET_MESSAGE_MAX_LEN, text);
    cx_strcpy (tweetItem->username, FEED_TWITTER_TWEET_USERNAME_MAX_LEN, username);
    
    tweetItem->next = feed->items;
    feed->items = tweetItem;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool feeds_twitter_parse (feed_twitter_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  
  // parsing could be moved to seperate thread
  
  bool success = false;
  
  cx_json_tree jsonTree = cx_json_tree_create (data, dataSize);
  
  if (jsonTree)
  {
    cx_json_node rootNode = cx_json_tree_root_node (jsonTree);

    
    bool error = false;
    
    cx_json_node metadataNode = cx_json_object_child (rootNode, "search_metadata");
    
    if (metadataNode)
    {
      cx_json_node maxIdNode = cx_json_object_child (metadataNode, "max_id_str");
      
      const char *max_id_str = maxIdNode ? cx_json_value_string (maxIdNode) : "";
      
      cx_strcpy (feed->maxIdStr, 32, max_id_str);
    }
    
    cx_json_node statusesNode = cx_json_object_child (rootNode, "statuses");
    
    if (statusesNode)
    {
      for (unsigned int i = 0, c = cx_json_array_size (statusesNode); (i < c) && !error; ++i)
      {        
        const char *text = NULL;
        const char *username = NULL;
        
        cx_json_node arrayNode = cx_json_array_member (statusesNode, i);
        
        if (arrayNode)
        {
          cx_json_node textNode = cx_json_object_child (arrayNode, "text");
          
          if (textNode)
          {
            text = cx_json_value_string (textNode);
          }
          
          cx_json_node userNode = cx_json_object_child (arrayNode, "user");
          
          if (userNode)
          {
            cx_json_node screennameNode = cx_json_object_child (userNode, "screen_name");
            
            if (screennameNode)
            {
              username = cx_json_value_string (screennameNode);
            }
          }
        }
        
        if (text && username)
        {
          feed_twitter_tweet_t *tweetItem = (feed_twitter_tweet_t *) cx_malloc (sizeof (feed_twitter_tweet_t));
          memset (tweetItem, 0, sizeof (feed_twitter_tweet_t));
        
          cx_str_html_unescape (tweetItem->username, FEED_TWITTER_TWEET_USERNAME_MAX_LEN, username);
          cx_str_html_unescape (tweetItem->text, FEED_TWITTER_TWEET_MESSAGE_MAX_LEN, text);
        
          tweetItem->next = feed->items;
          feed->items = tweetItem;
        }
      }
    }

    cx_json_tree_destroy (jsonTree);
    
    success = true;
  }
  else
  {
    CX_LOG_CONSOLE (1, "JSON parse error");
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

void feeds_weather_render (const feed_weather_t *feed, float x, float y, float z, float opacity)
{
  CX_ASSERT (feed);
  
  if (feed->dataReady && 
      (feed->conditionCode > WEATHER_CONDITION_CODE_INVALID) && 
      (feed->conditionCode < NUM_WEATHER_CONDITION_CODES))
  {
    cx_texture *image = g_weatherIcons [feed->conditionCode];
    CX_ASSERT (image);
    
    float w = (float) image->width;
    float h = (float) image->height;
    
    float x1 = x - (w * 0.5f);
    float y1 = y - (h * 0.5f);
    float x2 = x1 + w;
    float y2 = y1 + h;
    
    cx_colour colour = *cx_colour_white ();
    colour.a = opacity;
    
    cx_draw_quad (x1, y1, x2, y2, z, 0.0f, &colour, image);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_weather_data_cc_valid (const feed_weather_t *feed)
{
  CX_ASSERT (feed);
  
  return ((feed->conditionCode > WEATHER_CONDITION_CODE_INVALID) &&
          (feed->conditionCode < NUM_WEATHER_CONDITION_CODES));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void http_callback_weather (cx_http_request_id tId, const cx_http_response *response, void *userdata)
{
  CX_ASSERT (response);
  CX_ASSERT (userdata);
  
  feed_weather_t *feed = (feed_weather_t *) userdata;
  
  if (response->error == CX_HTTP_CONNECTION_ERROR)
  {
    feed->reqStatus = FEED_REQ_STATUS_ERROR;
    
    CX_LOG_CONSOLE (1, "http_callback_weather: Warning: no internet connection");
  }
  else
  {
    if ((response->statusCode == 200))
    {
      feeds_weather_clear (feed);
      
      bool valid =  response->data && (response->dataSize > 0);
      
      bool parsed = valid && feeds_weather_parse (feed, (const char *) response->data, response->dataSize);
      
      if (parsed)
      {
        feed->dataReady = true;
        feed->lastUpdate = cx_time_get_utc_epoch ();
        feed->reqStatus = FEED_REQ_STATUS_SUCCESS;
      }
      else
      {
        feed->reqStatus = FEED_REQ_STATUS_FAILURE;
      }
    }
    else
    {
      feeds_weather_clear (feed);
      feed->reqStatus = FEED_REQ_STATUS_FAILURE;
      
      char errorCode [32];
      cx_sprintf (errorCode, 32, "%d", response->statusCode);
      metrics_event_log (METRICS_EVENT_WEATHER_API_ERROR, errorCode);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_weather_search (feed_weather_t *feed, const char *query)
{
  CX_ASSERT (feed);
  CX_ASSERT (query);
  CX_ASSERT (feed->reqStatus == FEED_REQ_STATUS_INVALID);

  time_t currentTime = cx_time_get_utc_epoch ();
  
  if ((currentTime - feed->lastUpdate) > feed->ttlSecs)
  {
    feed->dataReady = false;
    feed->q = query;
    feed->reqStatus = FEED_REQ_STATUS_IN_PROGRESS;    
    
    // url
    
    const char *url = WEATHER_SEARCH_API_URL;
    
    // request
    
    char request [512];
    
  #if FEEDS_WEATHER_API_YAHOO
    cx_sprintf (request, 512, "%s?w=%s&u=c", url, query);
  #else
    cx_sprintf (request, 512, "%s/%s?cm_ven=LWO&cm_cat=rss&par=LWO_rss", url, query);
  #endif
    
    CX_LOG_CONSOLE (1, "%s", request);
    
    cx_http_get (request, NULL, 0, WEATHER_HTTP_REQUEST_TIMEOUT, http_callback_weather, feed);
    
    return true;
  }
  else
  {
    feed->reqStatus = FEED_REQ_STATUS_SUCCESS;
    
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void feeds_weather_clear (feed_weather_t *feed)
{
  CX_ASSERT (feed);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool feeds_weather_parse (feed_weather_t *feed, const char *data, int dataSize)
{
  CX_ASSERT (feed);
  CX_ASSERT (data);
  CX_ASSERT (dataSize > 0);
  
  bool success = false;
  
  // find > read temperature
  // find http:
  
  cx_xml_doc doc = cx_xml_doc_create (data, dataSize);
  
  if (doc)
  {  
    cx_xml_node rootNode = cx_xml_doc_root_node (doc);
    cx_xml_node channelNode = cx_xml_node_child (rootNode, "channel", NULL);
    
    if (channelNode)
    {
      cx_xml_node ttlNode = cx_xml_node_child (channelNode, "ttl", NULL);
      
      int ttlSecs = 0;
      
      if (ttlNode)
      {
        char *ttl = cx_xml_node_content (ttlNode);
        ttlSecs = atoi (ttl) * 60;
        cx_free (ttl);
      }
      
      cx_xml_node itemNode = cx_xml_node_child (channelNode, "item", NULL);
      
      if (itemNode)
      {
        cx_xml_node conditionNode = cx_xml_node_child (itemNode, "condition", "yweather");

        int tempCelsius = 0;
        int conditionCode = WEATHER_CONDITION_CODE_INVALID;
        
        if (conditionNode)
        {
          char *temp = cx_xml_node_attr (conditionNode, "temp");
          char *code = cx_xml_node_attr (conditionNode, "code");
          
          tempCelsius = atoi (temp);
          conditionCode = atoi (code);
          
          cx_free (temp);
          cx_free (code);
          
          feed->ttlSecs = ttlSecs;
          feed->celsius = tempCelsius;
          feed->conditionCode = conditionCode;  
        }
        else
        {
          feed->ttlSecs = 0;
        }
        
        success = true;
      }
    }
    
    cx_xml_doc_destroy (doc);
  }
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
