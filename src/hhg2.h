#ifndef READ_HT_SEEN
#define READ_HT_SEEN

#include "global_args.h"
#include "hh_header.h"

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

typedef struct {
  int chan1;
  int chan2;
  
  int num_bins;
  int center_bin;
  double correlation_window;
  double bin_time;

  uint64_t total;
  
  Histogram *hist;
} Correlation;

typedef struct { Correlation corr; } CorrelationGroup;

void corrInit(Correlation *corr, int chan1, int chan2) {
  int num_bins;
  corr->chan1 = chan1;
  corr->chan2 = chan2;
  
  num_bins = 2*(int) floor(global_args.correlation_window / global_args.bin_time) + 1;
  corr->num_bins = num_bins;
  corr->center_bin = (corr->num_bins - 1)/2;
  corr->correlation_window = global_args.correlation_window;
  corr->bin_time = global_args.bin_time;
  
  corr->total = 0;
  corr->hist = (Histogram *)malloc(num_bins * sizeof(Histogram));
}

void correlationUpdate(Correlation *corr, TimeBuffer *tb, int new_chan, double new_time) {
  int n, sign=1;
  double delta, delta_b;
  
  if (tb->count > 0) {
    // This ensures that deltaT is (T2 - T1) even when (new_chan == chan2)
    // Strangely, !=chan1 seems to result in ~5% faster runtime than ==chan2
    if (new_chan != corr->chan1) { 
	sign = -1;
      }

    for (n=0; n < tb->count; n++) {
      delta = sign * (new_time - tbGet(tb, n));
      delta_b = floor(corr->center_bin + (delta / corr->bin_time));
      ++ corr->hist[(int)delta_b].counts;
      ++ corr->total;
    }
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
		      channel, realtime);
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


uint64_t run_g2(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs) { 
  tTRec TRec;
  uint64_t n, m, num_photons, total_read=0;

  double overflow_correction=0;

  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK*sizeof(TRec.allbits));


  num_photons = PHOTONBLOCK;
  while (num_photons == PHOTONBLOCK) {
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for (n=0; n < num_photons; n++) {
      total_read++;
      ht3_v1_process(file_block[n], &overflow_correction, tbs, corrs);
    }
  }
  free(file_block);
  return(total_read);
}



uint64_t read_ht2_v1(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs, 
			     uint64_t *syncs, uint64_t *markers)
{

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };

  
  tTRec TRec;
  int channel, other_chan;
  uint64_t n, m, num_photons, overflow_corr=0, total_read=0;

  // In version 1, T2 mode had a 'resolution' of 0.5ps. 
  double realtime, resolution=5e-4;

  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK*sizeof(TRec.allbits));



  num_photons = PHOTONBLOCK;


  while (num_photons == PHOTONBLOCK) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for(n=0; n < num_photons; n++)
      {
	++ total_read;
	if (file_block[n].T2bits.special == 1) 
	  {
	    if(file_block[n].T2bits.channel==0x3F) // Sync counter overflow
	      {
		overflow_corr += (uint64_t)OLDT2WRAPAROUND;
	      }
	    else if ( file_block[n].T2bits.channel == 0 )
	      ++ *syncs;
	    else
	      ++ *markers;
	  }
	else
	  {
	    realtime = (overflow_corr + file_block[n].T2bits.timetag)*resolution;
	    channel = file_block[n].T2bits.channel;
	    tbWrite(&(tbs[channel].buffer), realtime);
	    for (m=0; m<4; m++){
	      tbPrune(&(tbs[m].buffer), realtime);
	    }

	    for (m=0; m<3; m++) 
	      {
		other_chan = lookup_others[channel][m];
		correlationUpdate(&(corrs[pair_lookup(channel, other_chan)].corr), 
				  &(tbs[other_chan].buffer), 
				  channel, realtime);
	      }
	  }
      }
  }
  free(file_block);
  return(total_read);
}


