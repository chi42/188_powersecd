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
#include <sys/time.h>

#include "powersec.h"
#include "ps_list.h"
#include "ps_sockets.h"

static const char *pidfile = PID_FILE;
static const char *socketname = SOCKET_NAME;

static int daemonize();
static void sig_to_exit(int sig);
static void sig_alarm(int sig);
static void cleanup();

static i_powersec dat;

static
void sig_to_exit(int sig)
{
  cleanup();
  syslog(LOG_INFO, "Recieved signal %d, exiting cleanly.\n", sig);

  exit(EXIT_SUCCESS);
}

static
void sig_alarm(int sig) 
{
  
  // fetch new data 
  // handle signalling to clients, close connections to clients
  // that are dead
}

static
void cleanup()
{
  unlink(socketname);
  unlink(pidfile);
  closelog();
}

static
int daemonize()
{
  char *pid_string;
  int i, t, fd;
  pid_t pid, sid;

  // fail if fork fails, else kill parent and child continues
  pid = fork();
  if (pid < 0)
    return -1;
  else if (pid > 0) 
    exit(EXIT_SUCCESS);

  // child will need a new sid
  sid = setsid();
  if (sid < 0)
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
  signal(SIGALRM, sig_alarm);
  signal(SIGTERM, sig_to_exit);
  signal(SIGQUIT, sig_to_exit);
  signal(SIGINT, sig_to_exit);

  // open for logging
  openlog(DAEMON, LOG_NDELAY, LOG_DAEMON);

  // get the pid file and lock it
  // lock file is also locked by the starting script, but we also
  //    leave this here for the sake of things
  //unlink(pidfile);
  fd = open(pidfile, O_WRONLY|O_CREAT, 640);
  if (fd >= 0) {
    if (lockf(fd, F_TLOCK, 0) >= 0) {

      t = pid = getpid();
      for(i = 1; t > 0; ++i) {
        t /= 10;
      }
      pid_string = malloc(i + 2); 
      if (!pid_string)
        return -1;

      snprintf(pid_string, i + 1, "%d\n", pid);
      write(fd, pid_string, i);

      free(pid_string);
      // note, leave PID_FILE open in case it gets unlinked by another
      // program
     
      // daemons do not run in a tty, so these STD files are not needed
      // leaving them around is a potential security vulnerability?
      // also, even in the case where we don't want to daemonize, probably
      // do not want to print out to stdout/err anyways...
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);

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
  struct ps_ucred cred;
  struct itimerval timer_v;
  sigset_t sigs;
  int r, s_fd = -1;

  // open up socket, start listening
  s_fd = ps_create(socketname);
  if (s_fd < 0) {
    fprintf(stderr, "Error in binding sockets.\n");
    exit(EXIT_FAILURE);
  }
  if (daemonize() < 0) {
    fprintf(stderr, "Error in daemonizing process, goodbye.\n");
    exit(EXIT_FAILURE);
  }

  syslog(LOG_INFO, "powersecd started.\n");
  // sigs structure is set up, no need to change it again  
  sigemptyset(&sigs);
  sigaddset(&sigs, SIGALRM);

  // set up timer, it will be raised regularly
  timer_v.it_interval.tv_sec  = SLEEP_SEC;
  timer_v.it_interval.tv_usec = SLEEP_USEC;
  timer_v.it_value.tv_sec  = SLEEP_SEC;
  timer_v.it_value.tv_usec = SLEEP_USEC;
  setitimer(ITIMER_REAL, &timer_v, NULL);

  while(1) {
    ps_accept(s_fd, &cred);  

    sigprocmask(SIG_BLOCK, &sigs, NULL);

    syslog(LOG_INFO, "POWERSECD PID: %d\n", cred.pid);
    
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
  }
  return 0;
}

