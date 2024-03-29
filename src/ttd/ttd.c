#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <math.h>

#include "ttd.h"

ttd_t ttd_rounded_divide(ttd_t t1, ttd_t t2) {
  long double t1d = t1;
  long double t2d = t2;

  return (ttd_t) llrintl(t1d / t2d);
}

int64_t int64_rounded_divide(int64_t t1, int64_t t2) {
  /* long double t1d = t1; */
  /* long double t2d = t2; */

  /* return llrintl(t1d/t2d); */

  int64_t abs_res = ((2 * llabs(t1) + t2) / (2 * t2));
  if (t1 >= 0) {
    return abs_res;
  } else {
    return (-abs_res);
  }
}

void ttd_print_version(char *program_name) {
  printf("%s, part of ttd version %s\n", program_name, _ttd_version_string);
  printf("Copyright (C) Greg Steinbrecher 2016\n");
  printf("ttd is released under the terms of the MIT license\n");
}
