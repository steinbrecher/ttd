#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
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
  pq_printf_file_info(&(buffer->file_info));

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

  return retcode;

  error_cleanup:
  pq_fb_cleanup(buffer);

  return retcode;

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

  return 0;
}

uint64_t pq_fb_get_block(pq_fb_t *buffer) {
  uint64_t num_photons=0;
  int16_t channel;
  ttd_t time;

  size_t n;

  num_photons = fread(buffer->file_block, sizeof(pq_rec_t), PHOTONBLOCK, buffer->fp);


  for (n=0; n<num_photons; n++) {
    buffer->buffered_records[n].channel =
            buffer->to_ttd(buffer->file_block[n], &(buffer->buffered_records[n].record),
                           &buffer->overflow_correction, &buffer->file_info);
  }
  buffer->total_read += num_photons;
  buffer->buffered_records_count = num_photons;
  buffer->buffered_records_idx = 0;
  return num_photons;
}

int pq_fb_pop(pq_fb_t *buffer, ttd_t *time, int16_t *channel) {
  if (buffer->empty) {
    return -1;
  }

  *channel = buffer->buffered_records[buffer->buffered_records_idx].channel;
  *time = buffer->buffered_records[buffer->buffered_records_idx].record;
  buffer->buffered_records_idx++;

  // If this was the last record in the buffer...
  if (buffer->buffered_records_idx == buffer->buffered_records_count) {
    // and the file is still open...
    if (buffer->file_open) {
      // try to get another block.
      // If fewer than PHOTONBLOCK records read, file is done

      if (pq_fb_get_block(buffer) < PHOTONBLOCK) {
        pq_fb_closefile(buffer);
      }
    }
    else {
      // If file isn't open, we're done
      buffer->empty = 1;
    }
  }
  return 0;
}