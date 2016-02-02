//
// Created by Greg Steinbrecher on 2/1/16.
//
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

#include "pq_ttd_cli.h"
#include "pq_ttd.h"

char *get_prefix(char* filename) {
  int i, dot_index = -1;
  char *prefix;
  for (i=0; i < strlen(filename); i++) {
    if (filename[i] == '.') {
      dot_index = i;
    }
  }
  if (dot_index == -1) {
    prefix = (char *)malloc((strlen(filename)+1)*sizeof(char));
    strcpy(prefix, filename);
    return(prefix);
  }
  prefix = (char *)malloc((dot_index+2)*sizeof(char));
  for (i=0; i<dot_index; i++) {
    prefix[i] = filename[i];
  }
  prefix[dot_index] = '\0';
  return prefix;
}

int main(int argc, char* argv[]) {
  FILE *ht_file;
  int retcode,exitcode=0;
  int prefix_allocated=0;
  char *output_prefix;

  retcode = pq_ttd_read_cli(argc, argv);
  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_pq_ttd_cli;
  }
  else if (retcode == PQ_TTD_CLI_EXIT_RETCODE) {
    goto cleanup_pq_ttd_cli;
  }

  if (pq_ttd_cli_args.infile == NULL) {
    printf("Error: Please supply input file with '-i [infile]'\n");
    goto cleanup_pq_ttd_cli;
  }

  if (pq_ttd_cli_args.output_prefix == NULL) {
    pq_ttd_cli_args.output_prefix = get_prefix(pq_ttd_cli_args.infile);
    pq_ttd_cli_args.output_prefix_allocated = 1;
  }

  printf("Output Prefix: %s\n", pq_ttd_cli_args.output_prefix);

  ht_file = fopen(pq_ttd_cli_args.infile, "rb");

  pq_fileinfo_t file_info;
  retcode = pq_parse_header(ht_file, &file_info);
  if (retcode < 0) {
    goto clean_file;
  }
  pq_printf_file_info(&file_info);

  // Benchmarking timers
  clock_t start, diff;

  printf("\n");
  start = clock();
  run_hh_convert(ht_file, &file_info);
  diff = clock() - start;

  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);

  clean_file:
  fclose(ht_file);

  cleanup_pq_ttd_cli:
  pq_ttd_cli_cleanup();
  exit(exitcode);
}