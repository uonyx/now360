//
//  cx_list.c
//
//  Created by Ubaka Onyechi on 18/08/2012.
//  Copyright (c) 2012 uonyechi.com. All rights reserved.
//

#include "cx_list.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_list *cx_list_insert (cx_list *list, const void *data)
{
  // assert data is not on the stack
  
  cx_list_node *node = (cx_list_node *) cx_malloc (sizeof (cx_list_node));
  
  cx_list_node *head = list;
  
  node->data = data;
  
  node->next = head;
  
  head = node;
  
  return head;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_list *cx_list_remove (cx_list *list, const void *data)
{
  cx_list_node *head = list;
  
  if (head->data == data)
  {
    cx_list_node *next = head->next;
    cx_free (head);
    head = next;
  }
  else
  {
    cx_list_node *prev = head;
    cx_list_node *curr = head->next;
    
    while (curr)
    {
      if (curr->data == data)
      {
        prev->next = curr->next;
        cx_free (curr);
        break;
      }
      
      prev = curr;
      curr = curr->next;
    }
  }
  
  return head;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_list *cx_list_reverse (cx_list *list)
{
  cx_list_node *head = list;
  cx_list_node *prev = NULL;
  
  while (head)
  {
    cx_list_node *next = head->next;
    
    head->next = prev;
    
    prev = head;
    head = next;
  }
  
  return prev;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

cx_list *cx_list_sort (cx_list *list, cx_list_sort_cmp_func cmpfunc)
{
  CX_FATAL_ERROR ("Not implemented");
  
  cx_list_node *head = list;
  
  return head;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

bool cx_list_exists (cx_list *list, const void *data)
{
  cx_list_node *node = list;
  
  while (node)
  {
    if (node->data == data)
    {
      return true;
    }
  }
  
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

void cx_list_free (cx_list *list)
{
  cx_list_node *node = list;
  
  while (node)
  {
    cx_list_node *next = node->next;
    
    cx_free (node);
    
    node = next;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////

#if 0

void cx_list2_insert_front (cx_list2 *list, const void *data)
{
  cx_list_node2 *node = (cx_list_node2 *) cx_malloc (sizeof (cx_list_node2));
  
  node->data = data;
  
  if (!list->head)
  {
    list->tail = node;
    list->head = node;
  }
  else
  {
    node->next = list->head;
    list->head->prev = node;
    list->head = node;
  }
}

void cx_list2_insert_back (cx_list2 *list, const void *data)
{
  cx_list_node2 *node = (cx_list_node2 *) cx_malloc (sizeof (cx_list_node2));
  
  node->data = data;
  
  if (!list->tail)
  {
    list->head = node;
    list->tail = node;
  }
  else
  {
    node->prev = list->tail;
    list->tail->next = node;
    list->tail = node;
  }
}

bool cx_list2_remove (cx_list2 *list, const void *data)
{
  bool found = false;
  
  if (list->head)
  {
    if (list->head->data == data)
    {
      cx_list_node2 *next = list->head->next;
      cx_free (list->head);
      
      next->prev = NULL;
      list->head = next;
      
      found = true;
    }
    else if (list->tail->data == data)
    {
      cx_list_node2 *prev = list->tail->prev;
      cx_free (list->tail);
      
      prev->next = NULL;
      list->tail = prev;
      
      found = true;
    }
    else
    {
      cx_list_node2 *curr = list->head->next;
      
      while (curr)
      {
        if (curr->data == data)
        {
          curr->prev->next = curr->next;
          curr->next->prev = curr->prev;
          
          cx_free (curr);
          found = true;
          
          break;
        }
        
        curr = curr->next;
      }
    }
  }
  
  return found;
}

#endif