uint64_t read_ht2_v2(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs, 
			     uint64_t *syncs, uint64_t *markers)
{
  
  tTRec TRec;
  int channel, other_chan;
  uint64_t n, m, num_photons, overflow_corr=0, total_read=0;

  double realtime, resolution=1e-3;

  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = PHOTONBLOCK;
  while (num_photons == PHOTONBLOCK) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for(n=0; n < num_photons; n++)
      {
	++ total_read;
	if (file_block[n].T2bits.special == 1) 
	  {
	    if(file_block[n].T2bits.channel==0x3F) // Sync counter overflow
	      {
		if(file_block[n].T2bits.timetag < 2) // 1 is old format, 1 is new format, both mean 1
		  overflow_corr += (uint64_t)T2WRAPAROUND;
		else
		  {
		    overflow_corr += file_block[n].T2bits.timetag * (uint64_t)T2WRAPAROUND;
		  }
	      }
	    else if ( file_block[n].T2bits.channel == 0 )
	      ++ *syncs;
	    else
	      ++ *markers;
	  }
	else
	  {
	    realtime = (overflow_corr + file_block[n].T2bits.timetag)*resolution;
	    channel = file_block[n].T2bits.channel;
	    tbWrite(&(tbs[channel].buffer), realtime);
	    for (m=0; m<4; m++){
	      tbPrune(&(tbs[m].buffer), realtime);
	    }

	    for (m=0; m<3; m++) 
	      {
		other_chan = lookup_others[channel][m];
		correlationUpdate(&(corrs[pair_lookup(channel, other_chan)].corr), 
				  &(tbs[other_chan].buffer), 
				  channel, realtime);
	      }
	  }
      }
  }
  free(file_block);
  return(total_read);
}

uint64_t read_ht3_v1(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs, 
			     uint64_t *markers)
{

  tTRec TRec;
  int channel, other_chan;
  uint64_t n, m, num_photons, sync_offset=0, total_read=0;

  double realtime;

  double sync_period = (double)1e9 / (double)TTTRHdr.SyncRate;
  double resolution = ((double)BinHdr.Resolution)*((double)1e-3);


  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = (uint64_t)(PHOTONBLOCK);

  while (num_photons == PHOTONBLOCK) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);
    for(n=0; n < num_photons; n++)
      {
	++ total_read;
	if (file_block[n].T3bits.special == 1) 
	  {
	    if(file_block[n].T3bits.channel==0x3F) // Sync counter overflow
	      {
		sync_offset += T3WRAPAROUND;
	      }
	    else
	      ++ *markers;
	  }
	else
	  {
	    realtime = (sync_offset + file_block[n].T3bits.nsync) * sync_period 
	      + file_block[n].T3bits.dtime * resolution;
	    channel = file_block[n].T3bits.channel;
	    tbWrite(&(tbs[channel].buffer), realtime);
	    for (m=0; m<4; m++){
	      tbPrune(&(tbs[m].buffer), realtime);
	    }

	    for (m=0; m<3; m++) 
	      {
		other_chan = lookup_others[channel][m];
		correlationUpdate(&(corrs[pair_lookup(channel, other_chan)].corr), 
				  &(tbs[other_chan].buffer), 
				  channel, realtime);
	      }
	  }
      }
  }
  free(file_block);
  return(total_read);
}

uint64_t read_ht3_v2(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs, 
			     uint64_t *markers)
{

  tTRec TRec;
  int channel, other_chan;
  uint64_t n, m, num_photons, sync_offset=0, total_read=0;

  double realtime;

  double sync_period = (double)1e9 / (double)TTTRHdr.SyncRate;
  double resolution = ((double)BinHdr.Resolution)*((double)1e-3);


  //tTRec *file_block = (tTRec *)calloc(PHOTONBLOCK, sizeof(TRec.allbits));
  tTRec *file_block = (tTRec *)malloc(PHOTONBLOCK * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = (uint64_t)(PHOTONBLOCK);

  while (num_photons == PHOTONBLOCK) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), PHOTONBLOCK, fpin);

    for(n=0; n < num_photons; n++)
      {
	++ total_read;
	if (file_block[n].T3bits.special == 1) 
	  {
	    if(file_block[n].T3bits.channel==0x3F) // Sync counter overflow
	      {
		if(file_block[n].T3bits.nsync < 2)
		  sync_offset += T3WRAPAROUND;
		else
		  sync_offset += file_block[n].T3bits.nsync * T3WRAPAROUND;
	      }
	    else
	      ++ *markers;
	  }
	else
	  {
	    realtime = (sync_offset + file_block[n].T3bits.nsync) * sync_period 
	      + file_block[n].T3bits.dtime * resolution;
	    channel = file_block[n].T3bits.channel;
	    tbWrite(&(tbs[channel].buffer), realtime);
	    for (m=0; m<4; m++){
	      tbPrune(&(tbs[m].buffer), realtime);
	    }

	    for (m=0; m<3; m++) 
	      {
		other_chan = lookup_others[channel][m];
		correlationUpdate(&(corrs[pair_lookup(channel, other_chan)].corr), 
				  &(tbs[other_chan].buffer), 
				  channel, realtime);
	      }
	  }
      }

  }
  free(file_block);
  return(total_read);
}
#endif	/* READ_HT_SEEN */
