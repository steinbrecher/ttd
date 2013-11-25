#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ttp.h"

void ttp_print_version() {
  printf("TTDP %s\n\n", _ttp_version_string);
  printf("Time tagged data processing software\n");
  printf("Copyright (c) 2013 Greg Steinbrecher\n");
  printf("Licensed under the MIT License\n\n");

}
