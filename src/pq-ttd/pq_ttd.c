#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

#include "pq_ttd.h"

int16_t pt2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int16_t channel = (int16_t) pq_rec.ph_t2_bits.channel;
  int32_t timetag = (int32_t) pq_rec.ph_t2_bits.timetag;

  if (channel == 0xF) { // Special record
    if ((timetag & 0xF) == 0) { // Overflow Record
      *overflow_correction += PQ_PT2_V2_WRAP;
    }
  } else {
    // Resolution is always 4ps here
    *ttd_rec = (*overflow_correction + timetag) * 4;
    return (channel);
  }
  return (-1);
}

int16_t pt3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int16_t channel = (int16_t) pq_rec.ph_t3_bits.channel;
  int32_t timetag = (int32_t) pq_rec.ph_t3_bits.timetag;
  int32_t nsync = (int32_t) pq_rec.ph_t3_bits.nsync;
  if (channel == 0xF) { // Overflow
    if (timetag == 0) { // Overflow Record
      *overflow_correction += PQ_PT3_V2_WRAP;
    }
  } else {
    *ttd_rec = (*overflow_correction + nsync) * file_info->sync_period + timetag * file_info->resolution;
    return (channel);
  }
  return (-1);
}


int16_t ht2_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  // Unpack pq_rec
  int16_t special = (int16_t) pq_rec.hh_t2_bits.special;
  int16_t channel = (int16_t) pq_rec.hh_t2_bits.channel;
  int32_t timetag = (int32_t) pq_rec.hh_t2_bits.timetag;

  if (special == 1) {
    if (channel == 0x3F) {
      *overflow_correction += PQ_HT2_V1_WRAP;
    }
      // Sync record
    else if (channel == 0) {
      // Channel indices are 0, 1, ... file_info->num_channels-1, so this is the 'extra' index for sync
      *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
      return (file_info->num_channels);
    }
    return (-1);
  } else {
    // Resolution of ht2 version 1 was 1/2 picosecond
    *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
    return (channel);
  }
}

int16_t ht2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int16_t special = pq_rec.hh_t2_bits.special;
  int16_t channel = pq_rec.hh_t2_bits.channel;
  int32_t timetag = pq_rec.hh_t2_bits.timetag;

  if (special == 1) {
    if (channel == 0x3F) {
      if (timetag < 2) {
        *overflow_correction += PQ_HT2_V2_WRAP;
      } else {
        *overflow_correction += PQ_HT2_V2_WRAP * timetag;
      }
    }
      // Sync record
    else if (channel == 0) {
      *ttd_rec = *overflow_correction + timetag;
      // Channel indices are 0, 1, ... file_info->num_channels-1, so this is the 'extra' index for sync
      return (file_info->num_channels);
    }
  } else {
    *ttd_rec = *overflow_correction + timetag;
    return (channel);
  }
  return (-1);
}

int16_t ht3_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  uint64_t sync_period = file_info->sync_period;
  uint64_t resolution = file_info->resolution;
  int special = pq_rec.hh_t3_bits.special;
  int channel = pq_rec.hh_t3_bits.channel;
  int nsync = pq_rec.hh_t3_bits.nsync;
  int dtime = pq_rec.hh_t3_bits.dtime;

  if (special == 1) {
    if (pq_rec.hh_t3_bits.channel == 0x3F) {
      *overflow_correction += PQ_HT3_V1_WRAP;
    }
  } else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return (channel);
  }
  return (-1);
}


int16_t ht3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  uint64_t sync_period = file_info->sync_period;
  uint64_t resolution = file_info->resolution;
  int special = pq_rec.hh_t3_bits.special;
  int channel = pq_rec.hh_t3_bits.channel;
  int nsync = pq_rec.hh_t3_bits.nsync;
  int dtime = pq_rec.hh_t3_bits.dtime;

  if (special == 1) {
    if (channel == 0x3F) {
      if (nsync < 2) {
        *overflow_correction += PQ_HT3_V2_WRAP;
      } else {
        *overflow_correction += nsync * PQ_HT3_V2_WRAP;
      }
    }
  } else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return (channel);
  }
  return (-1);
}

