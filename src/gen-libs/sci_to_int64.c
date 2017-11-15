#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "sci_to_int64.h"

int64_t sci_to_int64(const char *scinum, int *retcode) {
  retcode = 0;
  char end = '\0';
  char *pEnd = &end;
  char *eptr, *Eptr;
  size_t offset = 0;
  int64_t output;

  // Check for 'e' character
  eptr = strrchr(scinum, 'e');
  Eptr = strrchr(scinum, 'E');

  if ((eptr != NULL) && (Eptr != NULL)) {
    *retcode = SCITOLL_MULTIPLE_E;
    return (0);
  } else if (eptr != NULL) {
    offset = eptr - scinum;
  } else if (Eptr != NULL) {
    offset = Eptr - scinum;
  } else {
    output = strtoll(scinum, &pEnd, 10);
    if (*pEnd) {
      fprintf(stderr, "end: %d\n", (int) pEnd);
      fprintf(stderr, "Could not convert %s to integer\n", scinum);
      *retcode = SCITOLL_STRTOLL_FAILED;
      return (0);
    }
    return (output);
  }

  // Check for decimal
  char *dotptr = strchr(scinum, '.');
  if (dotptr != NULL) {
    *retcode = SCITOLL_DECIMAL;
    return (0);
  }

  if (offset == 0) {
    *retcode = SCITOLL_NO_MANTISSA;
    return (0);
  }

  if (offset == strlen(scinum)) {
    *retcode = SCITOLL_NO_EXPONENT;
    return (0);
  }

  char mantissa_str[strlen(scinum) + 1];
  char exponent_str[strlen(scinum) + 1];
  size_t i;

  // Collect mantissa string
  for (i = 0; i < offset; i++) {
    mantissa_str[i] = scinum[i];
  }
  mantissa_str[offset] = '\0';

  // Collect exponent string
  for (i = offset + 1; i < strlen(scinum) + 1; i++) {
    exponent_str[i - offset - 1] = scinum[i];
  }
  exponent_str[strlen(scinum) - offset] = '\0';

  // Convert exponent to integer and raise 10 to that power
  // (explicit loop since pow() is type double)

  int64_t exponent = strtoll(exponent_str, &pEnd, 10);
  if (*pEnd) {
    fprintf(stderr, "Could not convert %s to integer\n", exponent_str);
    *retcode = SCITOLL_STRTOLL_FAILED;
    return (0);
  }
  int64_t mantissa = strtoll(mantissa_str, &pEnd, 10);
  if (*pEnd) {
    fprintf(stderr, "Could not convert %s to integer\n", mantissa_str);
    *retcode = SCITOLL_STRTOLL_FAILED;
    return (0);
  }

  output = 1;
  int64_t old_output;
  for (i = 0; i < exponent; i++) {
    output *= 10;
    if (output < old_output) {
      *retcode = SCITOLL_OVERFLOW;
      return (0);
    }
    old_output = output;
  }

  output *= mantissa;
  if ((output < old_output) && (mantissa > 0)) {
    *retcode = SCITOLL_OVERFLOW;
    return (0);
  }

  return (output);
}

void sci_to_int64_printerr(const char *scinum, int retcode) {
  switch (retcode) {
    case SCITOLL_MULTIPLE_E:
      fprintf(stderr, "Error: %s contains multiple 'e' characters\n", scinum);
      break;
    case SCITOLL_DECIMAL:
      fprintf(stderr, "Error: %s contains a decimal\n", scinum);
      break;
    case SCITOLL_NO_MANTISSA:
      fprintf(stderr, "Error: %s needs a number before the 'e' character\n", scinum);
      break;
    case SCITOLL_NO_EXPONENT:
      fprintf(stderr, "Error: %s needs a number after the 'e' character\n", scinum);
      break;
    case SCITOLL_STRTOLL_FAILED:
      fprintf(stderr, "Error: strtoll conversion failed at some point in %s\n", scinum);
      break;
    case SCITOLL_OVERFLOW:
      fprintf(stderr, "Error: Conversion of %s overflowed.\n", scinum);
      break;
  }
}
