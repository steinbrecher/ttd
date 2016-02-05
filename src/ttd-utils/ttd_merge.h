#ifndef TTD_MERGE_HEADER_SEEN
#define TTD_MERGE_HEADER_SEEN

#ifndef PHOTONBLOCK
#define PHOTONBLOCK 16384
#endif

void ttd_merge_output(uint64_t *records, int64_t num_records, FILE *outfile) {
  fwrite(records, sizeof(uint64_t), num_records, outfile); 
}

int64_t ttd_merge(char *infile1, char *infile2, FILE *outfile) {
  uint64_t output_buffer[PHOTONBLOCK];
  int64_t output_buffer_count = 0;
  uint64_t t1, t2;
  ttd_fb_t *in1 = (ttd_fb_t *)malloc(sizeof(ttd_fb_t));
  ttd_fb_t *in2 = (ttd_fb_t *)malloc(sizeof(ttd_fb_t));

  ttd_fb_init(in1, PHOTONBLOCK, infile1, 0);
  ttd_fb_init(in2, PHOTONBLOCK, infile2, 0);

  t1 = ttd_fb_pop(in1);
  t2 = ttd_fb_pop(in2);

  while((in1->empty == 0) && (in2->empty == 0)) {
    if (t1 <= t2) {
      output_buffer[output_buffer_count] = t1;
      t1 = ttd_fb_pop(in1);
      output_buffer_count ++;
    }
    else {
      output_buffer[output_buffer_count] = t2;
      t2 = ttd_fb_pop(in2);
      output_buffer_count ++;
    }
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in1->empty == 0) {
    output_buffer[output_buffer_count] = t1;
    t1 = ttd_fb_pop(in1);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  while (in2->empty == 0) {
    output_buffer[output_buffer_count] = t2;
    t2 = ttd_fb_pop(in2);
    output_buffer_count ++;
    if (output_buffer_count == PHOTONBLOCK) {
      ttd_merge_output(output_buffer, PHOTONBLOCK, outfile);
      output_buffer_count = 0;
    }
  }
  if (output_buffer_count > 0) {
    ttd_merge_output(output_buffer, output_buffer_count, outfile);
  }
  
  ttd_fb_cleanup(in1);
  free(in1);
  
  ttd_fb_cleanup(in2);
  free(in2);

  return(0);
}

#endif // TTD_MERGE_HEADER_SEEN
