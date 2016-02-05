#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "scitoll.h"

long long scitoll(char* scinum, int* retcode) {
  char *eptr, *Eptr;
  int offset=0;
  long long output;


  // Check for 'e' character
  eptr = strrchr(scinum, 'e');
  Eptr = strrchr(scinum, 'E'); 

  if ((eptr != NULL) && (Eptr != NULL)) {
    *retcode = SCITOLL_MULTIPLE_E;
  }
  else if (eptr != NULL) {
    offset = eptr - scinum;
  }
  else if (Eptr != NULL) {
    offset = Eptr - scinum;
  }
  else {
    output = atoll(scinum);
    return(output);
  }

  // Check for decimal
  char *dotptr = strchr(scinum, '.');
  if (dotptr != NULL) {
    *retcode = SCITOLL_DECIMAL;
  }
  
  if (offset == 0) 
    *retcode = SCITOLL_NO_MANTISSA;

  if (offset == strlen(scinum))
    *retcode = SCITOLL_NO_EXPONENT;

  char mantissa[strlen(scinum)+1];
  char exponent[strlen(scinum)+1];
  int i;

  // Collect mantissa string
  for (i=0; i<offset; i++) {
    mantissa[i] = scinum[i];
  }
  mantissa[offset] = '\0';

  // Collect exponent string
  for (i=offset+1; i<strlen(scinum)+1; i++) {
    exponent[i-offset-1] = scinum[i];
  }
  exponent[strlen(scinum)-offset] = '\0';

  // Calculate output
  output = atoll(mantissa)*pow(10,atoll(exponent));

  return(output);
}
