#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "ps_list.h"


int ps_list_init(ps_list *list)
{
  if (!list) 
    return -1;

  list->size = 0;
  list->head = NULL;
  list->curr = NULL;

  return 0;
}

int ps_list_add(ps_list *list, client_node* node)
{
  if (!list || !node)
    return -1;

  if (list->head) 
    list->head->prev = node;

  list->size += 1;
  node->next = list->head;
  list->head = node;

    return 0;
}


int ps_list_del(ps_list *list, client_node* node)
{
  client_node *a, *b;
  if (!list || !node) 
    return -1;
  
  a = node->prev;
  b = node->next;

  list->size -= 1;

  if (a) 
    a->next = b;
  else 
    list->head = b; 
  if (b)
    b->prev = a;

  if (list->curr == node) 
    list->curr = a;

  free(node);  

  return 0;
}

int ps_list_free(ps_list *list)
{
  client_node *a, *b;
  
  if (list->head) { 
    a = list->head;

    while(a) {
      b = a->next;
      free(a);
      a = b;   
    }
  } 

  return 0;
}

client_node* ps_list_next(ps_list *list)
{
  if (!(list->curr))
    list->curr = list->head;
  else 
    list->curr = list->curr->next;

  return list->curr;
}


