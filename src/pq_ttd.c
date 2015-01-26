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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ttd.h"
#include "pq_parse.h"
#include "pq_records.h"
#include "pq_ttd_cli.h"
#include "pq_ttd.h"
#include "zlib.h"
#include "ttz.h"

//NOTE: Potential improvement is to pass the channel array structure to the various
// '_to_ttd' functions and have them handle the updates

int pt2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int channel = pq_rec.ph_t2_bits.channel;
  int timetag = pq_rec.ph_t2_bits.timetag;

  if (channel == 0xF) { // Special record
    if ((timetag & 0xF) == 0) { // Overflow Record
      *overflow_correction += PQ_PT2_V2_WRAP;
    }
  }
  else {
    // Resolution is always 4ps here
    *ttd_rec = (*overflow_correction + timetag)*4;
    return(channel);
  }
  return(-1);
}

int pt3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int channel = pq_rec.ph_t3_bits.channel;
  int timetag = pq_rec.ph_t3_bits.timetag;
  int nsync = pq_rec.ph_t3_bits.nsync;
  if (channel == 0xF) { // Overflow
    if (timetag  == 0) { // Overflow Record
      *overflow_correction += PQ_PT3_V2_WRAP;
    }
  }
  else {
    *ttd_rec = (*overflow_correction + nsync)*file_info->sync_period + timetag*file_info->resolution;
    return(channel);
  }
  return(-1);
}


int ht2_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  // Unpack pq_rec
  int special = pq_rec.hh_t2_bits.special;
  int channel = pq_rec.hh_t2_bits.channel;
  int timetag = pq_rec.hh_t2_bits.timetag;
  
  uint64_t realtime;

  if (special == 1) {
    if (channel==0x3F) {
      *overflow_correction += PQ_HT2_V1_WRAP;
    }
    // Sync record
    else if (channel == 0) {
      // Channel indices are 0, 1, ... file_info->num_channels, so this is the 'extra' index for sync
      *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
      return(file_info->num_channels); 
    }
    return(-1);
  }
  else {
    // Resolution of ht2 version 1 was 1/2 picosecond
    *ttd_rec = ttd_rounded_divide((*overflow_correction + timetag), 2);
    return(channel);
  }
}

int ht2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  int special = pq_rec.hh_t2_bits.special;
  int channel = pq_rec.hh_t2_bits.channel;
  int timetag = pq_rec.hh_t2_bits.timetag;

  if (special == 1) {
    if (channel==0x3F) {
      if (timetag < 2) {
	*overflow_correction += PQ_HT2_V2_WRAP;
      }
      else {
	*overflow_correction += PQ_HT2_V2_WRAP * timetag;
      }
    }
    // Sync record
    else if (channel==0) {
      *ttd_rec = *overflow_correction + timetag;
      // Channel indices are 0, 1, ... file_info->num_channels, so this is the 'extra' index for sync
      return(file_info->num_channels); 
    }
  }
  else {
    *ttd_rec = *overflow_correction + timetag;
    return(channel);
  }
  return(-1);
}

int ht3_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  uint64_t sync_period = file_info->sync_period;
  uint64_t resolution = file_info->resolution;
  int special = pq_rec.hh_t3_bits.special;
  int channel = pq_rec.hh_t3_bits.channel;
  int nsync = pq_rec.hh_t3_bits.nsync;
  int dtime = pq_rec.hh_t3_bits.dtime;

  if (special == 1) {
    if (pq_rec.hh_t3_bits.channel==0x3F) {
      *overflow_correction += PQ_HT3_V1_WRAP;
    }
  }
  else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(channel);
  }
  return(-1);
}


int ht3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info) {
  uint64_t sync_period = file_info->sync_period;
  uint64_t resolution = file_info->resolution;
  int special = pq_rec.hh_t3_bits.special;
  int channel = pq_rec.hh_t3_bits.channel;
  int nsync = pq_rec.hh_t3_bits.nsync;
  int dtime = pq_rec.hh_t3_bits.dtime;

  if (special == 1) {
    if (channel==0x3F) {
      if (nsync < 2) {
	*overflow_correction += PQ_HT3_V2_WRAP;
      }
      else {
	*overflow_correction += nsync*PQ_HT3_V2_WRAP;
      }
    }
  }
  else {
    *ttd_rec = (*overflow_correction + nsync) * sync_period + dtime * resolution;
    return(channel);
  }
  return(-1);
}


