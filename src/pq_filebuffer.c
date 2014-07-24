#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttd.h"
#include "pq_parse.h"
#include "pq_filebuffer.h"

int pq_fb_openfile(pq_fb_t *buffer, char* filename) {
  if ((buffer->fp = fopen(buffer->filename, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading\n", buffer->filename);
    return PQ_FB_FILE_OPEN_ERROR;
  }
  buffer->file_open = 1;

  return(0);
}

int pq_fb_cleanup(pq_fb_t *buffer) {
  if (buffer->filename_allocated) {
    free(buffer->filename);
    buffer->filename_allocated = 0;
  }

  if (buffer->file_open) {
    fclose(buffer->fp);
    buffer->file_open = 0;
  }

  if (buffer->buffer_allocated == 1) {
    free(buffer->buffered_records);
    buffer->buffer_allocated = 0;
  }
  return(0);
}

void pq_fb_set_to_ttd(pq_fb_t *buffer) {
  pq_fileinfo_t fileinfo = buffer->fileinfo;
  if (file_info.instrument == PQ_PH) {
    if (file_info.meas_mode == 2) {
      buffer->to_ttd = &pt2_v2_to_ttd;
    }
    else if(file_info.meas_mode == 3) {
      buffer->to_ttd = &pt3_v2_to_ttd;
    }
    else {
      return(-1);
    }
  }
  else if (file_info.instrument == PQ_HH) {
    if (file_info.meas_mode == 2) {
      if (file_info.fmt_version == 1) {
	buffer->to_ttd = &ht2_v1_to_ttd;
      }
      else if (file_info.fmt_version == 2) {
	buffer->to_ttd = &ht2_v2_to_ttd;
      }
      else {
	return(-1);
      }
    }
    else if (file_info.meas_mode == 3) {
      if (file_info.fmt_version == 1) {
	buffer->to_ttd = &ht3_v1_to_ttd;
      }
      else if (file_info.fmt_version == 2) {
	buffer->to_ttd = &ht3_v2_to_ttd;
      }
      else {
	return(-1);
      }
    }
  }
  else return(-1);
}

int pq_fb_init(pq_fb_t *buffer, uint64_t buffer_size, char* filename) {
  // Initialize values in the buffer struct
  int retcode = 0;
  buffer->offset = offset;

  buffer->buffer_size = buffer_size;
  buffer->buffer_fill = 0;

  buffer->num_read = 0;
  buffer->empty = 0;

  // Allocate the buffer itself
  buffer->buffer_allocated = 0;
  buffer->buffered_records = (ttd_t *)malloc(buffer->buffer_size * sizeof(ttd_t));
  if (buffer->buffered_records == NULL) {
    retcode = PQ_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->buffer_allocated = 1;

  // Allocate input filename string
  buffer->filename = (char *)malloc((strlen(filename)+1)*sizeof(char));
  if (buffer->filename == NULL) {
    retcode = PQ_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->filename_allocated = 1;
  strcpy(buffer->filename, filename);

  // Open the input file
  buffer->file_open = 0;
  retcode = pq_fb_openfile(buffer, buffer->filename);
  if (retcode < 0) 
    goto error_cleanup;

  // Parse the header
  retcode = pq_parse_header(buffer->fp, buffer->fileinfo); 
  if (retcode < 0) 
    goto error_cleanup;
  
  // Assign correct to_ttd function
  pq_fb_set_to_ttd(buffer);

  if (retcode == 0) 
    return(0);

 error_cleanup:
  pq_fb_cleanup(buffer);
  return(retcode);
}

