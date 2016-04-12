#ifndef _TTD_FILEBUFFER_HEADER_SEEN
#define _TTD_FILEBUFFER_HEADER_SEEN
typedef struct {
  int64_t offset;
  
  uint64_t buffer_size;
  uint64_t buffer_fill;
  uint64_t num_read;
  _Bool empty;

  ttd_t *buffered_records;

  char *filename;

  _Bool file_open;
  FILE *fp;
} ttd_fb_t;

int ttd_fb_init(ttd_fb_t *buffer, uint64_t buffer_size, char* filename, int64_t offset);
int ttd_fb_openfile(ttd_fb_t *buffer);
int ttd_fb_cleanup(ttd_fb_t *buffer);
ttd_t ttd_fb_pop(ttd_fb_t *buffer);

#define TTD_FB_MALLOC_ERROR -1;
#define TTD_FB_FILE_OPEN_ERROR -2;
char *get_extension(char* filename);

#endif // _TTD_FILEBUFFER_HEADER_SEEN

