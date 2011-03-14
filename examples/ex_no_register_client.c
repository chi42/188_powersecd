//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html
// 
// Even simpler then the other example clients.
// This connects for data only (no signals), recieves the data and
// then immediately prints the data out to STDOUT and dies

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


int main(void)
{
  
    int t, len, g_s;
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

    write(g_s, "\x01", 1);
    printf("Connected.\n");

	
    read(g_s, str, 8);
    printf("recieved message: %s\n", str);
          
    return 0;
}

