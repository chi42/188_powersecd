//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html
//
// A very simple demo client
// This program will connect to the socket and register itself to recieve
// signals. It will then sleep, waking up when it recieves a signal from the
// daemon whereupon the client will print status to STDOUT and go back to  
// sleep. 

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <stdint.h>
#include <errno.h>
#include <sys/select.h>

#define SOCK_PATH "/var/run/powersecd.sock"
int g_s;

void handler(int sign) 
{
  char buffer[10];
//  printf("SIG:\t");
  if( read(g_s, &buffer, 8) > 0) {
    buffer[8] = '\0'; 
    //scanf(buffer, "%d %d %d", a, b, c);
    printf("%s\n" , buffer);

  }
  else
    perror("SIGNAL, no data\n");

  return;
}

int main(void)
{
    int counter = 0;  
    int  t, len;
    struct sockaddr_un remote;
    char str[100];


    if ((g_s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(g_s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    write(g_s, "\x02", 1);
    signal(SIGUSR1, handler);
    printf("Connected.\n");

    while(1) 
	    select(0, NULL, NULL, NULL, NULL);
     
    return 0;
}
