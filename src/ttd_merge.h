#ifndef TTD_MERGE_HEADER_SEEN
#define TTD_MERGE_HEADER_SEEN

#ifndef PHOTONBLOCK
#define PHOTONBLOCK 16384
#endif

#include "ttd_doqkd_buffers.h"

void ttd_merge_output(uint64_t *records, int64_t num_records, FILE *outfile) {
  fwrite(records, sizeof(uint64_t), num_records, outfile); 
}

int64_t ttd_merge(doqkd_buffer_t *in1, doqkd_buffer_t *in2, FILE *outfile) {
  uint64_t output_buffer[PHOTONBLOCK];
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;
  t1 = ttd_buffer_pop(in1);
  t2 = ttd_buffer_pop(in2);

  // Initialize buffers
  // TODO: Adapt ttd_doqkd_buffers to be more general (i.e. not dokd-specific)
  // so that its helper functions can be used here and in other functions
  in1->offset = 0;
  in1->buffer_size = PHOTONBLOCK;
  in1->buffer_fill = 0;
  in1->num_read = 0;
  in1->empty = 0;
  in1->buffered_records = (ttd_t *)malloc(in1->buffer_size * sizeof(ttd_t));
  in1->buffer_allocated = 1;

  in2->offset = 0;
  in2->buffer_size=PHOTONBLOCK;
  in2->buffered_records = (ttd_t *)malloc(in2->buffer_size * sizeof(ttd_t));
  in2->buffer_fill = 0;
  in2->num_read = 0;
  in2->empty = 0;
  in2->buffered_records = (ttd_t *)malloc(in1->buffer_size * sizeof(ttd_t));
  in2->buffer_allocated = 1;

  while((in1->empty == 0) && (in2->empty == 0)) {
    if (t1 <= t2) {
      output_buffer[output_buffer_count] = t1;
      t1 = ttd_buffer_pop(in1);
      output_buffer_count ++;
    }
    else {
      output_buffer[output_buffer_count] = t2;
      t2 = ttd_buffer_pop(in2);
      output_buffer_count ++;
    }
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in1->empty == 0) {
    output_buffer[output_buffer_count] = t1;
    t1 = ttd_buffer_pop(in1);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in2->empty == 0) {
    output_buffer[output_buffer_count] = t2;
    t2 = ttd_buffer_pop(in2);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  if (output_buffer_count > 0) {
    ttd_merge_output(output_buffer, output_buffer_count, outfile);
  }
  

  return(0);
}

#endif // TTD_MERGE_HEADER_SEEN
