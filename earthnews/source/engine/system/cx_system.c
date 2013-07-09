//
//  cx_system.c
//
//  Created by Ubaka Onyechi on 14/01/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SYSTEM_MEMORY_SIZE_ALIGN (4)
#define CX_MALLOC_STD_C 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static bool g_initialised = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_system_memory_init (void)    
{ 
#if CX_SYSTEM_CUSTOM_MEMORY_ALLOCATOR_ENABLED
  CX_ERROR ("cx_system_memory_init: NOT YET IMPLEMENTED!");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void cx_system_memory_deinit (void)  
{ 
#if CX_SYSTEM_CUSTOM_MEMORY_ALLOCATOR_ENABLED
  CX_ERROR ("cx_system_memory_deinit: NOT YET IMPLEMENTED!");
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void *cx_malloc_memset (void *data, cxu32 fill, cxu32 size);

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_system_init (void)
{
  cx_system_memory_init ();
  cx_system_time_init ();
  
  g_initialised = true;
  
  return g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool _cx_system_deinit (void)
{
  cx_system_memory_deinit ();
  
  g_initialised = false;
  
  return !g_initialised;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void *_cx_malloc (size_t size)
{
  CX_ASSERT (g_initialised);
  
#if 0
  
  void *block = malloc (size);
  
  memset (block, 0, size);
  
  CX_FATAL_ASSERT (block);
  
  return block;
  
#else
  
  int pad = (CX_SYSTEM_MEMORY_SIZE_ALIGN - (size % CX_SYSTEM_MEMORY_SIZE_ALIGN));
  
  int allocsize = size + pad;
  
  void *block = malloc (allocsize);
  
#if (CX_DEBUG || 1)
  cx_malloc_memset (block, 0xDEADBEEF, allocsize);
#endif
  
  CX_FATAL_ASSERT (block);
  
  return block;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void _cx_free (void *data)
{
  CX_ASSERT (g_initialised);
  
  CX_ASSERT (data);
  //CX_ASSERT (data != 0xDEADBEEF);
  
  free (data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

static void *cx_malloc_memset (void *data, cxu32 fill, cxu32 size)
{
  CX_ASSERT (g_initialised);
  
  cxu32 *d32 = data;
  cxu32 cntr = size;
  
  while (cntr)
  {
    *d32++ = fill;
    cntr -= 4;
  }
  
  return data;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
