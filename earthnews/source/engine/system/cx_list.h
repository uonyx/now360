//
//  cx_list.h
//
//  Created by Ubaka Onyechi on 18/08/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#ifndef CX_LIST_H
#define CX_LIST_H

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cx_system.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#define CX_SINGLY_LINKED_LIST 0
#define CX_DOUBLY_LINKED_LIST 1

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef int (*cx_list_sort_cmp_func) (const void *, const void *);

#if CX_SINGLY_LINKED_LIST
typedef struct cx_list_node
{
  const void *data;
  struct cx_list_node *next;
} cx_list_node;

typedef cx_list_node cx_list;

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_list *cx_list_insert (cx_list *list, const void *data);
cx_list *cx_list_remove (cx_list *list, const void *data);
cx_list *cx_list_reverse (cx_list *list);
cx_list *cx_list_sort (cx_list *list, cx_list_sort_cmp_func cmpfunc);
cxu32 cx_list_to_array (cx_list *list, cx_list_node *dest, int cxu32);
bool cx_list_exists (cx_list *list, const void *data);
void cx_list_free (cx_list *list);
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if CX_DOUBLY_LINKED_LIST
typedef struct cx_list2_node
{
  const void *data;
  struct cx_list2_node *next;
  struct cx_list2_node *prev;
} cx_list2_node;

typedef struct cx_list2
{
  struct cx_list2_node *head;
  struct cx_list2_node *tail;
} cx_list2;

void cx_list2_init (cx_list2 *list);
void cx_list2_deinit (cx_list2 *list);
void cx_list2_insert_front (cx_list2 *list, const void *data);
void cx_list2_insert_back (cx_list2 *list, const void *data);
bool cx_list2_remove (cx_list2 *list, const void *data);
bool cx_list2_exists (cx_list2 *list, const void *data);
void cx_list2_sort (cx_list2 *list, cx_list_sort_cmp_func cmpfunc);

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif
