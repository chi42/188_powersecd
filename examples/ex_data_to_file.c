//http://beej.us/guide/bgipc/output/html/multipage/unixsock.html
// Another similarly simple test client. This client will connect to 
// the daemon and block until it recieves a signal. Results are 
// printed out to DATA_FILE (see below), not to STDOUT. This
// is just to make daemon easier to use for certain applications. It
// really isn't advised to use this too often. 
//
// Note, that the lseek is reset to the top of the file before every
// write. This prevents the file from growing larger, and causes old
// data to be overwritten.

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
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define SOCK_PATH "/var/run/powersecd.sock"
#define DATA_FILE "/ramdisk/ps_dat"

int g_s, g_fd;

void handler(int sign) 
{
  char buffer[10];
  if( read(g_s, &buffer, 8) > 0) {
    buffer[8] = '\n';
    lseek(g_fd, 0, SEEK_SET);
    write(g_fd, buffer, 9);
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

    g_fd = open(DATA_FILE, O_CREAT | O_RDWR);
    if (g_fd < 0) {
      perror("ramdisk dat file");
      exit(1);
    } 

    if ((g_s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }

    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, SOCK_PATH);
    len = strlen(remote.sun_path) + sizeof(remote.sun_family);
    if (connect(g_s, (struct sockaddr *)&remote, len) == -1) {
        perror("connect");
        exit(1);
    }

    write(g_s, "\x02", 1);
    signal(SIGUSR1, handler);
    printf("Connected, will write data to ramdisk file.\n");


    while(1) {
	    select(0, NULL, NULL, NULL, NULL);
    }
     
    return 0;
}

