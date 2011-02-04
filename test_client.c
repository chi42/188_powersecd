//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

#define SOCK_PATH "/var/run/powersecd.socket"

void handler(int sign) 
{
        //printf("SIGNAL reccieved!\n");
    return;
}

int main(void)
{
    int s, t, len;
    struct sockaddr_un remote;
    char str[100];

    signal(SIGUSR1, handler);

    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    printf("Trying to connect...\n");

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    printf("Connected.\n");
    write(s, "a", 1);


    while(1) {

      sleep(10);
      if (errno == EINTR)
        printf("SIGNAL reccieved!\n");
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