uint64_t run_hh_convert(FILE *fpin, pq_fileinfo_t *file_info) {
  int channels = file_info->num_channels;
  int instrument = file_info->instrument;
  int meas_mode = file_info->meas_mode;
  int fmt_version = file_info->fmt_version;

  // Output sync records as well in HT2 mode
  // TODO: Make this a command line switch
  int output_sync = 0;
  if ((file_info->instrument == PQ_HH) && (meas_mode == 2)) {
    channels++;
    output_sync = 1;
  }
  int compress = pq_ttd_cli_args.compress;
  int photonblock = PHOTONBLOCK;
  if (compress) {
    photonblock = TTZ_CHUNK/64;
  }
  ttd_t ttd_blocks[channels][photonblock];

  int j, k;
  // Select appropriate version of to_ttd
  int (*to_ttd)(pq_rec_t, ttd_t *, uint64_t *, pq_fileinfo_t *);
  if (instrument == PQ_PH) {
    if (meas_mode == 2) {
      to_ttd = &pt2_v2_to_ttd;
    }
    else if(meas_mode == 3) {
      to_ttd = &pt3_v2_to_ttd;
    }
    else {
      return(-1);
    }
  }
  else if (instrument == (PQ_HH)) {
    if (meas_mode == 2) {
      if (fmt_version == 1) {
	to_ttd = &ht2_v1_to_ttd;
      }
      else if (fmt_version == 2) {
	to_ttd = &ht2_v2_to_ttd;
      }
      else {
	return(-1);
      }
    }
    else if (meas_mode == 3) {
      if (fmt_version == 1) {
	to_ttd = &ht3_v1_to_ttd;
      }
      else if (fmt_version == 2) {
	to_ttd = &ht3_v2_to_ttd;
      }
      else {
	return(-1);
      }
    }
  }
  else {
    return(-1);
  }

  uint64_t num_photons = photonblock;
  uint64_t total_read=0;
  uint64_t overflow_correction = 0;

  ttd_t ttd_record;
  int ttd_buffer_count[channels];

  pq_rec_t *file_block = (pq_rec_t *) malloc(photonblock*sizeof(pq_rec_t));
  int ret, channel, result;

  // Open the output files
  FILE* outfiles[channels];
  char fname[80];
  char suffix[] = "ttd";
  FILE *fifos_in[channels], *fifos_out[channels];
  FILE *lockfile;
  char lockname[] = "ttz_lock1XXXXXX";
  int childnum;

  mode_t fifo_mode = S_IRUSR | S_IWUSR | S_IFIFO;
  int pid=1;
  // Where we initially write the ttd results to
  //FILE** write_to = &outfiles[0];

  if (compress) {
    snprintf(suffix, sizeof(suffix), "ttz");
    // Make a lock file so child process waits until we're done reading the pq files into the fifos
    mkstemp(lockname);
    lockfile = fopen(lockname, "w");
    fwrite(lockname, sizeof(char), sizeof(lockname), lockfile);
    fclose(lockfile);
    // Fork here
    for (j=0; j<channels; j++) {
      if (pid != 0) {
	childnum = j;
	pid = fork();
      }
    }
    if (pid != 0) {
      childnum = -1;
    }
    
    for (k=0; k<channels; k++) {
      if ((pid!=0)||(k==childnum)) {
	snprintf(fname, sizeof(fname), ".fifo_%s-channel-%d.%s", pq_ttd_cli_args.output_prefix, k+1, suffix);
      }
      if (pid != 0) {
	//printf("PARENT: Making FIFO %d\n", k);
	result = mknod(fname, fifo_mode, 0);
	if (result < 0) {
	  perror ("mknod");
	  exit (2);
	}      
	fifos_in[k] = fopen(fname, "w+b");
	SET_BINARY_MODE(fifos_in[childnum]);
      }
    }
    if (pid == 0) {
      //printf("CHILD %d: Waiting for FIFO %s...\n", childnum, fname);
      while (access(fname, F_OK)!=0) {
      }
      fifos_out[childnum] = fopen(fname, "rb");
      SET_BINARY_MODE(fifos_out[childnum]);
    }
  }
  
  // If we are compressing, child process should open the output files
  if (!(compress)) {
    for (k=0; k<channels; k++) {
      snprintf(fname, sizeof(fname), "%s-channel-%d.%s", pq_ttd_cli_args.output_prefix, k+1, suffix);
      outfiles[k] = fopen(fname, "wb");
    }
    // If we're outputting sync, change name of the last file
    if (output_sync) {
      snprintf(fname, sizeof(fname), "%s-channel-sync.%s", pq_ttd_cli_args.output_prefix, suffix);
      fclose(outfiles[channels-1]);
      outfiles[channels-1] = fopen(fname, "wb");
    }
  }
  else if (pid==0) {
    if ((output_sync)&&(childnum == channels-1)) {
      snprintf(fname, sizeof(fname), "%s-channel-sync.%s", pq_ttd_cli_args.output_prefix, suffix);
      outfiles[childnum] = fopen(fname, "wb");
      //printf("CHILD %d: Opening %s for writing\n", childnum, fname);
    }
    else {
      snprintf(fname, sizeof(fname), "%s-channel-%d.%s", pq_ttd_cli_args.output_prefix, childnum+1, suffix);
      //printf("CHILD %d: Opening %s for writing\n", childnum, fname);
      outfiles[childnum] = fopen(fname, "wb");
      //printf("CHILD %d File Status: %d\n", childnum, ferror(outfiles[childnum]));
    }
  }

  uint64_t n;
  //if (pid!=0) {
  //printf("PARENT: Reading ttd files...\n");
  //}
  // If we're compressing, child process should *not* run this loop
  while ((num_photons == photonblock)&&(pid!=0)) {
    // Read file block
    num_photons = fread(file_block, sizeof(pq_rec_t), photonblock, fpin);

    // Set buffer counters to 0
    for (k=0; k<channels; k++) {
      ttd_buffer_count[k] = 0;
    }

    // Read data into per-channel buffers
    for (n=0; n < num_photons; n++) {
      total_read++;
      channel = to_ttd(file_block[n], &ttd_record, &overflow_correction, file_info);
      if (channel != -1) {
	ttd_blocks[channel][ttd_buffer_count[channel]] = ttd_record;
	++ ttd_buffer_count[channel];
      }
    }
    
    // Write data to outfiles
    // fwrite(data, sizeof(element), sizeof(array), file);
    if (compress) {
      for (k=0; k<channels; k++) {
	//printf("PARENT: Writing to FIFO %d...\n", k);
	fwrite(&(ttd_blocks[k][j]), sizeof(ttd_record), ttd_buffer_count[k], fifos_in[k]);
      }
    }
    else {
      for (k=0; k<channels; k++) {
	fwrite(&(ttd_blocks[k][j]), sizeof(ttd_record), ttd_buffer_count[k], outfiles[k]);
      }
    }
  }
  // Delete Lock file
  if ((compress) && (pid != 0)) {
    //printf("Unlocked!\n");
    remove(lockname);
  }
  z_stream stream;
  int avail=0, locked=0, flush;
  unsigned char in[TTZ_CHUNK];
  
  if (pid == 0) {
    //printf("CHILD %d: Beginning compression...\n", childnum);
    def_init(1, &stream);
    while (locked==0) {
      while ((avail==0)&&(locked==0)) {
	avail = fread(in, 1, TTZ_CHUNK, fifos_out[childnum]);
	locked = access(lockname, F_OK);
      }
      stream.avail_in = avail;
      stream.next_in = in;
      locked = access(lockname, F_OK);
      flush = (locked==0) ? Z_NO_FLUSH : Z_FINISH;
      if (def_update(&stream, outfiles[childnum], flush) != Z_OK) {
	exit(-1);
      }
      avail = fread(in, 1, TTZ_CHUNK, fifos_out[childnum]);
    }
    avail = fread(in, 1, TTZ_CHUNK, fifos_out[childnum]);
    stream.avail_in = avail;
    stream.next_in = in;
    locked = access(lockname, F_OK);
    flush = (locked==0) ? Z_NO_FLUSH : Z_FINISH;
    if (def_update(&stream, outfiles[childnum], flush) != Z_OK) {
      exit(-1);
    }
    def_cleanup(&stream);
  }

  // Close the output files
  if (!(compress)) {
    for (k=0; k < channels; k++) {
      fclose(outfiles[k]);
    }
    free(&file_block);
  }
  else if (pid == 0) {
    //printf("CHILD %d: Closing output file\n", childnum);
    fclose(fifos_out[childnum]);
    fclose(outfiles[childnum]);
  }

  // Kill child process here
  if (pid == 0) {
    exit(0);
  }
  else if (compress) {
    for (k=0; k<channels; k++) {
      fclose(fifos_in[k]);
      snprintf(fname, sizeof(fname), ".fifo_%s-channel-%d.%s", pq_ttd_cli_args.output_prefix, k+1, suffix);
      remove(fname);
      free(&file_block);
    }
  }
  printf("Records Read: %" PRIu64 "\n", total_read);
  if (total_read != file_info->num_records) {
    printf("\nWARNING: Did not reach end of file.\n");
  }

  return(total_read);
}

