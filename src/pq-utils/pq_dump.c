//
// Created by Greg Steinbrecher on 1/31/16.
//
#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <stdlib.h>
#include "pq_dump.h"

int main(int argc, char* argv[]) {
  FILE *pq_file;

  // Check to make sure we got an input file (and nothing else) from command line
  int retcode=0, err=0;

  if (argc > 2) {
    fprintf(stderr, "Error: Too many inputs.\n");
    err=-1;
  }
  else if (argc < 2) {
    fprintf(stderr, "Error: Missing input file.\n");
    err=-2;
  }
  if (err < 0) {
    fprintf(stderr, "Usage: %s [input_file]\n", argv[0]);
    exit(err);
  }

  pq_fb_t fb;
  retcode = pq_fb_init(&fb, argv[1]);
  if (retcode < 0) {
    fprintf(stderr, "ERROR: pq_fb_init returned %d\n", retcode);
    exit(retcode);
  }

  ttd_t time;
  int16_t channel;
  uint64_t i;
  while (!(fb.empty)) {
    pq_fb_get_next(&fb, &time, &channel);
    printf("%" PRId16 ": %" PRIu64 "\n", channel, time);
  }

  pq_fb_cleanup(&fb);
  exit(0);


}