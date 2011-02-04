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

#include "ps_main.h"
#include "ps_list.h"
#include "ps_sockets.h"

// in hopes of reducing memory usage
// is this actually necessary or will the compiler do magic to make it
// happen for me?
static const char *pidfile = PID_FILE;
static const char *socketname = SOCKET_NAME;
static const char *dat_socketname = DATA_SOCKET;

// global list of client processes
static ps_list g_clients;

static int daemonize(const char *pfile);
static void sig_to_exit(int sig);
static void sig_alarm(int sig);
static void cleanup();
static void *ps_listen_thread(void *ps_args);

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
  int a;

  // fetch new data here


  while(cn = ps_list_next(&g_clients)) {
    if (cn->signal) 
      if( write(cn->c_fd, &a, 1) < 0) {
        close(cn->c_fd);
        ps_list_del(&g_clients, cn);
      } 
      else 
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
int daemonize(const char *pfile)
{
  char *pid_string;
  int i, t, fd;
  struct sigaction s_action;
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

  memset(&s_action, 0, sizeof(struct sigaction));
  // there are many signals that we need to handle carefully 
  // NOTE: we MUST use sigaction, signal() is not thread safe
  //signal(SIGHUP, reload_conf);
  s_action.sa_handler = SIG_IGN;
  sigaction(SIGCHLD, &s_action, NULL);
  sigaction(SIGPIPE, &s_action, NULL);

  s_action.sa_handler = sig_alarm;
  sigaction(SIGALRM, &s_action, NULL);

  s_action.sa_handler = sig_to_exit;
  sigaction(SIGTERM, &s_action, NULL);
  sigaction(SIGQUIT, &s_action, NULL);
  sigaction(SIGINT,  &s_action, NULL);

  // open for logging
  openlog(DAEMON, LOG_NDELAY, LOG_DAEMON);

  if(pfile) {
    // get the pid file and lock it
    // lock file is also locked by the starting script, but we also
    //    leave this here for the sake of things
    fd = open(pfile, O_WRONLY|O_CREAT, 640);
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
  // else, no pidfile to lock...
  
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  
  return 0;
}

int main(void)
{
  struct ps_ucred cred;
  struct itimerval timer_v;
  client_node *nd;
  sigset_t sigs;
  int r, c_fd, s_fd = -1;
  uint8_t cli_type;

  fd_set rdfs;
  struct timeval tv;  

  // open up socket, start listening
  s_fd = ps_create(socketname);
  if (s_fd < 0) {
    fprintf(stderr, "Error in binding sockets.\n");
    exit(EXIT_FAILURE);
  }

  if (daemonize(pidfile) < 0) {
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

  // the 'event' loop
  while(1) {

    c_fd = ps_accept(s_fd, &cred);  
  
    if (c_fd >= 0) {
      nd = malloc(sizeof(client_node));
      nd->c_fd = c_fd;
      nd->pid  = cred.pid; 
      
      // this read will block  
      if (read(c_fd, &cli_type, 1) > 0) {

        // now we enter the critical section, block delievery of SIGALARM 
        sigprocmask(SIG_BLOCK, &sigs, NULL);
        
        if(cli_type == PS_DATAONLY) {
          data_trans(c_fd);
          close(c_fd);
        }
        else if (cli_type == PS_REGISTER) {
          ps_list_add(&g_clients, nd);
        }
        else
          close(c_fd);
          
        sigprocmask(SIG_UNBLOCK, &sigs, NULL);
        // end of critical section, unblock SIGALARM
      } 
      else 
        // else, the read failed or was interrupted, either way, we kill 
        // the connection
        close(c_fd);
    }
  }
  return 0;
}
