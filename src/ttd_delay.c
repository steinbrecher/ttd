#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <getopt.h>

#include "ttd.h"
#include "ttp.h"
#include "ttd_filebuffer.h"

#define TTD_DELAY_EXIT 1

struct {
  int verbose; // -v

  int infile_allocated;
  char* infile; // -i

  int outfile_allocated; 
  char* outfile; // -o

  int64_t delay;
} ttd_delay_cli_args;

static const struct option ttd_delay_longopts[] = {
  { "version", no_argument, NULL, 'V' },
  { "help", no_argument, NULL, 'h' },

  { "verbose", no_argument, NULL, 'v' },

  { "input", required_argument, NULL, 'i' },
  { "output", required_argument, NULL, 'o' },

  { "offset", required_argument, NULL, 'T' },
};

static const char *ttd_delay_optstring = "Vhvi:o:T:";

void ttd_delay_print_help(char* program_name) {
  int len = strlen(program_name)+1;
  char pn_spaces[len];
  int i;
  for (i=0; i<len-1; i++) {
      pn_spaces[i] = ' ';
    }
  pn_spaces[len-1] = '\0';
  printf("Usage: %s [-i infile] [-o outfile] [-T offset_time]\n", program_name);
  
  printf("\tNotes: \n");
  printf("\t\t-T (--offset): Amount to offset data in picoseconds. This must be positive.");

  printf("\tOther options:\n");
  printf("\t\t-v (--verbose):\t\tEnable verbose output to stdout\n");
  printf("\t\t-h (--help):\t\tPrint this help dialog\n");
  printf("\t\t-V (--version):\t\tPrint program version\n");
}

#define SCITOLL_MULTIPLE_E -1
#define SCITOLL_NO_E -2
#define SCITOLL_NO_MANTISSA -3
#define SCITOLL_NO_EXPONENT -4
// Converts number in scientific notation to int64_t
long long scitoll(char* scinum) {
  char *eptr, *Eptr;
  int offset;
  eptr = strrchr(scinum, 'e');
  Eptr = strrchr(scinum, 'E'); 
  if ((eptr != NULL) && (Eptr != NULL)) {
    return(SCITOLL_MULTIPLE_E);
  }
  else if (eptr != NULL) {
    offset = eptr - scinum;
  }
  else if (eptr != NULL) {
    offset = Eptr - scinum;
  }
  else {
    return(SCITOLL_NO_E);
  }
  
  if (offset == 0) 
    return(SCITOLL_NO_MANTISSA);

  if (offset == strlen(scinum))
    return(SCITOLL_NO_EXPONENT);

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
  long long output;
  output = atoll(mantissa)*pow(10,atoll(exponent));
  return output;
}
    

int ttd_delay_read_cli(int argc, char* argv[]) {
  ttd_delay_cli_args.verbose = 0;
  ttd_delay_cli_args.infile_allocated = 0;
  ttd_delay_cli_args.outfile_allocated = 0;
  ttd_delay_cli_args.delay = 0;

  char *found_e, *found_E;

  // Parse command line
  int option_index, opt;
  opt = getopt_long(argc, argv, ttd_delay_optstring, ttd_delay_longopts, &option_index);
  while (opt != -1) {
    switch (opt) {
    case 'v':
      ttd_delay_cli_args.verbose = 1;
      break;

    case 'V':
      ttp_print_version(argv[0]);
      return(TTD_DELAY_EXIT);

    case 'h':
      ttd_delay_print_help(argv[0]);
      return(TTD_DELAY_EXIT);

    case 'i':
      ttd_delay_cli_args.infile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttd_delay_cli_args.infile_allocated = 1;
      strcpy(ttd_delay_cli_args.infile, optarg);
      break;

    case 'o':
      ttd_delay_cli_args.outfile = (char *)malloc((strlen(optarg)+1)*sizeof(char));
      ttd_delay_cli_args.outfile_allocated = 1;
      strcpy(ttd_delay_cli_args.outfile, optarg);
      break;

    case 'T':
      found_e = strrchr(optarg, 'e');
      found_E = strrchr(optarg, 'E'); 
      if ((found_e == NULL) && (found_E == NULL)) {
	ttd_delay_cli_args.delay = atoll(optarg);
      }
      else {
	//TODO: scitoll error handling here
	ttd_delay_cli_args.delay = scitoll(optarg);
      }
    default:
      break;
    }
    opt = getopt_long(argc, argv, ttd_delay_optstring, ttd_delay_longopts, &option_index);
  }
  return(0);
}


// TODO: Error handling
int main(int argc, char *argv[]) {
  int exitcode = 0;
  
  ttd_delay_read_cli(argc, argv);

  int outfile_open=0;
  FILE *outfile;
  outfile = fopen(ttd_delay_cli_args.outfile, "wb");
  outfile_open=1;


  printf("Input file: %s\n", ttd_delay_cli_args.infile);
  printf("Output file: %s\n", ttd_delay_cli_args.outfile);
  printf("Delay Time: %" PRId64 " ps\n", ttd_delay_cli_args.delay);
  
  ttd_fb_t fb;
  ttd_fb_init(&fb, 16384, ttd_delay_cli_args.infile, ttd_delay_cli_args.delay);

  ttd_t time;
  while (!(fb.empty)) {
    fwrite(&time, sizeof(ttd_t), 1, outfile); 
    time = ttd_fb_pop(&fb);
  }
  
 cleanup_fb:
  ttd_fb_cleanup(&fb);
 cleanup_outfile:
  if (outfile_open) {
    fclose(outfile);
    outfile_open = 0;
  }
 cleanup_cli:
  if (ttd_delay_cli_args.infile_allocated) {
    free(ttd_delay_cli_args.infile);
    ttd_delay_cli_args.infile_allocated = 0;
  }
  if (ttd_delay_cli_args.outfile_allocated) {
    free(ttd_delay_cli_args.outfile);
    ttd_delay_cli_args.outfile_allocated = 0;
  }
  exit(exitcode);
}
  
  
  
