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

#ifndef HT_CONVERT_SEEN
#define HT_CONVERT_SEEN


#include "hh_header.h"

//NOTE: Potential improvement is to pass the channel array structure to the various
// '_to_pqb' functions and have them handle the updates

typedef struct {
  uint8_t channel;
  double time;
} pqb_t; // As in, 'picoquant Binary'

typedef struct { pqb_t record; } pqb_buffer_t;

typedef struct { pqb_buffer_t *buffer; } pqb_buffer_group_t;


int ht2_v1_to_pqb(tTRec TRec, pqb_t *pqb_rec, double *overflow_correction) {

  // Unpack TRec
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;
  
  double realtime;
  double resolution=0.5; // In version 1, T2 mode resolution locked to 0.5ps

  if (special == 1) {
    if (channel==0x3F) {
      *overflow_correction += OLDHT2WRAPAROUND;
    }
    return(0);
  }
  else {
    pqb_rec->channel = channel;
    pqb_rec->time = (*overflow_correction + timetag)*resolution;
    return(1);
  }
}

int ht2_v2_to_pqb(tTRec TRec, pqb_t *pqb_rec, double *overflow_correction) {
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;

  double realtime;

  if (special == 1) {
    if (channel==0x3F) {
      if (timetag < 2) {
	*overflow_correction += OLDHT2WRAPAROUND;
      }
      else {
	*overflow_correction += OLDHT2WRAPAROUND * timetag;
      }
    }
  }
  else {
    pqb_rec->channel = channel;
    pqb_rec->time = *overflow_correction + timetag;
    return(1);
  }
  return(0);
}

int ht3_v1_to_pqb(tTRec TRec, pqb_t *pqb_rec, double *overflow_correction) {
  double realtime;
  double sync_period = convert_properties.sync_period;
  double resolution = convert_properties.resolution;
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
    pqb_rec->channel = channel;
    pqb_rec->time = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(1);
  }
  return(0);
}


int ht3_v2_to_pqb(tTRec TRec, pqb_t *pqb_rec, double *overflow_correction) {
  double realtime;
  double sync_period = convert_properties.sync_period;
  double resolution = convert_properties.resolution;
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
    pqb_rec->channel = channel;
    pqb_rec->time = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(1);
  }
  return(0);
}


typedef struct { FILE *fp; } file_group_t;

uint64_t run_hh_convert(FILE *fpin) {
  int channels = convert_properties.channels;
  int meas_mode = convert_properties.meas_mode;
  int file_format_version = convert_properties.file_format_version;

  pqb_buffer_group_t pqb_buffer_group[channels];

  int j, k;
  for (k=0; k<channels; k++) {
    pqb_buffer_group[k].buffer = (pqb_buffer_t *) malloc(PHOTONBLOCK * sizeof(pqb_t));
  }

  // Select appropriate version of to_pqb
  int (*to_pqb)(tTRec, pqb_t *, double *);
  if (meas_mode == 2) {
    if (file_format_version == 1) {
      to_pqb = &ht2_v1_to_pqb;
    }
    else if (file_format_version == 2) {
      to_pqb = &ht2_v2_to_pqb;
    }
    else {
      return(-1);
    }
  }
  else if (meas_mode == 3) {
    if (file_format_version == 1) {
      to_pqb = &ht3_v1_to_pqb;
    }
    else if (file_format_version == 2) {
      to_pqb = &ht3_v2_to_pqb;
    }
    else {
      return(-1);
    }
  }

  uint64_t num_photons = PHOTONBLOCK;
  uint64_t total_read=0;
  double overflow_correction = 0;

  pqb_t pqb_record;
  int pqb_buffer_count[channels];

  tTRec *file_block = (tTRec *) malloc(PHOTONBLOCK*sizeof(tTRec));
  int ret, channel;

  // Open the output files
  file_group_t outfiles[channels];
  char fname[80];
  for (k=0; k<channels; k++) {
    snprintf(fname, sizeof(fname), "channel%d.pqb", k);
    outfiles[k].fp = fopen(fname, "wb");
  }

  uint64_t n;
  while (num_photons == PHOTONBLOCK) {
    // Read file block
    num_photons = fread(file_block, sizeof(tTRec), PHOTONBLOCK, fpin);

    // Set buffer counters to 0
    for (k=0; k<channels; k++) {
      pqb_buffer_count[k] = 0;
    }

    // Read data into per-channel buffers
    for (n=0; n < num_photons; n++) {
      total_read++;
      ret = to_pqb(file_block[n], &pqb_record, &overflow_correction);
      if (ret==1) {
	channel = pqb_record.channel;
	pqb_buffer_group[channel].buffer[pqb_buffer_count[channel]] = (pqb_buffer_t){ pqb_record };
	++ pqb_buffer_count[channel];
      }
    }
    
    // Write data to outfiles
    // fwrite(data, sizeof(element), sizeof(array), file);
    for (k=0; k<channels; k++) {
      fwrite(pqb_buffer_group[channel].buffer, sizeof(pqb_record), pqb_buffer_count[k], outfiles[k].fp);
    }
  }
  // Close the output files
  for (k=0; k < channels; k++) {
    fclose(outfiles[k].fp);
  }

  // Deallocate malloc'd memory
  free(file_block);
  for (k=0; k < channels; k++) {
    free(pqb_buffer_group[k].buffer);
  }

  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != TTTRHdr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  return(total_read);
}

#endif // HT_CONVERT_SEEN
