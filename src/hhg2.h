#ifndef READ_HT_SEEN
#define READ_HT_SEEN

#include "global_args.h"
#include "hh_header.h"

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

uint64_t read_ht2_v1(FILE *fpin, TimeBufferGroup *tbs, CorrelationGroup *corrs, 
			     uint64_t *syncs, uint64_t *markers)
{
  
  tTRec TRec;
  int channel, other_chan;
  uint64_t n, m, num_photons, overflow_corr=0, total_read=0;

  // In version 1, T2 mode had a 'resolution' of 0.5ps. 
  double realtime, resolution=5e-4;

  //tTRec *file_block = (tTRec *)calloc(global_args.pages*PHOTONS_PER_PAGE, sizeof(TRec.allbits));
  tTRec *file_block = (tTRec *)malloc(global_args.pages*PHOTONS_PER_PAGE*sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = global_args.pages*PHOTONS_PER_PAGE;


  while (num_photons == global_args.pages*PHOTONS_PER_PAGE) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), global_args.pages*PHOTONS_PER_PAGE, fpin);

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

  //tTRec *file_block = (tTRec *)calloc(global_args.pages*PHOTONS_PER_PAGE, sizeof(TRec.allbits));
  tTRec *file_block = (tTRec *)malloc(global_args.pages * PHOTONS_PER_PAGE * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = global_args.pages*PHOTONS_PER_PAGE;
  while (num_photons == global_args.pages*PHOTONS_PER_PAGE) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), global_args.pages*PHOTONS_PER_PAGE, fpin);

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


  tTRec *file_block = (tTRec *)malloc(global_args.pages * PHOTONS_PER_PAGE * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = (uint64_t)(global_args.pages*PHOTONS_PER_PAGE);

  while (num_photons == global_args.pages*PHOTONS_PER_PAGE) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), global_args.pages*PHOTONS_PER_PAGE, fpin);
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


  //tTRec *file_block = (tTRec *)calloc(global_args.pages*PHOTONS_PER_PAGE, sizeof(TRec.allbits));
  tTRec *file_block = (tTRec *)malloc(global_args.pages * PHOTONS_PER_PAGE * sizeof(TRec.allbits));

  static int lookup_others[4][3] = {
    { 1, 2, 3 },
    { 0, 2, 3 },
    { 0, 1, 3 }, 
    { 0, 1, 2 }
  };


  num_photons = (uint64_t)(global_args.pages*PHOTONS_PER_PAGE);

  while (num_photons == global_args.pages*PHOTONS_PER_PAGE) {
    
    // This is much, much faster than reading from file one record at a time
    num_photons = fread(file_block, sizeof(TRec.allbits), global_args.pages*PHOTONS_PER_PAGE, fpin);

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
