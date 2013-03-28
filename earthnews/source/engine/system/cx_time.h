//
//  cx_time.h
//
//  Created by Ubaka Onyechi on 10/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_TIME_H
#define CX_TIME_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum cx_time_zone
{
  CX_TIME_ZONE_INVALID = -1,
  CX_TIME_ZONE_LOCAL,
  CX_TIME_ZONE_UTC,
  CX_NUM_TIME_ZONES
} cx_time_zone;

typedef struct cx_timer
{
  uint64_t startTime;
  cxf64 elapsedTime; /* milliseconds */
  bool active;
} cx_timer;

typedef struct cx_date
{
  struct tm calendar;
  cxi32 unixTimestamp;
} cx_date;

typedef enum cx_date_str_format
{
  CX_DATE_STR_FORMAT_HHMMSS,
  CX_DATE_STR_FORMAT_HHMMSSYYMMDD
} cx_date_str_format;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_time_get_unix_timestamp (cx_time_zone zone);
cxi32 cx_time_get_utc_offset (void);
void  cx_time_set_date (cx_date *date, cx_time_zone zone);
void  cx_time_get_date_string (cx_date *date, char *dst, cxu32 dstSize);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void  cx_time_start_timer (cx_timer *timer);
void  cx_time_stop_timer (cx_timer *timer);
void  cx_time_reset_timer (cx_timer *timer);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
