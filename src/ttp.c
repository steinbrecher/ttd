#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <math.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "ttp.h"

void ttp_print_version(char *program_name) {
  printf("%s Version %s\n", program_name, _ttp_version_string);
}
