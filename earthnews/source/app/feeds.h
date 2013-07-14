//
//  feeds.h
//  earthnews
//
//  Created by Ubaka Onyechi on 14/07/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef EARTHNEWS_FEEDS_H
#define EARTHNEWS_FEEDS_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FEEDS_WEATHER_API_YAHOO   1
#define FEEDS_TWITTER_API_1_0     0

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  FEED_REQ_STATUS_INVALID,
  FEED_REQ_STATUS_IN_PROGRESS,
  FEED_REQ_STATUS_SUCCESS,
  FEED_REQ_STATUS_FAILURE,
  FEED_REQ_STATUS_ERROR,
} feed_req_status_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FEED_NEWS_TITLE_MAX_LEN (128)
#define FEED_NEWS_LINK_MAX_LEN (256)

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct feed_news_item_t
{
  struct
  {
    int mon;
    int mday;
    int secs;
  } pubDateInfo;
  
  char title [FEED_NEWS_TITLE_MAX_LEN];
  char link [FEED_NEWS_LINK_MAX_LEN];
  struct feed_news_item_t *next;
} feed_news_item_t;

typedef struct feed_news_t
{
  const char *query;
  feed_news_item_t *items;
  cxi64 lastUpdate;
  feed_req_status_t reqStatus;
  cx_http_request_id httpReqId;
  char link [FEED_NEWS_LINK_MAX_LEN];
} feed_news_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FEED_TWITTER_TWEET_USERNAME_MAX_LEN (32)
#define FEED_TWITTER_TWEET_MESSAGE_MAX_LEN (512) //over-compensating for multibyte chars

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct feed_twitter_tweet_t
{
  char username [FEED_TWITTER_TWEET_USERNAME_MAX_LEN];
  char text [FEED_TWITTER_TWEET_MESSAGE_MAX_LEN];
  struct feed_twitter_tweet_t *next;
} feed_twitter_tweet_t;

typedef struct feed_twitter_t
{
  const char *query;
  feed_twitter_tweet_t *items;
  cxi64 lastUpdate;
  feed_req_status_t reqStatus;
  cx_http_request_id httpReqId;
#if FEEDS_TWITTER_API_1_0
  unsigned long long maxId;
#else
  char maxIdStr [32];
#endif
} feed_twitter_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct feed_weather_t
{  
  const char *q;
  int celsius;
  int conditionCode;
  cxi64 lastUpdate;
  int ttlSecs;
  bool dataReady;
  feed_req_status_t reqStatus;
} feed_weather_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_init (void);
void feeds_deinit (void);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_news_search (feed_news_t *feed, const char *query);
void feeds_news_cancel_search (feed_news_t *feed);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void feeds_twitter_search (feed_twitter_t *feed, const char *query, bool local, float lat, float lon);
void feeds_twitter_cancel_search (feed_twitter_t *feed);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool feeds_weather_search (feed_weather_t *feed, const char *query);
bool feeds_weather_data_cc_valid (const feed_weather_t *feed);
void feeds_weather_render (const feed_weather_t *feed, float x, float y, float z, float opacity);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

