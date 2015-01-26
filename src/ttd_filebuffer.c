#ifndef __STDC_FORMAT_MACROS
#define __STDC_FORMAT_MACROS
#endif
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "ttd.h"
#include "ttd_filebuffer.h"

#include "zlib.h"
#include "ttz.h"

char *get_extension(char* filename) {
  int i, dot_index = -1;
  char *extension = (char *)malloc(4*sizeof(char));

  for (i=0; i<3; i++) {
    extension[i] = filename[strlen(filename)-3+i];
  }
  extension[3] = '\0';
  return extension;
}

int ttd_fb_openfile(ttd_fb_t *buffer, char* filename) {
  FILE *fifo, *datafile;
  char fifo_name[] = "ttz_fifoXXXXXX";
  mode_t fifo_mode = S_IRUSR | S_IWUSR | S_IFIFO;
  int pid, result, fd;
  if (buffer->compressed) {
    fd = mkstemp(fifo_name); 
    close(fd);
    remove(fifo_name);
    result = mknod(fifo_name, fifo_mode, 0);
    if (result < 0) {
      perror ("mknod");
      exit (2);
    }
    pid = fork();
    // Set up the parent
    if (pid!=0) {
      buffer->fp = fopen(fifo_name, "rb");
      buffer->file_open = 1;
      return(0);
    }
    // Set up decompression child
    else {
      fifo = fopen(fifo_name, "wb");
      if ((datafile = fopen(buffer->filename, "rb")) == NULL) {
	printf("ERROR: Could not open %s for reading\n", buffer->filename);
	return TTD_FB_FILE_OPEN_ERROR;
      }
      inf(datafile, fifo);
      exit(0);
    }
  }
  else{
    if ((buffer->fp = fopen(buffer->filename, "rb")) == NULL) {
      printf("ERROR: Could not open %s for reading\n", buffer->filename);
      return TTD_FB_FILE_OPEN_ERROR;
    }
    buffer->file_open = 1;
  }
  return(0);
}

int ttd_fb_init(ttd_fb_t *buffer, uint64_t buffer_size, char* filename, int64_t offset) {
  int retcode = 0;
  char *extension = get_extension(filename);
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
  if (strcmp(extension, "ttz")==0) {
    buffer->compressed = 1;
  }
  else {
    buffer->compressed = 0;
  }
  buffer->file_open = 0;
  retcode = ttd_fb_openfile(buffer, buffer->filename);

  if (retcode < 0) {
    ttd_fb_cleanup(buffer);
  }
  
  return(retcode);

 error_cleanup:
  free(extension);
  ttd_fb_cleanup(buffer);
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
    buffer->buffer_allocated = 0;
  }
  return(0);
}

ttd_t ttd_fb_pop(ttd_fb_t *buffer) {
  ttd_t event_time;
  if (buffer->empty == 1) {
    fprintf(stderr, "Error: ttd_fb_pop called on empty buffer. Zero returned; bug present in calling code.\n");
    return(0);
  }
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
