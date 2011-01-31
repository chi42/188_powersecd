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
#include <stdint.h>

#include "powersec.h"
#include "ps_list.h"
#include "ps_sockets.h"

// in hopes of reducing memory usage
// is this actually necessary or will the compiler do magic to make it
// happen for me?
static const char *pidfile = PID_FILE;
static const char *socketname = SOCKET_NAME;

static int daemonize();
static void sig_to_exit(int sig);
static void sig_alarm(int sig);
static void cleanup();

static ps_list g_clients;

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
  client_node *cn;

  // fetch new data 

  syslog(LOG_INFO, "RECIEVED SIGLARAM\n");

  while(cn = ps_list_next(&g_clients)) {
    if (cn->signal) 
      kill(cn->pid, ALERT_SIG);
  }
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
  client_node *nd;
  sigset_t sigs;
  int r, c_fd, s_fd = -1;
  uint8_t temp;

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

  ps_list_init(&g_clients);

  // set up timer, it will be raised regularly
  timer_v.it_interval.tv_sec  = SLEEP_SEC;
  timer_v.it_interval.tv_usec = SLEEP_USEC;
  timer_v.it_value.tv_sec  = SLEEP_SEC;
  timer_v.it_value.tv_usec = SLEEP_USEC;
  setitimer(ITIMER_REAL, &timer_v, NULL);

  while(1) {
    c_fd = ps_accept(s_fd, &cred);  
  
    if (c_fd >= 0) {
      nd = malloc(sizeof(client_node));
      nd->c_fd = c_fd;
      nd->pid  = cred.pid; 
        
      // don't really care about what is read, just as long as we get something
      // NOTE: c_fd is returned by ps_accept as a non-blocking socket!
      //    therefore, this read() will return immediately!
      if (read(c_fd, &temp, 1) > 0)
        nd->signal = 1;
      else 
        nd->signal = 0;

      // now we enter the critical section, block delievery of SIGALARM 
      sigprocmask(SIG_BLOCK, &sigs, NULL);

      ps_list_add(&g_clients, nd);
      
      sigprocmask(SIG_UNBLOCK, &sigs, NULL);
      // end of critical section, unblock SIGALARM
    }
  }
  return 0;
}
