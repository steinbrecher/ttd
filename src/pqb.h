#ifndef PQB_HEADER_SEEN
#define PQB_HEADER_SEEN

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <stddef.h>

typedef struct {
  uint64_t time;
} pqb_t; // As in, 'picoquant binary'

typedef struct { pqb_t record; } pqb_buffer_t;

typedef struct { pqb_buffer_t *buffer; } pqb_buffer_group_t;

#define PQB_T_FMT PRIu64

#endif //PQB_HEADER_SEEN
