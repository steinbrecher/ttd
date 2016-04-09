#ifndef _TTD_HEADER_SEEN
#define _TTD_HEADER_SEEN

#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <stddef.h>

#define TTD_MAX_TIME 0xFFFFFFFFFFFFFFFF
typedef uint64_t ttd_t;

static const char _ttd_version_string[] = "0.4.0";
void ttd_print_version(char* program_name);

ttd_t ttd_rounded_divide(ttd_t t1, ttd_t t2);
int64_t int64_rounded_divide(int64_t t1, int64_t t2);

#endif // _TTD_HEADER_SEEN