int16_t get_pq_converter(pq_to_ttd_t *to_ttd, pq_fileinfo_t *file_info) {
  int instrument = file_info->instrument;
  int meas_mode = file_info->meas_mode;
  int fmt_version = file_info->fmt_version;
  int16_t retcode = 0;

  if (instrument == PQ_PH) {
    if (meas_mode == 2) {
      *to_ttd = &pt2_v2_to_ttd;
    } else if (meas_mode == 3) {
      *to_ttd = &pt3_v2_to_ttd;
    } else {
      retcode = -1;
    }
  } else if (instrument == (PQ_HH)) {
    if (meas_mode == 2) {
      if (fmt_version == 1) {
        *to_ttd = &ht2_v1_to_ttd;
      } else if (fmt_version == 2) {
        *to_ttd = &ht2_v2_to_ttd;
      } else {
        retcode = -1;
      }
    } else if (meas_mode == 3) {
      if (fmt_version == 1) {
        *to_ttd = &ht3_v1_to_ttd;
      } else if (fmt_version == 2) {
        *to_ttd = &ht3_v2_to_ttd;
      } else {
        retcode = -1;
      }
    }
  } else {
    retcode = -1;
  }
  return retcode;
}

uint64_t run_hh_convert(FILE *fpin, pq_fileinfo_t *file_info) {
  int channels = file_info->num_channels;
  //int instrument = file_info->instrument;
  int meas_mode = file_info->meas_mode;
  //int fmt_version = file_info->fmt_version;
  //int retcode;

  // Output sync records as well in HT2 mode
  if ((file_info->instrument == PQ_HH) && (meas_mode == 2) && pq_ttd_cli_args.output_sync) {
    channels++;
  }

  ttd_t ttd_blocks[channels][PHOTONBLOCK];

  int k;
  // Select appropriate version of to_ttd
  pq_to_ttd_t to_ttd;
  get_pq_converter(&to_ttd, file_info);

  uint64_t num_photons = PHOTONBLOCK;
  uint64_t total_read = 0;
  uint64_t overflow_correction = 0;

  ttd_t ttd_record;
  int ttd_buffer_count[channels];

  pq_rec_t *file_block = (pq_rec_t *) malloc(PHOTONBLOCK * sizeof(pq_rec_t));
  if (file_block == NULL) {
    printf("ERROR: Photon Block Not Allocated!\n");
    exit(-1);
  }
  int channel;

  // Open the output files
  FILE *outfiles[channels];
  char fname[80];
  char suffix[] = "ttd";


  for (k = 0; k < channels; k++) {
    snprintf(fname, sizeof(fname), "%s-channel-%d.%s", pq_ttd_cli_args.output_prefix, k + 1, suffix);
    outfiles[k] = fopen(fname, "wb");
  }
  // If we're outputting sync, change name of the last file
  if (pq_ttd_cli_args.output_sync) {
    snprintf(fname, sizeof(fname), "%s-channel-sync.%s", pq_ttd_cli_args.output_prefix, suffix);
    fclose(outfiles[channels - 1]);
    outfiles[channels - 1] = fopen(fname, "wb");
  }

  uint64_t n;

  while (num_photons == PHOTONBLOCK) {
    // Read file block
    num_photons = fread(file_block, sizeof(pq_rec_t), PHOTONBLOCK, fpin);

    // Set buffer counters to 0
    for (k = 0; k < channels; k++) {
      ttd_buffer_count[k] = 0;
    }

    // Read data into per-channel buffers
    for (n = 0; n < num_photons; n++) {
      total_read++;
      channel = to_ttd(file_block[n], &ttd_record, &overflow_correction, file_info);
      if (channel != -1) {
        ttd_blocks[channel][ttd_buffer_count[channel]] = ttd_record;
        ++ttd_buffer_count[channel];
      }
    }

    // Write data to outfiles
    // fwrite(data, sizeof(element), sizeof(array), file);
    for (k = 0; k < channels; k++) {
      fwrite(&(ttd_blocks[k]), sizeof(ttd_record), ttd_buffer_count[k], outfiles[k]);
    }
  }

  // Close the output files
  for (k = 0; k < channels; k++) {
    fclose(outfiles[k]);
  }
  free(file_block);


  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != file_info->num_records) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  return (total_read);
}


/*

*/