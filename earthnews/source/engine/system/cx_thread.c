//
//  cx_thread.c
//
//  Copyright (c) 2012 Ubaka Onyechi. All rights reserved.
//

#include "cx_thread.h"
#include "cx_string.h"
#include <unistd.h>

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void *cx_thread_pthread_func (void *data)
{
  CX_ASSERT (data);
  
  cx_thread *thread = (cx_thread *) data;
  
  pthread_setname_np (thread->name);
  
  cx_thread_monitor_wait (&thread->start);
  
  cx_thread_exit_status exitStatus = CX_THREAD_EXIT_STATUS_SUCCESS;
  
  if (thread->func)
  {
    exitStatus = thread->func (thread->funcUserData);
  }
  
  pthread_exit ((void *) exitStatus);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_thread *cx_thread_create (const char *name, cx_thread_type type, cx_thread_func func, void *userdata)
{
  CX_ASSERT (func);
  
  cx_thread *thread = cx_malloc (sizeof (cx_thread));
  
  thread->func = func;
  thread->funcUserData = userdata;
  thread->name = name;
  thread->type = type;
  
  cx_thread_monitor_init (&thread->start);
  
  pthread_attr_t attr;
  pthread_attr_init (&attr);
  pthread_attr_setdetachstate (&attr, type);
  
  int rc = pthread_create (&thread->id, &attr, cx_thread_pthread_func, thread);
  
  if (rc != 0)
  {
    cx_free (thread);
    thread = NULL;
  }
  
  pthread_attr_destroy (&attr);
  
  return thread;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_destroy (cx_thread *thread)
{
  CX_ASSERT (thread);
 
  cx_thread_monitor_deinit (&thread->start);
  
  cx_free (thread);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_start (cx_thread *thread)
{
  CX_ASSERT (thread);
  
  cx_thread_monitor_signal (&thread->start);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_cancel (cx_thread *thread)
{
  CX_ASSERT (thread);
  
  //pthread_cancel (thread->id);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_join (cx_thread *thread, cx_thread_exit_status *exitStatus)
{
  CX_ASSERT (thread);
  
  void *status = NULL;
  
  int rc = pthread_join (thread->id, &status);
  
  CX_REF_UNUSED (rc);
  
  if (exitStatus)
  {
    *exitStatus = (cx_thread_exit_status) status;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_detach (cx_thread *thread)
{
  CX_ASSERT (thread);
  
  int rc = pthread_detach (thread->id); 
  
  CX_REF_UNUSED (rc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_sleep (cxu32 millisecs)
{
  usleep (millisecs * 1000);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_thread_mutex_init (cx_thread_mutex *mutex)
{
  CX_ASSERT (mutex);
  
  int rc = pthread_mutex_init (mutex, NULL);
  
  return (rc == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_thread_mutex_deinit (cx_thread_mutex *mutex)
{
  CX_ASSERT (mutex);
  
  int rc = pthread_mutex_destroy (mutex);
  
  return (rc == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_mutex_lock (cx_thread_mutex *mutex)
{
  CX_ASSERT (mutex);
  
  int rc = pthread_mutex_lock (mutex);
  
  CX_REF_UNUSED (rc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_mutex_unlock (cx_thread_mutex *mutex)
{
  CX_ASSERT (mutex);
  
  int rc = pthread_mutex_unlock (mutex);
  
  CX_REF_UNUSED (rc);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_thread_monitor_init (cx_thread_monitor *monitor)
{
  CX_ASSERT (monitor);
  
  bool ret = false;
  
  int rc = pthread_mutex_init (&monitor->mutex, NULL);
  
  if (rc == 0)
  {
    rc = pthread_cond_init (&monitor->cond, NULL);
    
    ret = (rc == 0);
  }
  
  monitor->sigcount = 0;
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_thread_monitor_deinit (cx_thread_monitor *monitor)
{
  CX_ASSERT (monitor);
  
  bool ret = false; 
  
  int rc = pthread_mutex_destroy (&monitor->mutex);
  
  ret = (rc == 0);
  
  rc = pthread_cond_destroy (&monitor->cond);
  
  ret &= (rc == 0);
  
  return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_monitor_signal (cx_thread_monitor *monitor)
{
  CX_ASSERT (monitor);
  
  int rc = pthread_mutex_lock (&monitor->mutex);
  
  monitor->sigcount++; // sigcount: protects against logical error of calling 'signal' before 'wait' & useful for debug
  
  rc = pthread_cond_signal (&monitor->cond);
  
  rc = pthread_mutex_unlock (&monitor->mutex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_thread_monitor_wait (cx_thread_monitor *monitor)
{
  CX_ASSERT (monitor);
  
  int rc = pthread_mutex_lock (&monitor->mutex);
  
  if (monitor->sigcount == 0)
  {
    rc = pthread_cond_wait (&monitor->cond, &monitor->mutex);
  }
  
  monitor->sigcount--;
  
  rc = pthread_mutex_unlock (&monitor->mutex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_thread_monitor_wait_timed (cx_thread_monitor *monitor, cxu32 timeout)
{
  CX_ASSERT (monitor);
  
  struct timeval tv;
  struct timespec ts;
  
  gettimeofday (&tv, NULL);
  
  ts.tv_sec = tv.tv_sec + 0;
  ts.tv_nsec = (tv.tv_usec + (timeout * 1000)) * 1000;
  
  int retc = 0;
  
  int rc = pthread_mutex_lock (&monitor->mutex);
  
  if (monitor->sigcount == 0)
  {
    retc = pthread_cond_timedwait (&monitor->cond, &monitor->mutex, &ts);
    
    if (retc == 0)
    {
      monitor->sigcount--;
    }
  }
  else
  {
    monitor->sigcount--;
  }
  
  rc = pthread_mutex_unlock (&monitor->mutex);
  
  return (retc == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
