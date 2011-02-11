//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html

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

#define SOCK_PATH "/var/run/powersecd.sock"
int g_s;

void handler(int sign) 
{
  char buffer[10];
  printf("SIG:\t");
  if( read(g_s, &buffer, 8) > 0) {
    buffer[8] = '\0';
    printf("%s\n" , buffer);
  }
  else
    perror("SIGNAL, no data\n");

  return;
}

int main(void)
{
  
    int  t, len;
    struct sockaddr_un remote;
    char str[100];


    if ((g_s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(g_s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    write(g_s, "\x02", 1);
    signal(SIGUSR1, handler);
    printf("Connected.\n");


    while(1) {
      if(sleep(10))
	printf("INTERRUPT!\n");
    }
      
    //while(printf("> "), fgets(str, 100, stdin), !feof(stdin)) {
    //    if (send(s, str, strlen(str), 0) == -1) {
    //        perror("send");
    //        exit(1);
    //    }

    //    if ((t=recv(s, str, 100, 0)) > 0) {
    //        str[t] = '\0';
    //        printf("echo> %s", str);
    //    } else {
    //        if (t < 0) perror("recv");
    //        else printf("Server closed connection\n");
    //        exit(1);
    //    }
    //}


    return 0;
}
