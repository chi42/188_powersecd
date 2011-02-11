#include <stdio.h>
#include <stdint.h>
#include <syslog.h>
#include "ps_data.h"

static int ps_data_battery_plug(uint8_t *battery, uint8_t *plug);
static int ps_data_security(uint8_t *security);

static 
int ps_data_battery_plug(uint8_t *battery, uint8_t *plug) 
{
  *battery = 10;
  *plug = 1;
 
  return 0;
}

static
int ps_data_security(uint8_t *security)
{
  *security = 10;

  return 0;
}

int ps_data_fetch(ps_dat *dat) 
{
  int  r;

  #ifdef PS_TEST_INFILE
  static FILE *test_f = NULL;
  static uint8_t loop = 0, fflag = 0;

  unsigned int a, b, c; 
  #endif

  ps_data_battery_plug(&(dat->power), &(dat->plug));
  ps_data_security(&(dat->security));
  
  #ifdef PS_TEST_INFILE
  if (!fflag) {
    fflag = 1;
    test_f = fopen(PS_TEST_INFILE, "r");
    if (!test_f)
      syslog(LOG_INFO,
	     "Testing mode, could not open test input file stream.\n");
  }   

  if (test_f) {
    while(loop < 2) {
      ++loop;
      r = fscanf(test_f, "%u %u %u", &a, &b, &c);
      if (r == 3) {
	dat->power = (uint8_t)a;
	dat->security = (uint8_t)b;
	dat->plug = (uint8_t)c;

	loop = 0;
	break;
      }
      else if (r == EOF) {
	fseek(test_f, 0, SEEK_SET);
      }
      else {
	syslog(LOG_INFO, "Bad test input file.\n");
	break;
      }
    }
  }
  #endif
  
  return 0;
}
