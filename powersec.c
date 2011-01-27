#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <sys/signal.h>

#include "powersec.h"
#include "ps_sockets.h"

static const char *pidfile = PID_FILE;
static const char *socketname = SOCKET_NAME;

static int daemonize(pid_t *pid, pid_t *sid);
static void sig_to_exit(int sig);
static void cleanup();

static
void sig_to_exit(int sig)
{
  cleanup();
  syslog(LOG_INFO, "Recieved signal %d, exiting cleanly.\n", sig);

  exit(EXIT_SUCCESS);
}


static
void cleanup()
{
  unlink(socketname);
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

  // there are many signals that we need to handle carefully 
  //signal(SIGHUP, reload_conf);
  signal(SIGCHLD, SIG_IGN);
  signal(SIGPIPE, SIG_IGN);
  signal(SIGTERM, sig_to_exit);
  signal(SIGQUIT, sig_to_exit);
  signal(SIGINT, sig_to_exit);


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
  fd_set r_fds;
  struct timeval tv;
  struct ps_ucred cred;

  int r, s_fd = -1;

  if (daemonize(&pid, &sid) < 0) {
    fprintf(stderr, "Error in daemonizing process, goodbye.\n");
    exit(EXIT_FAILURE);
  }

  // open up socket, start listening
  s_fd = ps_create(socketname);
  if (s_fd < 0) {
    fprintf(stderr, "Error in binding sockets.\n");
    exit(EXIT_FAILURE);
  }

  // daemons do not run in a tty, so these STD files are not needed
  // leaving them around is a potential security vulnerability?
  // also, even in the case where we don't want to daemonize, probably
  // do not want to print out to stdout/err anyways...
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  syslog(LOG_INFO, "powersecd started.\n");

  FD_ZERO(&r_fds);
  FD_SET(s_fd, &r_fds);

  while(1) {
    // note that select() CAN modify timeval struct on return, so to ensure
    //   correct operation on all systems, we have to reset it each time
    tv.tv_usec = 0;
    tv.tv_sec  = SLEEP_TIME; 
    
    r = select(s_fd + 1, &r_fds, NULL, NULL, &tv);
    if (r > 0) {
      ps_accept(s_fd, &cred);  
      syslog(LOG_INFO, "POWERSECD PID: %d\n", cred.uid);
      
    }
    else if (r < 0)
      if (errno != EINTR)
        syslog(LOG_ERR, "Error in polling for new connections.\n");
  }

  return 0;
}
