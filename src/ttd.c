#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include "ttd.h"

ttd_t ttd_rounded_divide(ttd_t t1, ttd_t t2) {
  ttd_t trunc = t1 / t2;
  ttd_t rem = t1 % t2; 
  
  // If remainder is more than half of t2, division is rounded up
  if (2*rem > t2) {
    return(trunc + 1);
  }
  // Alternatively, if it's less, the truncated result is fine. 
  else if (2*rem < t2) {
    return(trunc);
  }
  // If we haven't returned, 2*rem == t2 and we need to round to even
  if ((trunc % 2) == 0) {
    return(trunc);
  }
  return(trunc + 1); 
}

// This method is faster, but is going to be inaccurate for large values 
// of t1 or t2 due to limited floating point precision. This function 
// deliberately has the same interface as ttd_rounded_divide so that a 
// conditional function pointer may be set depending on which is desired
// at a given part of program execution.
ttd_t ttd_rounded_divide_unsafe(ttd_t t1, ttd_t t2) {
  return (ttd_t) round( ((double)t1) / ((double)t2) );
}
