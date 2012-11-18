//
//  cx_time.c
//
//  Created by Ubaka Onyechi on 10/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_time.h"
#include "cx_thread.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct cx_sys_time
{
  uint64_t prevTime;
  cxf64 deltaTime;
  cxf64 totalTime;
  bool update;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct cx_sys_time s_systemTime;
static cx_thread_mutex s_criticalSection; // for thread-unsafe and non-reentrant crt time functions
static mach_timebase_info_data_t s_timebase;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_system_time_init (void)
{
  cx_thread_mutex_init (&s_criticalSection);
  
  //struct timeval t;
  //gettimeofday (&t, NULL);  
  //s_systemTime.prevTime = ((uint64_t) t.tv_sec * 1000000) + (uint64_t) t.tv_usec; // microseconds
  
  mach_timebase_info (&s_timebase);
  s_systemTime.prevTime = mach_absolute_time ();
  s_systemTime.deltaTime = 0.0f;
  s_systemTime.totalTime = 0.0f;
  s_systemTime.update = true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_system_time_update (void)
{
  //struct timeval t;
  //gettimeofday (&t, NULL);  
  //u_int64_t currentTime = ((uint64_t) t.tv_sec * 1000000) + (uint64_t) t.tv_usec;
  
  u_int64_t currentTime = mach_absolute_time ();
  u_int64_t deltaTime = currentTime - s_systemTime.prevTime;
  s_systemTime.prevTime = currentTime;
  
  if (s_systemTime.update)
  {
    // microseconds to seconds
    //s_systemTime.deltaTime = (cxf64) deltaTime / (1000000.0f);
    //s_systemTime.totalTime += s_systemTime.deltaTime;
  
    // nanoseconds to seconds 
    s_systemTime.deltaTime = (cxf64) deltaTime * (cxf64) s_timebase.numer / (cxf64) s_timebase.denom / 1e9;
    s_systemTime.totalTime += s_systemTime.deltaTime;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_system_time_pause (bool pause)
{
  s_systemTime.update = !pause;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf64 cx_system_time_get_delta_time (void)
{
  return s_systemTime.deltaTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf64 cx_system_time_get_total_time (void)
{
  return s_systemTime.totalTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_time_get_unix_timestamp (cx_time_zone zone)
{
  CX_ASSERT ((zone > CX_TIME_ZONE_INVALID) && (zone < CX_NUM_TIME_ZONES));
  
  cxi32 timestamp = 0;
  
  cx_thread_mutex_lock (&s_criticalSection);
  
  switch (zone) 
  {
    case CX_TIME_ZONE_LOCAL:
    {
      time_t rawTime = time (NULL);
      CX_ASSERT (rawTime != -1);
      
      timestamp = (cxi32) rawTime;
      break;
    }
    
    case CX_TIME_ZONE_UTC:
    {
      time_t rawTime = time (NULL);
      CX_ASSERT (rawTime != -1);
      
      struct tm *gmt = gmtime (&rawTime);
      time_t utcTime = mktime (gmt);
      CX_ASSERT (utcTime != -1);
      
      timestamp = (cxi32) utcTime;
      break;
    }
      
    default:
    {
      break;
    }
  }
  
  cx_thread_mutex_unlock (&s_criticalSection);
  
  return timestamp;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxi32 cx_time_get_utc_offset (void)
{
  cx_thread_mutex_lock (&s_criticalSection);
  
  time_t rawTime = time (NULL);
  
  struct tm utcTm = *gmtime (&rawTime);
  struct tm locTm = *localtime (&rawTime);
  
  time_t utcTime = mktime (&utcTm);
  time_t locTime = mktime (&locTm);
  
  double diffSecs = difftime (locTime, utcTime);
  cxi32 diffHr = (cxi32) (diffSecs / (60.0f * 60.0f));
  
  cx_thread_mutex_unlock (&s_criticalSection);
  
  return diffHr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_time_set_date (cx_date *date, cx_time_zone zone)
{
  CX_ASSERT (date);
  CX_ASSERT ((zone > CX_TIME_ZONE_INVALID) && (zone < CX_NUM_TIME_ZONES));
  
  cx_thread_mutex_lock (&s_criticalSection); // gmtime not reentrant
  
  switch (zone) 
  {
    case CX_TIME_ZONE_LOCAL:
    {
      time_t rawTime = time (NULL);
      struct tm locTm = *localtime (&rawTime);
      time_t locTime = mktime (&locTm);
      
      date->calendar = locTm;
      date->unixTimestamp = (cxi32) locTime;
      
      break;
    }
      
    case CX_TIME_ZONE_UTC:
    {
      time_t rawTime = time (NULL);
      struct tm utcTm = *gmtime (&rawTime);
      time_t utcTime = mktime (&utcTm);
      
      date->calendar = utcTm;
      date->unixTimestamp = (cxi32) utcTime;
      
      break;
    }
      
    default:
    {
      date->unixTimestamp = 0;
      
      break;
    }
  }
  
  cx_thread_mutex_unlock (&s_criticalSection);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


void cx_time_start_timer (cx_timer *timer)
{
  CX_ASSERT (timer);
  
  timer->elapsedTime = 0.0f;
  timer->active = true;
  timer->startTime = mach_absolute_time ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_time_stop_timer (cx_timer *timer)
{
  CX_ASSERT (timer);
  
  // Reference: http://developer.apple.com/library/mac/#qa/qa1398/_index.html
  
  if (timer->active)
  {
    uint64_t endTime = mach_absolute_time ();
    
    uint64_t elapsed = endTime - timer->startTime;
    
    // convert to milliseconds
    
    mach_timebase_info_data_t timebase;
    
    mach_timebase_info (&timebase);
    
    timer->elapsedTime = ((cxf64) elapsed * (cxf64) timebase.numer / (cxf64) timebase.denom) / 1e6;
    
    timer->active = false;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_time_reset_timer (cx_timer *timer)
{
  CX_ASSERT (timer);
  
  timer->startTime = 0;
  timer->elapsedTime = 0.0f;
  timer->active = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
