#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
//#include <syslog.h>
//#include <string.h>

#define DAEMON   "powersec"
#define PID_FILE "/var/run/powersec.pid"

int daemon_init(pid_t *pid, pid_t *sid)
{
  *pid = fork();
  // fail on bad fork, else kill parent and let child continue
  if (*pid < 0) {
    exit(EXIT_FAILURE);
  }
  else if (*pid > 0) {
    open(PID_FILE, O_WRONLY | O_CREAT);
    exit(EXIT_SUCCESS);
  }

  // clear file mask
  umask(0); 

  // do logging init here if necessary

  // child will need a new sid
  *sid = setsid();
  if (*sid < 0) {
    exit(EXIT_FAILURE);
  }

  if (chdir("/") < 0) {
    exit(EXIT_FAILURE);
  }
  
  // unnecessary to have std in/out/err, so close them, else its a 
  // potential vulnerability
  close(STDIN_FILENO);
  close(STDERR_FILENO);
  close(STDOUT_FILENO);

}

int main(void)
{

  pid_t pid, sid;
 

  while (1) {

  }

  exit(EXIT_SUCCESS); 
}
