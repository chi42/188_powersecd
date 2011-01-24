#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>

#include "powersec.h"

static const char *pidfile = PID_FILE;

static int daemonize(pid_t *pid, pid_t *sid);
static void sigterm_exit(int sig);
static void cleanup();

static
void sigterm_exit(int sig)
{
  cleanup();
  syslog(LOG_INFO, "Recieved SIGTERM, exiting cleanly.\n");

  exit(EXIT_SUCCESS);
}


static
void cleanup()
{
  unlink(pidfile);
  closelog();
}

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

  // go to root dir
  if (chdir("/") < 0)
    return -1;

  // there are many signals that we should ignore
  signal(SIGCHLD, SIG_IGN);
  signal(SIGTERM, sigterm_exit);

  // get the pid file and lock it
  // lock file is also locked by the starting script, but we also
  //    leave this here for the sake of things
  //unlink(pidfile);
  fd = open(pidfile, O_WRONLY|O_CREAT, 640);
  if (fd >= 0) {
    if (lockf(fd, F_TLOCK, 0) >= 0) {
      bzero(pid_string, 10);
      snprintf(pid_string, 9, "%d\n", getpid());
      write(fd, pid_string, 9);
      // note, leave PID_FILE open in case it gets unlinked by another
      // program

      // daemons do not run in a tty, so these STD files are not needed
      // leaving them around is a potential security vulnerability?
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

      // finally open for logging
      openlog(DAEMON, LOG_NDELAY, LOG_DAEMON);

      return 0;
    }
  }
  // else failed to lock pidfile
  fprintf(stderr, 
	  "Could not create/lock " PID_FILE ", is the daemon already running?\n");
  return -1;
}


int main(void)
{
  pid_t pid, sid;

  if (daemonize(&pid, &sid) < 0) {
    fprintf(stderr, "Error in daemonizing process, goodbye.\n");
    exit(EXIT_FAILURE);
  }

  syslog(LOG_INFO, "powersecd started.\n");

  while(1) {

    sleep(5);

  }

  return 0;
}
