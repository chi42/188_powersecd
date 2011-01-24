#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>

#define DAEMON     "powersecd"
#define PID_FILE   "/var/run/npowersecd.pid"


static
int daemonize(pid_t *pid, pid_t *sid)
{
  char pid_string[10];
  int fd;

  // fail if fork fails, else kill parent and child continues
  *pid = fork();
  if (*pid < 0)
    return -1;
  else if (*pid > 0) 
    exit(EXIT_SUCCESS);

  // child will need a new sid
  *sid = setsid();
  if (*sid < 0)
    return -1;
  
  // clear file mask
  umask(0);

  if (chdir("/") < 0)
    return -1;

  openlog(DAEMON, LOG_NDELAY, LOG_DAEMON);

  unlink(PID_FILE);
  fd = open(PID_FILE, O_WRONLY|O_CREAT|O_EXCL, 644);
  if (fd >= 0) {
    bzero(pid_string, 10);
    snprintf(pid_string, 9, "%d", getpid());
    write(fd, pid_string, 9);
    // note, leave PID_FILE open in case it gets unlinked by another
    // program
  }
  else
    return -1;

  // daemons do not run in a tty, so these STD files are not needed
  // leaving them around is a potential security vulnerability?
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  return 0;
}


int main(void)
{

  pid_t pid, sid;
  printf("hello world!\n");
  if (daemonize(&pid, &sid) < 0) {
    fprintf(stderr, "Error in daemonizing process, goodbye.");
    exit(EXIT_FAILURE);
  }

  while(1) {
    sleep(5);
  }
  exit(EXIT_SUCCESS); 
}
