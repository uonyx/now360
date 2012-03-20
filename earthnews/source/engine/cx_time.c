//
//  cx_time.c
//  earthnews
//
//  Created by Ubaka  Onyechi on 10/01/2012.
//  Copyright (c) 2012 SonOfLagos. All rights reserved.
//

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_time.h"
#include <mach/mach.h>
#include <mach/mach_time.h>
#include <sys/time.h>
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

struct cx_sys_time
{
  uint64_t prevTime;
  cxf64 deltaTime;
  cxf64 totalTime;
  BOOL update;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static struct cx_sys_time s_systemTime;
static mach_timebase_info_data_t s_timebase;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_time_global_init (void)
{
  //struct timeval t;
  //gettimeofday (&t, NULL);  
  //s_systemTime.prevTime = ((uint64_t) t.tv_sec * 1000000) + (uint64_t) t.tv_usec; // microseconds
  
  mach_timebase_info (&s_timebase);
  s_systemTime.prevTime = mach_absolute_time ();
  s_systemTime.deltaTime = 0.0f;
  s_systemTime.totalTime = 0.0f;
  s_systemTime.update = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_time_global_update (void)
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

void cx_time_global_pause (BOOL pause)
{
  s_systemTime.update = !pause;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf64 cx_time_global_delta_time (void)
{
  return s_systemTime.deltaTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cxf64 cx_time_global_total_time (void)
{
  return s_systemTime.totalTime;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_timer_reset (cx_timer *timer)
{
  CX_ASSERT (timer);
  
  timer->startTime = 0;
  timer->elapsedTime = 0.0f;
  timer->active = FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_timer_start (cx_timer *timer)
{
  CX_ASSERT (timer);
  
  timer->elapsedTime = 0.0f;
  timer->active = TRUE;
  timer->startTime = mach_absolute_time ();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_timer_stop (cx_timer *timer)
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
    
    timer->active = FALSE;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////


