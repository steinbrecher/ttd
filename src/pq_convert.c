#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <getopt.h>
#include <time.h>
#include <string.h>

#include "ttd.h"
#define PQ_CONVERT
#include "hh_header.h"
#include "pq_convert.h"
#include "ttd.h"

//NOTE: Potential improvement is to pass the channel array structure to the various
// '_to_ttd' functions and have them handle the updates

int ht2_v1_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, ttd_t *overflow_correction) {
  // Unpack TRec
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;
  
  uint64_t realtime;

  if (special == 1) {
    if (channel==0x3F) {
      *overflow_correction += OLDHT2WRAPAROUND;
    }
    // Sync record
    else if (channel == 0) {
      // Channel indices are 0, 1, ... convert_properties.channels, so this is the 'extra' index for sync
      *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
      return(convert_properties.channels); 
    }
    return(-1);
  }
  else {
    // Resolution of ht2 version 1 was 1/2 picosecond
    *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
    return(channel);
  }
}

int ht2_v2_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, ttd_t *overflow_correction) {
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;

  if (special == 1) {
    if (channel==0x3F) {
      if (timetag < 2) {
	*overflow_correction += HT2WRAPAROUND;
      }
      else {
	*overflow_correction += HT2WRAPAROUND * timetag;
      }
    }
    // Sync record
    else if (channel==0) {
      *ttd_rec = *overflow_correction + timetag;
      // Channel indices are 0, 1, ... convert_properties.channels, so this is the 'extra' index for sync
      return(convert_properties.channels); 
    }
  }
  else {
    *ttd_rec = *overflow_correction + timetag;
    return(channel);
  }
  return(-1);
}

int ht3_v1_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, ttd_t *overflow_correction) {
  uint64_t sync_period = convert_properties.sync_period;
  uint64_t resolution = convert_properties.resolution;
  int special = TRec.T3bits.special;
  int channel = TRec.T3bits.channel;
  int nsync = TRec.T3bits.nsync;
  int dtime = TRec.T3bits.dtime;

  if (special == 1) {
    if (TRec.T3bits.channel==0x3F) {
      *overflow_correction += HT3WRAPAROUND;
    }
  }
  else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(channel);
  }
  return(-1);
}


int ht3_v2_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, ttd_t *overflow_correction) {
  uint64_t sync_period = convert_properties.sync_period;
  uint64_t resolution = convert_properties.resolution;
  int special = TRec.T3bits.special;
  int channel = TRec.T3bits.channel;
  int nsync = TRec.T3bits.nsync;
  int dtime = TRec.T3bits.dtime;

  if (special == 1) {
    if (channel==0x3F) {
      if (nsync < 2) {
	*overflow_correction += HT3WRAPAROUND;
      }
      else {
	*overflow_correction += nsync*HT3WRAPAROUND;
      }
    }
  }
  else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(channel);
  }
  return(-1);
}


uint64_t run_hh_convert(FILE *fpin) {
  int channels = convert_properties.channels;
  int meas_mode = convert_properties.meas_mode;
  int file_format_version = convert_properties.file_format_version;

  // Output sync records as well in HT2 mode
  // TODO: Make this a command line switch
  if (meas_mode == 2) {
    channels++;
  }

  ttd_t ttd_blocks[channels][PHOTONBLOCK];

  int j, k;
  // Select appropriate version of to_ttd
  int (*to_ttd)(pq_hh_rec_t, ttd_t *, uint64_t *);
  if (meas_mode == 2) {
    if (file_format_version == 1) {
      to_ttd = &ht2_v1_to_ttd;
    }
    else if (file_format_version == 2) {
      to_ttd = &ht2_v2_to_ttd;
    }
    else {
      return(-1);
    }
  }
  else if (meas_mode == 3) {
    if (file_format_version == 1) {
      to_ttd = &ht3_v1_to_ttd;
    }
    else if (file_format_version == 2) {
      to_ttd = &ht3_v2_to_ttd;
    }
    else {
      return(-1);
    }
  }

  uint64_t num_photons = PHOTONBLOCK;
  uint64_t total_read=0;
  uint64_t overflow_correction = 0;

  ttd_t ttd_record;
  int ttd_buffer_count[channels];

  pq_hh_rec_t *file_block = (pq_hh_rec_t *) malloc(PHOTONBLOCK*sizeof(pq_hh_rec_t));
  int ret, channel;

  // Open the output files
  FILE* outfiles[channels];
  char fname[80];
  for (k=0; k<channels; k++) {
    snprintf(fname, sizeof(fname), "channel%d.ttd", k);
    outfiles[k] = fopen(fname, "wb");
  }

  uint64_t n;
  while (num_photons == PHOTONBLOCK) {
    // Read file block
    num_photons = fread(file_block, sizeof(pq_hh_rec_t), PHOTONBLOCK, fpin);

    // Set buffer counters to 0
    for (k=0; k<channels; k++) {
      ttd_buffer_count[k] = 0;
    }

    // Read data into per-channel buffers
    for (n=0; n < num_photons; n++) {
      total_read++;
      channel = to_ttd(file_block[n], &ttd_record, &overflow_correction);
      if (channel != -1) {
	ttd_blocks[channel][ttd_buffer_count[channel]] = ttd_record;
	++ ttd_buffer_count[channel];
      }
    }
    
    // Write data to outfiles
    // fwrite(data, sizeof(element), sizeof(array), file);
    for (k=0; k<channels; k++) {
	fwrite(&(ttd_blocks[k][j]), sizeof(ttd_record), ttd_buffer_count[k], outfiles[k]);
    }
  }

  // Close the output files
  for (k=0; k < channels; k++) {
    fclose(outfiles[k]);
  }

  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != pq_hh_hdr_tttr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  return(total_read);
}

int main(int argc, char* argv[]) {
  FILE *ht_file;

  // Try to open the input file
  if((ht_file = fopen(argv[argc-1],"rb")) == NULL) { 
      printf("\n ERROR: Input file cannot be opened.\n"); 
      return(-1);
    }

  if (read_header(ht_file) < 0) { 
      fclose(ht_file);
      printf("\n ERROR: Cannot read header. Is %s an HT2 or HT3 file?", argv[1]);
      return(-1);
    }
  write_convert_properties();

  // Benchmarking timers
  clock_t start, diff; 		

  printf("\n");
  start = clock();
  run_hh_convert(ht_file);
  diff = clock() - start;

  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);

  fclose(ht_file);
  exit(0);
}
