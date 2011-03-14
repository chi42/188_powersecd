// just for testing the linked list functions 

#include "../ps_list.h"

#include <stdio.h>
#include <stdlib.h>


int main ()
{
  ps_list lis;
  int i;
  client_node *n;

  ps_list_init(&lis);

  for(i = 0; i < 20; ++i) {
    n = malloc(sizeof(client_node));
    n->c_fd = i;
    ps_list_add(&lis, n);
  }

  printf("size %d\n", lis.size);  

  while(n = ps_list_next(&lis)) 
    printf("next %d\n", n->c_fd);

  printf("\n\n");

  while(n = ps_list_next(&lis)) {
    printf("next %d\n", n->c_fd);
    if( !(n->c_fd % 2 ))
      ps_list_del(&lis, n);
  }

  printf("size %d\n", lis.size);  

  while(n = ps_list_next(&lis)) 
    printf("next %d\n", n->c_fd);

  printf("\n\n");

  ps_list_free(&lis);
  
  printf("size %d\n", lis.size);  

  return 0;
}


