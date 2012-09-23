//
//  worker.c
//  earthnews
//
//  Created by Ubaka Onyechi on 18/08/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "worker.h"
#include "../engine/cx_engine.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct task_t
{
  task_func func;
  void *userdata;
  task_status *status;
  struct task_t *next;
} task_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TASK_MAX_COUNT (16)

static task_t *s_taskPoolArray = NULL;

static task_t *s_taskFreeList = NULL;
static task_t *s_taskBusyList = NULL;
static int s_taskFreeListCount = 0;
static int s_taskBusyListCount = 0;

static cx_thread *s_thread = NULL;
static cx_thread_monitor s_threadMonitor;
static cx_thread_mutex s_sharedDataMutex;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static task_t *task_list_insert_back (task_t *head, task_t *task, int *count)
{
  if (head)
  {
    task_t *curr = head;
    
    while (curr && curr->next)
    {
      curr = curr->next;
    }
    
    curr->next = task;
  }
  else
  {
    head = task;
  }

  ++(*count);
  
  return head;
}

static task_t *task_list_pop_front (task_t *head, task_t **front, int *count)
{
  *front = head;

  if (head)
  {
    head = head->next;

    --(*count);
  }
  
  return head;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static cx_thread_exit_status worker_thread_func (void *data)
{
  cx_thread_exit_status exitStatus = CX_THREAD_EXIT_STATUS_SUCCESS;
  
  while (1)
  {
    cx_thread_monitor_wait (&s_threadMonitor);
    
    cx_thread_mutex_lock (&s_sharedDataMutex);
    
    task_t *task = NULL;
    
    s_taskBusyList = task_list_pop_front (s_taskBusyList, &task, &s_taskBusyListCount);
    
    while (task)
    {
      task->func (task->userdata);
    
      if (task->status)
      {
        *task->status = TASK_STATUS_COMPLETE;
      }
      
      task->func = NULL;
      task->userdata = NULL;
      task->next = NULL;
      
      s_taskFreeList = task_list_insert_back (s_taskFreeList, task, &s_taskFreeListCount);
      s_taskBusyList = task_list_pop_front (s_taskBusyList, &task, &s_taskBusyListCount);
    }
    
    cx_thread_mutex_unlock (&s_sharedDataMutex);
  }
  
  return exitStatus;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void worker_init (void)
{
  s_taskPoolArray = (task_t *) cx_malloc (sizeof (task_t) * TASK_MAX_COUNT);
  
  unsigned int i;
  
  for (i = 0; i < TASK_MAX_COUNT; ++i)
  {
    task_t *task = &s_taskPoolArray [i];
    
    s_taskFreeList = task_list_insert_back (s_taskFreeList, task, &s_taskFreeListCount);
  }
  
  cx_thread_mutex_init (&s_sharedDataMutex);
  
  cx_thread_monitor_init (&s_threadMonitor);
  
  s_thread = cx_thread_create ("earthnews worker thread", CX_THREAD_TYPE_JOINABLE, worker_thread_func, NULL);
  
  cx_thread_start (s_thread);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void worker_deinit (void)
{
  cx_thread_destroy (s_thread);
  
  cx_thread_monitor_deinit (&s_threadMonitor);
  
  cx_thread_mutex_deinit (&s_sharedDataMutex);
  
  cx_free (s_taskPoolArray);
  
  s_thread = NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void worker_update (void)
{
  CX_FATAL_ASSERT (s_thread);
  
  cx_thread_mutex_lock (&s_sharedDataMutex);
  
  if (s_taskBusyListCount > 0)
  {
    cx_thread_monitor_signal (&s_threadMonitor);
  }
  
  cx_thread_mutex_unlock (&s_sharedDataMutex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool worker_add_task (task_func func, void *userdata, task_status *status)
{
  CX_FATAL_ASSERT (s_thread);
  CX_ASSERT (func);
  
  bool success = false;
  
  task_t *task = NULL;
  
  cx_thread_mutex_lock (&s_sharedDataMutex);
  
  s_taskFreeList = task_list_pop_front (s_taskFreeList, &task, &s_taskFreeListCount);
  
  if (task)
  {
    if (status)
    {
      *status = TASK_STATUS_INPROGRESS;
    }
    
    task->func = func;
    task->userdata = userdata;
    task->status = status;
    
    s_taskBusyList = task_list_insert_back (s_taskBusyList, task, &s_taskBusyListCount);
    
    success = true;
  }
  else
  {
    if (status)
    {
      *status = TASK_STATUS_INVALID;
    }
  }
  
  cx_thread_mutex_unlock (&s_sharedDataMutex);
  
  return success;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////