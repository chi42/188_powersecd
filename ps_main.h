#ifndef __PS_MAIN_H__
#define __PS_MAIN_H__

#include <stdio.h>

#define DAEMON       "powersecd"
#define MAX_CONNECT  10
#define SLEEP_SEC   2            // seconds
#define SLEEP_USEC  0            // micro-seconds, (should always be 0)
#define ALERT_SIG   SIGUSR1

#define MESG_SIZ    8            // including NULL byte (2 + 1 + 2 +1 + 1 + 1)
#define PSPRINT(___BUFFER, ___DAT) \
    snprintf(___BUFFER, MESG_SIZ, "%2u %2u %u", \
	     ___DAT.power, ___DAT.security, ___DAT.plug);

#endif

