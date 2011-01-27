#ifndef __PS_SOCKETS_H__
#define __PS_SOCKETS_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#define MAX_LISTEN 10

//FIXME
// this is a terrible thing to do, but I cannot find ucred anywhere
// so I stole the struct definition from the BSD header and put it here
// how getsockopt() manages to access it i have no idea
struct ps_ucred 
{
  pid_t pid;
  uid_t uid;
  gid_t gid;
};

int ps_create(const char *soc_name);
int ps_accept(int fd, struct ps_ucred *cred);

#endif 
