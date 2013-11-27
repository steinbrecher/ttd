#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "scitollu.h"

unsigned long long scitollu(char* scinum) {
  char *eptr, *Eptr;
  int offset;
  long long output;


  // Check for 'e' character
  eptr = strrchr(scinum, 'e');
  Eptr = strrchr(scinum, 'E'); 

  if ((eptr != NULL) && (Eptr != NULL)) {
    return(SCITOLLU_MULTIPLE_E);
  }
  else if (eptr != NULL) {
    offset = eptr - scinum;
  }
  else if (Eptr != NULL) {
    offset = Eptr - scinum;
  }
  else {
    output = atoll(scinum);
    if (output < 0) {
      return(SCITOLLU_NEGATIVE);
    }
    return(output);
  }

  // Check for decimal
  char *dotptr = strchr(scinum, '.');
  if (dotptr != NULL) {
    return(SCITOLLU_DECIMAL);
  }
  
  if (offset == 0) 
    return(SCITOLLU_NO_MANTISSA);

  if (offset == strlen(scinum))
    return(SCITOLLU_NO_EXPONENT);

  char mantissa[strlen(scinum)+1];
  char exponent[strlen(scinum)+1];
  int i;
  for (i=0; i<offset; i++) {
    mantissa[i] = scinum[i];
  }
  mantissa[offset] = '\0';
  for (i=offset+1; i<strlen(scinum)+1; i++) {
    exponent[i-offset-1] = scinum[i];
  }
  exponent[strlen(scinum)-offset] = '\0';
  output = atoll(mantissa)*pow(10,atoll(exponent));
  if(output < 0) {
    return(SCITOLLU_NEGATIVE);
  }

  return(output);
}
