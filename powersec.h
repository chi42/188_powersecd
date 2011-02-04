#ifndef __POWERSECD_H__
#define __POWERSECD_H__

#define DAEMON       "powersecd"
#define PID_FILE     "/var/run/powersecd.pid"
#define MAX_CONNECT  10
#define SOCKET_NAME  "/var/run/powersecd.socket"
#define DATA_SOCKET  "/var/run/powersecd_data.socket"

// SLEEP_TIME specifies the amount of time between 
// daemon actions, in seconds 
#define SLEEP_SEC   8
#define SLEEP_USEC  0

// not sure what this signal is going to be yet...
#define ALERT_SIG   SIGUSR1


#endif


