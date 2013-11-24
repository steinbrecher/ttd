#ifndef TTD_HEADER_SEEN
#define TTD_HEADER_SEEN

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stddef.h>

typedef uint64_t ttd_t;

//typedef struct { ttd_t record; } ttd_buffer_t;

//typedef struct { ttd_buffer_t *buffer; } ttd_buffer_group_t;

ttd_t ttd_rounded_divide(ttd_t t1, ttd_t t2);
ttd_t ttd_rounded_divide_unsafe(ttd_t t1, ttd_t t2);

#endif // TTD_HEADER_SEEN
