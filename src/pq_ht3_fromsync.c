#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "pq_parse.h"
#include "pq_records.h"

int ht3_fromsync(pq_rec_t pq_rec) {
  return((int)pq_rec.hh_t3_bits.dtime);
}

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int num_bins;
  int center_bin;

  int64_t total;

  ttd_ccorr_stats_t stats;

  int rbs_allocated[2];
  ttd_rb_t *rbs[2];

  int hist_allocated;
  ttd_t *hist;
} ttd_hist_t;


int main(int argc, char* argv[]) {
  FILE *ht_file;
  int retcode,exitcode=0;

  ht_file = fopen(pq_ttd_cli_args.infile, "rb");
  pq_fileinfo_t file_info;
  retcode = pq_parse_header(ht_file, &file_info);
  if (retcode < 0) {
    goto clean_file;
  }
  pq_printf_file_info(&file_info);

  

 clean_file:
  fclose(ht_file);

  exit(exitcode);
}

