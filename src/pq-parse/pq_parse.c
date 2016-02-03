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

#include "ttd.h"
#include "pq_parse.h"


// This function is separated from the following because it's common to all
// PicoQuant files (PicoHarp and HydraHarp) and so might be useful to call
// from other code.
int pq_parse_filetype(FILE *fp, pq_fileinfo_t *file_info) {
  fread(&pq_file_text, sizeof(pq_file_text), 1, fp);

  if (strcmp(pq_file_text.instrument, PQ_HH_TEXT)==0) {
    file_info->instrument = PQ_HH;
    if (strcmp(pq_file_text.version, PQ_HH_V1_TEXT)==0) {
      file_info->fmt_version = PQ_HH_V1;
      return(0);
    }
    else if (strcmp(pq_file_text.version, PQ_HH_V2_TEXT)==0) {
      file_info->fmt_version = PQ_HH_V2;
      return(0);
    }
    else {
      return(PQ_FILETYPE_VERSION_NOT_RECOGNIZED);
    }
  }
  else if (strcmp(pq_file_text.instrument, PQ_PH_TEXT)==0) {
    file_info->instrument = PQ_PH;
    if (strcmp(pq_file_text.version, PQ_PH_V2_TEXT)==0) {
      file_info->fmt_version = PQ_PH_V2;
      return(0);
    }
    else {
      return(PQ_FILETYPE_VERSION_NOT_RECOGNIZED);
    }
  }
  else {
    return(PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED);
  }
  // Shouldn't get here, so not returning anything. If compiler throws a warning, 
  // look for a bug above.
}

int pq_parse_header(FILE *fp, pq_fileinfo_t *file_info) {
  int i, retcode=0;
  int image_header_size;
  // Instrument type and file format version  are necessary to decide how to parse the 
  // rest of the header
  retcode = pq_parse_filetype(fp, file_info);

  switch (retcode) {
  case PQ_FILETYPE_INSTRUMENT_NOT_RECOGNIZED:
    printf("ERROR: Didn't recognize the instrument type in this file.\n");
    printf("       Is it a PicoHarp or HydraHarp time-tagged file?\n");
    goto exit_block;

  case PQ_FILETYPE_VERSION_NOT_RECOGNIZED:
    printf("ERROR: File version %s not supported.\n", pq_file_text.version);
    goto exit_block;

  default:
    if (retcode < 0) {
      // Really shouldn't get here
      printf("Warning: pq_parse_filetype raised unrecognized error. Please report.\n");
      goto exit_block;
    }
    break;
  }

  if (file_info->instrument == PQ_HH) {
    // Read most of header, including # of channels
    fread(&pq_hh_header, sizeof(pq_hh_header), 1, fp);

    // Read channel-dependent data
    fread(&pq_hh_chanblock, sizeof(pq_hh_chan_t), pq_hh_header.num_input_channels, fp);
    fread(&pq_hh_rates, sizeof(int32_t), pq_hh_header.num_input_channels, fp);
    // Read tttr data
    fread(&pq_hh_tttr, sizeof(pq_hh_tttr), 1, fp);

    image_header_size = pq_hh_tttr.image_header_size;

    // Write the shared data structure information
    file_info->meas_mode = pq_hh_header.meas_mode;
    file_info->resolution = (ttd_t) round(pq_hh_header.resolution);
    file_info->sync_rate = pq_hh_tttr.sync_rate;
    file_info->num_records = pq_hh_tttr.num_records;
    file_info->num_channels = pq_hh_header.num_input_channels;
  }
  else if (file_info->instrument == PQ_PH) {
    // Reads through everything but the image header. PicoHarp has a nice static header!
    fread(&pq_ph_header, sizeof(pq_ph_header), 1, fp);
    image_header_size = pq_ph_header.image_header_size;
    
    file_info->meas_mode = pq_ph_header.meas_mode;
    if (file_info->meas_mode == PQ_T2_MODE) {
      file_info->resolution = 4;
    }
    else if (file_info->meas_mode == PQ_T3_MODE) {
      // Cast from float->double
      double res_d = (double) pq_ph_header.board_info.resolution;
      // Multiply by 1000 to go from ns->ps
      file_info->resolution = (ttd_t)round(res_d * 1e3);
    }
    file_info->sync_rate = pq_ph_header.chan0_inp_rate;
    file_info->num_records = pq_ph_header.num_records;
    file_info->num_channels = 2;
  }

  file_info->sync_period = (ttd_t) round(1e12 / ((double)file_info->sync_rate));
  fseek(fp, image_header_size*sizeof(pq_image_header_record), SEEK_CUR);
  
 exit_block:
  return(retcode);
}

void pq_printf_file_info(pq_fileinfo_t *file_info) {
  if (file_info->instrument == PQ_HH) {
    printf("Instrument: HydraHarp\n");
  }
  else if (file_info->instrument == PQ_PH) {
    printf("Instrument: PicoHarp\n");
  }
  else {
    printf("Unrecognized Instrument!\n");
  }
  if (file_info->meas_mode == PQ_T2_MODE) {
    printf("Measurement Mode: T2\n");
  }
  else if (file_info->meas_mode == PQ_T3_MODE) {
    printf("Measurement Mode: T3\n");
  }
  else {
    printf("Unrecgonized Measurement Mode!\n");
  }
  printf("File Format Version: %d\n", file_info->fmt_version);
  printf("Number of records: %" PRId64 "\n", file_info->num_records);
  printf("Timing Resolution: %" PRIu64 " ps\n", file_info->resolution);
  printf("Sync Period: %" PRIu64 " ps\n", file_info->sync_period);
  printf("Sync Rate: %d Hz\n", file_info->sync_rate);
}

  
  
