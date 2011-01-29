#ifndef __POWERSECD_H__
#define __POWERSECD_H__

#define DAEMON       "powersecd"
#define PID_FILE     "/var/run/powersecd.pid"
#define MAX_CONNECT  10
#define SOCKET_NAME  "/var/run/powersecd.socket"
// SLEEP_TIME specifies the amount of time between 
// daemon actions, in seconds 
#define SLEEP_TIME   5

typedef struct i_powersect {
  int a;
  int b;
//bunch of stuff
} i_powersec;

#endif


