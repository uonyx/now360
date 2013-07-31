//
//  metrics.c
//  earthnews
//
//  Created by Ubaka Onyechi on 08/06/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "metrics.h"
#include "../engine/utility/cx_analytics.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_initialised = false;
bool g_disableTracking = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void metrics_init (void)
{
  CX_ASSERT (!g_initialised);
  
  cx_analytics_session_data data;
  
  data.flurry.apiKey = "TKG2869N5WSZMN3ZYZRR";
  
  cx_analytics_session_begin (&data);
  
  g_initialised = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void metrics_deinit (void)
{
  CX_ASSERT (g_initialised);
  
  cx_analytics_session_end ();
  
  g_initialised = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void metrics_event_log (metrics_event_type_t eventType, void *data)
{
  if (!g_disableTracking)
  {
    cxi64 utcTimestamp = cx_time_get_utc_epoch ();
    char utcTimestampStr [32];
    cx_sprintf (utcTimestampStr, 32, "%lld", utcTimestamp);
    
    const char *timstamp = &utcTimestampStr [0];
    
    switch (eventType)
    {
      case METRICS_EVENT_LOAD_BEGIN:
      {
        const char *e = "load_begin";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }

      case METRICS_EVENT_LOAD_END:
      {
        const char *e = "load_end";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_MUSIC_QUEUE_1:
      {
        const char *e = "music_queue";
        const char *pn [2] = { "timestamp", "button" };
        const char *pv [2] = { timstamp, "toggle" };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
        break;
      }
        
      case METRICS_EVENT_MUSIC_QUEUE_2:
      {
        const char *e = "music_queue";
        const char *pn [2] = { "timestamp", "button" };
        const char *pv [2] = { timstamp, "queue" };
        
        cx_analytics_log_event (e, pn, pv, 2);
        break;
      }

      case METRICS_EVENT_FIRST_CITY:
      {
        CX_ASSERT (data);
        
        const char *city = (const char *) data;
        
        const char *e = "first_city";
        const char *pn [2] = { "timestamp", "name" };
        const char *pv [2] = { timstamp, city };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
        break;
      }
        
      case METRICS_EVENT_CLICK_NEWS:
      {
        const char *e = "click_news";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_OPEN_TWITTER_LINK:
      {
        const char *e = "open_twitter_link";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_LINK_SHARE:
      {
        CX_ASSERT (data);
        
        const char *medium = (const char *) data;
        
        const char *e = "link_share";
        const char *pn [2] = { "timestamp", "medium" };
        const char *pv [2] = { timstamp, medium };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
        break;
      }
    
      case METRICS_EVENT_APP_BG:
      {
        const char *e = "app_bg";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_APP_FG:
      {
        const char *e = "app_fg";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_APP_TERM:
      {
        const char *e = "app_mem_term";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
        
      case METRICS_EVENT_APP_MEMORY_WARNING:
      {
        const char *e = "app_mem_warning";
        const char *pn = "timestamp";
        const char *pv = timstamp;
        
        cx_analytics_log_event (e, &pn, &pv, 1);
        
        break;
      }
       
      case METRICS_EVENT_TWITTER_API_ERROR:
      {
        const char *error = data ? (const char *) data : "";
        
        const char *e = "twitter_api_error";
        const char *pn [2] = { "timestamp", "errorCode" };
        const char *pv [2] = { timstamp, error };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
        break;
      }
        
      case METRICS_EVENT_WEATHER_API_ERROR:
      {
        const char *error = data ? (const char *) data : "";
        
        const char *e = "weather_api_error";
        const char *pn [2] = { "timestamp", "errorCode" };
        const char *pv [2] = { timstamp, error };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
        break;
      }

      case METRICS_EVENT_NEWS_API_ERROR:
      {
        const char *error = data ? (const char *) data : "";
        
        const char *e = "news_api_error";
        const char *pn [2] = { "timestamp", "errorCode" };
        const char *pv [2] = { timstamp, error };
        
        cx_analytics_log_event (e, pn, pv, 2);
        
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

void metrics_disable_tracking (bool enabled)
{
  g_disableTracking = enabled;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
