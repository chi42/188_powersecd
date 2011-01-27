#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <errno.h>

#include  "ps_sockets.h"

// for testing purpose only...
//#include <stdio.h>

int ps_create(const char *soc_name) 
{
  int fd, res;
  struct sockaddr_un s_addr;

  // just in case
  unlink(soc_name);
  
  fd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (fd < 0) 
    return fd;

  memset(&s_addr, 0, sizeof(s_addr));
  s_addr.sun_family = AF_UNIX;
  strcpy(s_addr.sun_path, soc_name);
 
  // bind to port 
  res = bind(fd, (struct sockaddr *)&s_addr, sizeof(s_addr));
  if (res < 0) 
    return res; 

  res = listen(fd, MAX_LISTEN);
  if (res < 0) 
    return res;

  return fd;
}

int ps_accept(int fd, struct ps_ucred *cred) 
{
  int new_fd, len;
  struct sockaddr_un client;

  len = sizeof(struct sockaddr_un);  

  while (1) {
    new_fd = accept(fd, (struct sockaddr *)&client, &len);
    if (new_fd < 0) {
      // if accept failed because of an interrupt, we kee  looping...
      if (errno == EINTR) 
        continue;
      else
        return new_fd;
    }

    if (cred) {
      len = sizeof(struct ps_ucred);
      getsockopt(new_fd, SOL_SOCKET, SO_PEERCRED, cred, &len);
    }
    return new_fd;
  }

  // this will probably keep some compilers happy (i think) 
  return -1;
}

