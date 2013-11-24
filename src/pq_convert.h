#ifndef PHOTONBLOCK
#define PHOTONBLOCK 32768
#endif 

#ifndef T2WRAPAROUND
#define T2WRAPAROUND 33554432 // 2^25
#endif

#ifndef OLDHT2WRAPAROUND
#define OLDHT2WRAPAROUND 33552000 // 2^25 - 2,432
#endif

#ifndef HT3WRAPAROUND
#define HT3WRAPAROUND 1024 // 2^10
#endif

#ifndef PQ_CONVERT_HEADER_SEEN
#define PQ_CONVERT_HEADER_SEEN

#include "hh_header.h"

//NOTE: Potential improvement is to pass the channel array structure to the various
// '_to_ttd' functions and have them handle the updates


int ht2_v1_to_ttd(tTRec TRec, ttd_t *ttd_rec, uint64_t *overflow_correction) {
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
      // TODO: Implement proper rounding here
      *ttd_rec = (*overflow_correction + timetag)/2;
      // Channel indices are 0, 1, ... convert_properties.channels, so this is the 'extra' index for sync
      return(convert_properties.channels); 
    }
    return(-1);
  }
  else {
    // Resolution of ht2 version 1 was 1/2 picosecond
    // TODO: Implement proper rounding here
    *ttd_rec = (*overflow_correction + timetag)/2;
    return(channel);
  }
}

int ht2_v2_to_ttd(tTRec TRec, ttd_t *ttd_rec, uint64_t *overflow_correction) {
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;

  if (special == 1) {
    if (channel==0x3F) {
      if (timetag < 2) {
	*overflow_correction += OLDHT2WRAPAROUND;
      }
      else {
	*overflow_correction += OLDHT2WRAPAROUND * timetag;
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

int ht3_v1_to_ttd(tTRec TRec, ttd_t *ttd_rec, uint64_t *overflow_correction) {
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


int ht3_v2_to_ttd(tTRec TRec, ttd_t *ttd_rec, uint64_t *overflow_correction) {
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


typedef struct { FILE *fp; } file_group_t;

uint64_t run_hh_convert(FILE *fpin) {
  int channels = convert_properties.channels;
  int meas_mode = convert_properties.meas_mode;
  int file_format_version = convert_properties.file_format_version;

  // Output sync records as well in HT2 mode
  // TODO: Make this a command line switch
  if (meas_mode == 2) {
    channels++;
  }

  ttd_buffer_group_t ttd_buffer_group[channels];

  int j, k;
  for (k=0; k<channels; k++) {
    ttd_buffer_group[k].buffer = (ttd_buffer_t *) malloc(PHOTONBLOCK * sizeof(ttd_t));
  }

  // Select appropriate version of to_ttd
  int (*to_ttd)(tTRec, ttd_t *, uint64_t *);
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

  tTRec *file_block = (tTRec *) malloc(PHOTONBLOCK*sizeof(tTRec));
  int ret, channel;

  // Open the output files
  file_group_t outfiles[channels];
  char fname[80];
  for (k=0; k<channels; k++) {
    snprintf(fname, sizeof(fname), "channel%d.ttd", k);
    outfiles[k].fp = fopen(fname, "wb");
  }

  uint64_t n;
  while (num_photons == PHOTONBLOCK) {
    // Read file block
    num_photons = fread(file_block, sizeof(tTRec), PHOTONBLOCK, fpin);

    // Set buffer counters to 0
    for (k=0; k<channels; k++) {
      ttd_buffer_count[k] = 0;
    }

    // Read data into per-channel buffers
    for (n=0; n < num_photons; n++) {
      total_read++;
      channel = to_ttd(file_block[n], &ttd_record, &overflow_correction);
      if (channel != -1) {
	ttd_buffer_group[channel].buffer[ttd_buffer_count[channel]] = (ttd_buffer_t){ ttd_record };
	++ ttd_buffer_count[channel];
      }
    }
    
    // Write data to outfiles
    // fwrite(data, sizeof(element), sizeof(array), file);
    for (k=0; k<channels; k++) {
	fwrite(&(ttd_buffer_group[k].buffer[j]), sizeof(ttd_record), ttd_buffer_count[k], outfiles[k].fp);
    }
  }
  // Close the output files
  for (k=0; k < channels; k++) {
    fclose(outfiles[k].fp);
  }

  // Deallocate malloc'd memory
  free(file_block);
  for (k=0; k < channels; k++) {
    free(ttd_buffer_group[k].buffer);
  }

  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  return(total_read);
}

#endif // HT_CONVERT_SEEN
