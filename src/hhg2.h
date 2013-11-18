#ifndef READ_HT_SEEN
#define READ_HT_SEEN

#include "global_args.h"
#include "hh_header.h"
#include "correlation.h"

#define T2WRAPAROUND 33554432 // 2^25
#define OLDT2WRAPAROUND 33552000 // 2^25 - 2,432
#define T3WRAPAROUND 1024 // 2^10

#define PHOTONBLOCK 32768

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

void g2_insert(double realtime, int channel, TimeBufferGroup *tbs, CorrelationGroup *corrs) {
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
    correlationUpdate(&(corrs[pair_lookup(channel, other_chan)].corr), 
		      &(tbs[other_chan].buffer), 
		      channel, 
		      realtime);
  }

}

void ht2_v1_process(tTRec TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *corrs) {
  int channel, other_chan,m;
  double realtime, resolution=0.5; // In version 1, T2 mode resolution locked to 0.5ps

  if (TRec.T2bits.special == 1) {
    if (TRec.T2bits.channel==0x3F) {
      *overflow_correction += OLDT2WRAPAROUND;
    }
  }
  else {
    realtime = (*overflow_correction + TRec.T2bits.timetag)*resolution;
    channel = TRec.T2bits.channel;
    g2_insert(realtime, channel, tbs, corrs);
  }
}

void ht2_v2_process(tTRec TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *corrs) {
  int other_chan, m;
  double realtime, resolution=1; // In version 2, T2 mode resolution changed to 1ps
  int special = TRec.T2bits.special;
  int channel = TRec.T2bits.channel;
  int timetag = TRec.T2bits.timetag;

  if (special == 1) {
    if (channel==0x3F) {
      if (timetag < 2) {
	*overflow_correction += OLDT2WRAPAROUND;
      }
      else {
	*overflow_correction += OLDT2WRAPAROUND*timetag;
      }
    }
  }
  else {
    realtime = (*overflow_correction + timetag)*resolution;
    channel = TRec.T2bits.channel;
    g2_insert(realtime, channel, tbs, corrs);
  }
}

void ht3_v1_process(tTRec TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *corrs) {
  int channel, other_chan,m;
  double realtime;

  if (TRec.T3bits.special == 1) {
    if (TRec.T3bits.channel==0x3F) {
      *overflow_correction += T3WRAPAROUND;
    }
  }
  else {
    realtime = (*overflow_correction + TRec.T3bits.nsync) * SyncPeriod + TRec.T3bits.dtime*BinHdr.Resolution;
    channel = TRec.T3bits.channel;
    g2_insert(realtime, channel, tbs, corrs);
  }
}

void ht3_v2_process(tTRec TRec, double *overflow_correction, 
		    TimeBufferGroup *tbs, CorrelationGroup *corrs) {
  double realtime;
  int special = TRec.T3bits.special;
  int channel = TRec.T3bits.channel;
  int nsync = TRec.T3bits.nsync;


  if (special == 1) {
    if (channel==0x3F) {
      if (nsync < 2) {
	*overflow_correction += T3WRAPAROUND;
      }
      else {
	*overflow_correction += nsync*T3WRAPAROUND;
      }
    }
  }
  else {
    realtime = (*overflow_correction + TRec.T3bits.nsync) * SyncPeriod + TRec.T3bits.dtime*BinHdr.Resolution;
    channel = TRec.T3bits.channel;
    g2_insert(realtime, channel, tbs, corrs);
  }
}

uint64_t run_g2(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs) { 
  tTRec TRec;
  uint64_t n, m, num_photons, total_read=0;

  double overflow_correction=0;

  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK*sizeof(TRec.allbits));

  void (*process)(tTRec, double *, TimeBufferGroup *, CorrelationGroup *);
  // Select which function to use to process photon records
  if (BinHdr.MeasMode == 2) {
    if (strcmp(TxtHdr.FormatVersion, "1.0")==0) {
      process = &ht2_v1_process;
    }
    else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) {
      process = &ht2_v2_process;
    }
    else {
      return(-1);
    }
  }
  else if (BinHdr.MeasMode == 3) {
    if (strcmp(TxtHdr.FormatVersion, "1.0")==0) {
      process = &ht3_v1_process;
    }
    else if (strcmp(TxtHdr.FormatVersion, "2.0")==0) {
      process = &ht3_v2_process;
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
      process(file_block[n], &overflow_correction, tbs, corrs);
      /* if ((n%1000)==0) { */
      /* 	printf("%" PRIu64 ", %g\n", total_read, tbLastTime(&(tbs[0].buffer))); */
      /* } */
    }
  }
  free(file_block);
  return(total_read);
}

#endif	/* READ_HT_SEEN */
