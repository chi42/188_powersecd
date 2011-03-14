#ifndef __PS_DATA_H__
#define __PS_DATA_H__

#include <stdint.h>

#define PS_PROC_BAT_F "/proc/battery/"
#define PS_PROC_NET_F "/proc/net/dev"


typedef struct ps_dat_t {
  unsigned int power;
  unsigned int security;
  uint8_t plug;
} ps_dat;

int ps_data_fetch(ps_dat *dat);

#endif

