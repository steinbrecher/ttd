#ifndef PHOTONBLOCK
#define PHOTONBLOCK 32768
#endif 

#ifndef READ_HT_SEEN
#define READ_HT_SEEN

#include "hh_header.h"
#include "correlation.h"

#define T2WRAPAROUND 33554432 // 2^25
#define OLDHT2WRAPAROUND 33552000 // 2^25 - 2,432
#define HT3WRAPAROUND 1024 // 2^10

// Silly hack to speed up conditional
static inline int pair_lookup (int a, int b) {
  switch ( (10 * a + b) ) 
    {
    case 1: return(0);
    case 10: return(0);

    case 2: return(1);
    case 20: return(1);

    case 3: return(2);
    case 30: return(2);

    case 12: return(3);
    case 21: return(3);

    case 13: return(4);
    case 31: return(4);

    case 23: return(5);
    case 32: return(5);

    default: return(-1);
    }
}

void g2_insert(double realtime, int channel, TimeBufferGroup *tbs, CorrelationGroup *correlations) {
  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  tbWrite(&(tbs[channel].buffer), realtime);

  int m;
  for (m=0; m<4; m++) {
    tbPrune(&(tbs[m].buffer), realtime);
  }

  int other_chan;
  for (m=0; m<3; m++) {
    other_chan = lookup_others[channel][m];
    correlationUpdate(&(correlations[pair_lookup(channel, other_chan)].corr), 
		      &(tbs[other_chan].buffer), 
		      channel, 
		      realtime);
  }
}

void ht2_v1_g2(pq_hh_rec_t TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *correlations) {
  // Unpack TRec
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;

  int other_chan;
  double realtime;
  double resolution=0.5; // In version 1, T2 mode resolution locked to 0.5ps

  if (special == 1) {
    if (channel == 0x3F) {
      *overflow_correction += OLDHT2WRAPAROUND;
    }
  }
  else {
    realtime = (*overflow_correction + timetag)*resolution;
    g2_insert(realtime, channel, tbs, correlations);
  }
}

void ht2_v2_g2(pq_hh_rec_t TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *correlations) {
  int other_chan;
  // In version 2, T2 mode resolution changed to 1ps, so we don't need a resolution variable
  double realtime; 
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
  }
  else {
    realtime = *overflow_correction + timetag;
    channel = TRec.T2bits.channel;
    g2_insert(realtime, channel, tbs, correlations);
  }
}

void ht3_v1_g2(pq_hh_rec_t TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *correlations) {
  int other_chan;
  double realtime;
  double sync_period = g2_properties.sync_period;
  double resolution = g2_properties.resolution;

  int special = TRec.T3bits.special;
  int channel = TRec.T3bits.channel;
  int nsync = TRec.T3bits.nsync;
  int dtime = TRec.T3bits.dtime;


  if (special == 1) {
    if (channel == 0x3F) {
      *overflow_correction += HT3WRAPAROUND;
    }
  }
  else {
    realtime = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    g2_insert(realtime, channel, tbs, correlations);
  }
}

void ht3_v2_g2(pq_hh_rec_t TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *correlations) {
  double realtime;
  double sync_period = g2_properties.sync_period;
  double resolution = g2_properties.resolution;

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
    realtime = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    g2_insert(realtime, channel, tbs, correlations);
  }
}

void output_g2_csv(CorrelationGroup *correlations) {
  FILE *data_file;
  char fname[80];
  int n,pairs = g2_properties.channel_pairs;
  uint64_t m;
  double bin_time = g2_properties.bin_time;
  double correlation_window = g2_properties.correlation_window;

  // Outputs the hist files
  for (n=0; n < pairs; n++) {
    snprintf(fname, sizeof(fname), "hist_%d%d.csv", // Most reliable way to create string from int 
	     correlations[n].corr.chan1, correlations[n].corr.chan2); 
    data_file = fopen(fname,"wb");

    for (m=0; m < correlations[n].corr.num_bins; m++) {
      fprintf(data_file, "%g, %g\n", 
	      ((m*bin_time) - correlation_window), (double)(correlations[n].corr.hist[m].counts));
    }

    fclose(data_file);
  }
}


// NOTE: This assumes that the file pointer is pointing at the part of the file with photon records. 
uint64_t run_g2(FILE *fpin) { 
  pq_hh_rec_t TRec;
  uint64_t n, m, num_photons, total_read=0;

  double overflow_correction=0;
  double correlation_window = g2_properties.correlation_window;


  // Read out necessary g2_properties
  int channels = g2_properties.channels;
  int pairs = g2_properties.channel_pairs;
  int meas_mode = g2_properties.meas_mode;
  int file_format_version = g2_properties.file_format_version;

  TimeBufferGroup tbs[channels]; // Container of ring buffers for arrival times 
  CorrelationGroup correlations[pairs]; // Container of correlation tracking objects 

  // Note: Need to free tb->times for each of these when done
  for (n=0; n < channels; n++) {
    tbInit(&(tbs[n].buffer), n, 4096, correlation_window);
  }

  // Note: Need to free corr->hist for each of these when done
  for (n=0; n < channels-1; n++) {
      for (m=n+1; m < channels; m++) {
	  corrInit(&(correlations[pair_lookup(n,m)].corr), n, m);
	}
    }

  // Allocate memory for mapping blocks of the input file
  pq_hh_rec_t *file_block = (pq_hh_rec_t *) malloc(PHOTONBLOCK*sizeof(TRec.allbits));

  // Function pointer to the generic g2 function call (mode and version dependent)
  void (*g2)(pq_hh_rec_t, double *, TimeBufferGroup *, CorrelationGroup *);

  // Select which function to use to g2 photon records
  if (meas_mode == 2) {
    if (file_format_version == 1) {
      g2 = &ht2_v1_g2;
    }
    else if (file_format_version == 2) {
      g2 = &ht2_v2_g2;
    }
    else {
      return(-1);
    }
  }
  else if (meas_mode == 3) {
    if (file_format_version == 1) {
      g2 = &ht3_v1_g2;
    }
    else if (file_format_version == 2) {
      g2 = &ht3_v2_g2;
    }
    else {
      return(-1);
    }
  }

  num_photons = PHOTONBLOCK;
  while (num_photons == PHOTONBLOCK) {
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for (n=0; n < num_photons; n++) {
      total_read++;
      g2(file_block[n], &overflow_correction, tbs, correlations);
    }
  }

  // Output Statistics
  printf("\n*********************************** g2 Results ***********************************\n");
  for(n=0; n<channels; n++) {
      printf("Total Photons on Channel %" PRIu64 ": %" PRIu64 "\n", n, tbs[n].buffer.total_counts);
    }
  printf("\n");
  for (n=0; n<pairs; n++) {
    printf("Total Counts in Correlation %d->%d: %" PRIu64 "\n", 
	   correlations[n].corr.chan1, correlations[n].corr.chan2, correlations[n].corr.total);
  }

  // Write to files
  output_g2_csv(correlations);

  // Free allocated memory
  free(file_block);

  for (n=0; n < channels; n++) {
    free(tbs[n].buffer.times);
  }

  for (n=0; n < channels-1; n++) {
      for (m=n+1; m < channels; m++) {
	free(correlations[pair_lookup(n,m)].corr.hist);
	}
    }

  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != pq_hh_hdr_tttr.nRecords) {
    printf("\nWARNING: Did not reach end of file.\n");
  }


  
  return(total_read);
}

#endif	/* READ_HT_SEEN */