char *get_prefix(char* filename) {
  int i, dot_index = -1;
  char *prefix;
  for (i=0; i < strlen(filename); i++) {
    if (filename[i] == '.') {
      dot_index = i;
    }
  }
  if (dot_index == -1) {
    prefix = (char *)malloc((strlen(filename)+1)*sizeof(char));
    strcpy(prefix, filename);
    return(prefix);
  }
  prefix = (char *)malloc((dot_index+2)*sizeof(char));
  for (i=0; i<dot_index; i++) {
    prefix[i] = filename[i];
  }
  prefix[dot_index] = '\0';
  return prefix;
}

int main(int argc, char* argv[]) {
  FILE *ht_file;
  int retcode,exitcode=0;
  int prefix_allocated=0;
  char *output_prefix;

  retcode = pq_ttd_read_cli(argc, argv);
  if (retcode < 0) {
    exitcode = retcode;
    goto cleanup_pq_ttd_cli;
  }
  else if (retcode == PQ_TTD_CLI_EXIT_RETCODE) {
    goto cleanup_pq_ttd_cli;
  }

  if (pq_ttd_cli_args.infile == NULL) {
    printf("Error: Please supply input file with '-i [infile]'\n");
    goto cleanup_pq_ttd_cli;
  }

  if (pq_ttd_cli_args.output_prefix == NULL) {
    pq_ttd_cli_args.output_prefix = get_prefix(pq_ttd_cli_args.infile);
    pq_ttd_cli_args.output_prefix_allocated = 1;
  }

  printf("Output Prefix: %s\n", pq_ttd_cli_args.output_prefix);

  ht_file = fopen(pq_ttd_cli_args.infile, "rb");

  pq_fileinfo_t file_info;
  retcode = pq_parse_header(ht_file, &file_info);
  if (retcode < 0) {
    goto clean_file;
  }
  pq_printf_file_info(&file_info);

  // Benchmarking timers
  clock_t start, diff; 		

  printf("\n");
  start = clock();
  run_hh_convert(ht_file, &file_info);
  diff = clock() - start;

  double read_time = (double)diff / CLOCKS_PER_SEC;
  printf("Elapsed Time: %g seconds\n", read_time);
  
 clean_file:
  fclose(ht_file);

 cleanup_pq_ttd_cli:
  pq_ttd_cli_cleanup();
  exit(exitcode);
}
