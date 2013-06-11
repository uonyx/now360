//
//  metrics.h
//  earthnews
//
//  Created by Ubaka Onyechi on 08/06/2013.
//  Copyright (c) 2013 uonyechi.com. All rights reserved.
//

#ifndef EARTHNEWS_METRICS_H
#define EARTHNEWS_METRICS_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum
{
  METRICS_EVENT_LOAD_BEGIN,
  METRICS_EVENT_LOAD_END,
  METRICS_EVENT_MUSIC_QUEUE_1,
  METRICS_EVENT_MUSIC_QUEUE_2,
  METRICS_EVENT_FIRST_CITY,
  METRICS_EVENT_CLICK_NEWS,
  METRICS_EVENT_OPEN_TWITTER_LINK,
  METRICS_EVENT_LINK_SHARE,
  METRICS_EVENT_APP_BG,
  METRICS_EVENT_APP_FG,
  METRICS_EVENT_TWITTER_API_ERROR_403,
  METRICS_EVENT_TWITTER_API_ERROR_406,
  METRICS_EVENT_TWITTER_API_ERROR_410,
} metrics_event_type_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void metrics_init (void);
void metrics_deinit (void);
void metrics_event_log (metrics_event_type_t eventType, void *data);
void metrics_disable_tracking (bool flag);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
