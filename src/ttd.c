#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "ttd.h"

ttd_t ttd_rounded_divide(ttd_t t1, ttd_t t2) {
  long double t1d = t1;
  long double t2d = t2;
  
  return (ttd_t)llrintl(t1d/t2d);
}

int64_t int64_rounded_divide(int64_t t1, int64_t t2) {
  long double t1d = t1;
  long double t2d = t2;
  
  return llrintl(t1d/t2d);
}

void ttd_print_version() {
  printf("TTD %s\n\n", _ttd_version_string);
  printf("Time tagged data processing software\n");
  printf("Copyright (c) 2013-2014 Greg Steinbrecher\n");
  printf("Licensed under the MIT License\n\n");
}
