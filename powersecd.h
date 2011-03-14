#ifndef __POWERSECD_H__
#define __POWERSECD_H__

// connection type status...
#define PS_DATAONLY   1
#define PS_REGISTER   2

// define this to enable reading from text file
// for testing only!!
#define PS_TEST_INFILE "/ramdisk/ps_test_in"

#define PID_FILE     "/var/run/powersecd.pid"
#define SOCKET_NAME  "/var/run/powersecd.sock"

#endif

