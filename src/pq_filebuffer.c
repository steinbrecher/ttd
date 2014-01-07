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

int pq_fb_init(pq_fb_t *buffer, uint64_t buffer_size, char* filename, int64_t offset) {
  int retcode = 0;
  buffer->offset = offset;

  buffer->buffer_size = buffer_size;
  buffer->buffer_fill = 0;

  buffer->num_read = 0;
  buffer->empty = 0;

  buffer->buffer_allocated = 0;
  buffer->buffered_records = (ttd_t *)malloc(buffer->buffer_size * sizeof(ttd_t));
  if (buffer->buffered_records == NULL) {
    retcode = TTD_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->buffer_allocated = 1;


  buffer->filename = (char *)malloc((strlen(filename)+1)*sizeof(char));
  if (buffer->filename == NULL) {
    retcode = TTD_FB_MALLOC_ERROR;
    goto error_cleanup;
  }
  buffer->filename_allocated = 1;
  strcpy(buffer->filename, filename);

  buffer->file_open = 0;
  retcode = ttd_fb_openfile(buffer, buffer->filename);

  if (retcode < 0) {
    ttd_fb_cleanup(buffer);
  }

  return(retcode);

 error_cleanup:
  ttd_fb_cleanup(buffer);
  return(retcode);
}

