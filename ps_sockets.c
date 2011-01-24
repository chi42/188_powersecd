#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include  "ps_sockets.h"

int socket_create(const char *soc_name) 
{
  int fd, res;

  struct sockaddr_un s_addr;
  
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

