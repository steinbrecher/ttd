#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "ttp_cli.h"
#include "ttd.h"
#include "ttd_filebuffer.h"

int ttd_fb_openfile(ttd_fb_t *buffer) {
  if ((buffer->fp = fopen(buffer->filename, "rb")) == NULL) {
    printf("ERROR: Could not open %s for reading\n", buffer->filename);
    return(-1);
  }
  buffer->file_open = 1;
  if (ttp_cli_args.verbose == 1) {
    printf("ttb_fb_openfile: Opened %s for reading\n", buffer->filename);
  }
  return(0);
}

int ttd_fb_init(ttd_fb_t *buffer, uint64_t buffer_size, char* filename, int64_t offset) {
  int retcode = 0;
  buffer->offset = offset;

  buffer->buffer_size = buffer_size;
  buffer->buffer_fill = 0;

  buffer->num_read = 0;
  buffer->empty = 0;

  buffer->buffer_allocated = 0;
  buffer->buffered_records = (ttd_t *)malloc(buffer->buffer_size * sizeof(ttd_t));
  buffer->buffer_allocated = 1;


  buffer->filename = (char *)malloc((strlen(filename)+1)*sizeof(char));
  buffer->filename_allocated = 1;
  strcpy(buffer->filename, filename);

  buffer->file_open = 0;
  retcode = ttd_fb_openfile(buffer);

  if (retcode < 0) {
    ttd_fb_cleanup(buffer);
  }

  return(retcode);
}

int ttd_fb_cleanup(ttd_fb_t *buffer) {
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
  }
  return(0);
}

ttd_t ttd_fb_pop(ttd_fb_t *buffer) {
  ttd_t event_time;
  if (buffer->num_read < buffer->buffer_fill) {
    event_time = buffer->buffered_records[buffer->num_read] + buffer->offset;
    buffer->num_read ++;
  }

  else if (buffer->file_open == 1) {
    buffer->buffer_fill = fread(buffer->buffered_records, sizeof(ttd_t), buffer->buffer_size, buffer->fp);
    event_time = buffer->buffered_records[0] + buffer->offset;
    buffer->num_read = 1;
    if ((buffer->buffer_fill < buffer->buffer_size) && buffer->file_open) {
      fclose(buffer->fp);
      buffer->file_open = 0;
    }
  }

  if ((buffer->num_read == buffer->buffer_fill) && (buffer->file_open == 0)) {
    buffer->empty = 1;
  }
  return(event_time);
}
