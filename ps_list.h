#ifndef __ps_list_h__
#define __ps_list_h__

#include <stdint.h>

typedef struct client_node_t {
  int c_fd;
  int pid;
  uint8_t first;
  struct client_node_t *next;
  struct client_node_t *prev;
} client_node;

typedef struct ps_list_t {
  unsigned int size;
  client_node *head;
  client_node *curr;
} ps_list;

int ps_list_init(ps_list *list);
int ps_list_del(ps_list *list, client_node* node);
int ps_list_add(ps_list *list, client_node* node);
int ps_list_free(ps_list *list);
client_node* ps_list_next(ps_list *list);

#endif


