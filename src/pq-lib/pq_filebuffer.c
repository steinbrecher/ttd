#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>

#include "pq_filebuffer.h"


int pq_fb_init(pq_fb_t *buffer, char* filename) {
  // Set up default values
  int retcode = 0;

  buffer->buffer_fill = 0;
  buffer->num_read = 0;
  buffer->empty = 0;

  buffer->num_photons = PHOTONBLOCK;
  buffer->total_read = 0;
  buffer->overflow_correction = 0;

  buffer->lastTime = 0;

  // Initialize Ringbuffers
  int i;
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    ttd_rb_init(&(buffer->rbs[i]), PHOTONBLOCK, 0);
  }

  // Initialize Offsets
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    buffer->channel_offsets[i] = 0;
    buffer->channel_active[i] = 1;
  }
  pq_fb_update_active(buffer);

  // Initialize counters
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    buffer->num_read_per_channel[i] = 0;
  }

  // Allocate array for buffering
  buffer->buffered_records = (pq_chanrec_t *)malloc(PHOTONBLOCK * sizeof(pq_chanrec_t));
  if (buffer->buffered_records == NULL) {
    retcode = PQ_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->buffer_allocated = 1;

  // Allocate filename string
  buffer->filename = (char *)malloc((strlen(filename)+1)*sizeof(char));
  if (buffer->filename == NULL) {
    retcode = PQ_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->filename_allocated = 1;
  strcpy(buffer->filename, filename);

  // Open file for reading
  buffer->file_open = 0;
  retcode = pq_fb_openfile(buffer);

  if (retcode < 0) {
    pq_fb_cleanup(buffer);
  }

  // Parse file header
  if (pq_parse_header(buffer->fp, &(buffer->file_info)) < 0) {
    goto error_cleanup;
  }

  // Figure out which conversion function to use
  if (get_pq_converter(&(buffer->to_ttd), &(buffer->file_info) ) < 0) {
    goto error_cleanup;
  }

  // Print file info
  pq_print_file_info(&(buffer->file_info));

  // Disable channels that aren't in use
  for (i=buffer->file_info.num_channels+1; i<PQ_HH_MAX_CHANNELS; i++) {
    pq_fb_disable_channel(buffer, (int16_t)i);
  }

  // Allocate block for photon records
  buffer->file_block_allocated = 0;
  buffer->file_block = (pq_rec_t *) malloc(PHOTONBLOCK*sizeof(pq_rec_t));
  if (buffer->file_block == NULL) {
    printf("ERROR: Could not allocate %d bytes for file buffer\n", PHOTONBLOCK);
    retcode = PQ_FB_MALLOC_ERROR;
  }
  buffer->file_block_allocated = 1;

  // Get first file block
  uint64_t num_photons;
  num_photons = pq_fb_get_block(buffer);
  if (num_photons < PHOTONBLOCK) {
    printf("Only got %"PRIu64 " photon records.\n", num_photons);
    pq_fb_closefile(buffer);
  }
//  _Bool anyEmpty = 0;
  int16_t chan;
  //printf("Num active: %d\n", buffer->num_active_channels);
  for (i=0; i<buffer->num_active_channels; i++) {
    chan = buffer->active_channels[i];

    if (buffer->active_rbs[chan]->count == 0) {
      //printf("WARNING: Channel %d empty to start, disabling\n", chan);
      pq_fb_disable_channel(buffer, chan);
      //anyEmpty = 1;
    }
    //else {
      //printf("Channel %d started with %d counts\n", chan, buffer->active_rbs[chan]->count);
    //}
  }

  return retcode;

  error_cleanup:
  pq_fb_cleanup(buffer);

  return retcode;
}

void pq_fb_update_active(pq_fb_t *buffer) {
  int16_t i, count;
  count = 0;
  for (i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    if (buffer->channel_active[i]) {
      buffer->active_channels[count] = i;
      buffer->active_rbs[count] = &(buffer->rbs[i]);
      count++;
    }
  }
  buffer->num_active_channels = count;
}

void pq_fb_enable_channel(pq_fb_t *buffer, int16_t channel) {
  buffer->channel_active[channel] = 1;
  pq_fb_update_active(buffer);
}

void pq_fb_disable_channel(pq_fb_t *buffer, int16_t channel) {
  buffer->channel_active[channel] = 0;
  pq_fb_update_active(buffer);
}

int pq_fb_openfile(pq_fb_t *buffer) {
  if ((buffer->fp = fopen(buffer->filename, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading\n", buffer->filename);
    return PQ_FB_FILE_OPEN_ERROR;
  }
  buffer->file_open = 1;

  return(0);
}

int pq_fb_closefile(pq_fb_t *buffer) {
  fclose(buffer->fp);
  buffer->file_open = 0;
  return 0;
}

int pq_fb_cleanup(pq_fb_t *buffer){
  int i;
  if (buffer->filename_allocated) {
    free(buffer->filename);
    buffer->filename_allocated = 0;
  }
  if (buffer->file_open) {
    fclose(buffer->fp);
    buffer->file_open = 0;
  }
  if (buffer->buffer_allocated) {
    free(buffer->buffered_records);
    buffer->buffer_allocated = 0;
  }

  if (buffer->file_block_allocated) {
    free(buffer->file_block);
    buffer->file_block_allocated = 0;
  }

  for(i=0; i<PQ_HH_MAX_CHANNELS; i++) {
    ttd_rb_cleanup(&(buffer->rbs[i]));
  }

  return 0;
}

uint64_t pq_fb_get_block(pq_fb_t *buffer) {
  uint64_t num_photons=0;
  int16_t channel;
  ttd_t time;

  size_t n;

  num_photons = fread(buffer->file_block, sizeof(pq_rec_t), PHOTONBLOCK, buffer->fp);


  for (n=0; n<num_photons; n++) {
    channel = buffer->to_ttd(buffer->file_block[n], &time, &buffer->overflow_correction, &buffer->file_info);
    // Make sure not to buffer special records (trying to do so would result in forbidden memory access)
    if (channel < 0) {continue;}
    // If channel isn't active, skip it
    if (!(buffer->channel_active[channel])) {continue;}
    // Otherwise, push it onto the correct ringbuffer, with offset added
    ttd_rb_insert(&(buffer->rbs[channel]), time + buffer->channel_offsets[channel]);
  }
  buffer->total_read += num_photons;
  return num_photons;
}

int pq_fb_get_next(pq_fb_t *buffer, ttd_t *recTime, int16_t *recChannel) {
  if (buffer->empty) {
    return -1;
  }

  int16_t i, firstChannel;
  ttd_rb_t *firstBuffer, *rb;
  ttd_t firstTime, timeHere;

  // Initialize with first active channel
  rb = buffer->active_rbs[0];
  firstChannel = buffer->active_channels[0];
  firstBuffer = buffer->active_rbs[0];
  firstTime = rb->times[rb->start];//ttd_rb_peek(buffer->active_rbs[0]);


  // Loop over other active channels to find smallest time
  for (i=1; i<buffer->num_active_channels; i++) {
    rb = buffer->active_rbs[i];
    if (rb->count == 0) {continue;}
    //printf("Peeking at buffer %d\n", buffer->active_channels[i]);
    timeHere = rb->times[rb->start];
    if (timeHere < firstTime) {
      firstBuffer = rb;
      firstChannel = buffer->active_channels[i];
      firstTime = timeHere;
    }
  }
  if (firstTime < buffer->lastTime) {
    printf("Got out of order time %" PRIu64 " on channel %d\n", firstTime, firstChannel);
  }
  buffer->lastTime = firstTime;

  // Assign values to record
  *recChannel = firstChannel;
  *recTime = firstTime;
  buffer->num_read_per_channel[firstChannel]++;

  // Remove from ringbuffer
  ttd_rb_del(firstBuffer);

  // If the buffer is empty, get more records -- if available.
  // If not available (i.e. we're at end of file), disable the channel.
  if (firstBuffer->count == 0) {
    if (buffer->file_open) {
      if (pq_fb_get_block(buffer) < PHOTONBLOCK) {
        pq_fb_closefile(buffer);
      }
    }
    else {
      pq_fb_disable_channel(buffer, firstChannel);
      if(buffer->num_active_channels == 1) {
        if (buffer->active_rbs[0]->count == 0) {
          pq_fb_disable_channel(buffer, buffer->active_channels[0]);
        }
      }
      if (buffer->num_active_channels == 0) {
        buffer->empty = 1;
      }
    }
  }

  return 0;

}