#ifndef __PS_DATA_H__
#define __PS_DATA_H__

#include <stdint.h>

typedef struct ps_dat_t {
  uint8_t power;
  uint8_t security;
  uint8_t plug;
} ps_dat;

#define PS_PROC_BAT_F "/proc/battery/"
#define PS_PROC_NET_F "/proc/net/dev"

#define PS_TEST_INFILE "/etc/ps_test_in"

int ps_data_fetch(ps_dat *dat);

#endif
