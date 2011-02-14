
#include <stdio.h>
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
#include <stdlib.h>
#include <time.h>

#include "ps_main.h"
#include "ps_list.h"
#include "ps_sockets.h"
#include "powersecd.h"
#include "ps_data.h"

// in hopes of reducing memory usage
// is this actually necessary or will the compiler do magic to make it
// happen for me?
static const char *pidfile = PID_FILE;
static const char *socketname = SOCKET_NAME;

// global list of client processes
static ps_list g_clients;
static time_t g_last_get;
static ps_dat g_dat;

static int daemonize(const char *pfile);
static void sig_to_exit(int sig);
static void sig_alarm(int sig);
static void cleanup();
static int write_all(int fd, char *buf, int b_size);
static void timer_set(long sec, long usec);

static
int write_all(int fd, char *buf, int b_size)
{
  int r = 0;
  do {
    r = write(fd, buf + r, b_size - r);
    if (r < 0)
      return r;
  } while(b_size - r);

  return 0;
}

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
  int r, send = 0;
  uint8_t buffer[9];
  
  static unsigned int power, security, plug = 255;
  

#ifdef PS_TEST_INFILE
  static int no_get = 0;
  int i;
#endif

  // point of all the preprocessors, is to remove the case
  // of skipping a line in the input testfile when there is 
  // only client and it decided to drop and reconnect
#ifdef PS_TEST_INFILE
  if (!no_get) {
#endif

  if (g_last_get + SLEEP_SEC <= time(NULL))
    ps_data_fetch(&g_dat);

#ifdef PS_TEST_INFILE
  }
  no_get = 0;
  i = 0;
#endif
 
  if (plug != 255) {
    send = power ^ g_dat.power;
    send |= security ^ g_dat.security;
    send |= plug ^ g_dat.plug;
  }
  else
    send = 255;

  r = PSPRINT(buffer, g_dat);
  while(cn = ps_list_next(&g_clients)) {

    if (send || cn->first) {
      if (write_all(cn->c_fd, buffer, MESG_SIZ) < 0) {
	close(cn->c_fd);
	ps_list_del(&g_clients, cn);
      } 
      else {
	kill(cn->pid, SIGUSR1);
	cn->first = 0;
#ifdef PS_TEST_INFILE
	++i;
#endif 
      }
    }
  }
  // if there are no clients, we clear the alarm, wake only on new
  // connection, which sends daemon into permanent blocking state
  if (!g_clients.size)
      timer_set(0, 0);

#ifdef PS_TEST_INFILE
  if (send && i < 1) {
    no_get = 1;
  }
#endif

  if (time(&g_last_get) < 0)
    g_last_get = 0;
	   
  power = g_dat.power;
  security = g_dat.security;
  plug = g_dat.plug;
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

  // clear file mask  
  umask(0);

  // open for logging
  openlog(DAEMON, LOG_NDELAY, LOG_DAEMON);

  // get the pid file and lock it
  // lock file is also locked by the starting script, but we also
  //    leave this here for the sake of things
  fd = open(pfile, O_WRONLY|O_CREAT, 0444);
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

static
void timer_set(long sec, long usec) 
{
  struct itimerval tv;

  // set up timer, it will be raised regularly
  tv.it_interval.tv_sec  = sec;
  tv.it_interval.tv_usec = usec;
  tv.it_value.tv_sec  = sec;
  tv.it_value.tv_usec = usec;

  setitimer(ITIMER_REAL, &tv, NULL);
  
  return;
}


int main(void)
{
  struct ps_ucred cred;
  client_node *nd;
  sigset_t sigs;
  int r, c_fd, s_fd = -1;
  uint8_t cli_type, buffer[9];

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
  
  g_last_get = 0;
  g_dat.plug = 255;

  // the 'event' loop
  while(1) {
    
    c_fd = ps_accept(s_fd, &cred);  
  
    if (c_fd >= 0) {
      
      // this read will block  
      if (read(c_fd, &cli_type, 1) > 0) {

        // now we enter the critical section, block delievery of SIGALARM 
        sigprocmask(SIG_BLOCK, &sigs, NULL);
        
        if(cli_type == PS_DATAONLY) {
	  if (g_last_get + SLEEP_SEC <= time(NULL))
	    ps_data_fetch(&g_dat);

          PSPRINT(buffer, g_dat);
	  write_all(c_fd, buffer, MESG_SIZ);
          close(c_fd);
        }
        else if (cli_type == PS_REGISTER) {
	  if (!(g_clients.size))
	    timer_set(SLEEP_SEC, SLEEP_USEC);

          nd = malloc(sizeof(client_node));
          nd->c_fd  = c_fd;
          nd->pid   = cred.pid; 
	  nd->first = 1;
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

