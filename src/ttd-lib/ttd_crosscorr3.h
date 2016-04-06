#ifndef _TTD_CROSSCORR3_HEADER
#define _TTD_CROSSCORR3_HEADER

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int64_t num_bins;
  int64_t center_bin;

  int64_t total_coinc;
  uint64_t rbs_counts[3];

  int rbs_allocated[3];
  ttd_rb_t *rbs[3];

  int hist_allocated;
  ttd_t *hist;

} ttd_ccorr3_t;

void ttd_ccorr3_init(ttd_ccorr3_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size);

ttd_ccorr3_t *ttd_ccorr3_build(ttd_t bin_time, ttd_t window_time, size_t rb_size);

void ttd_ccorr3_update(ttd_ccorr3_t *ccorr, int rb_num, ttd_t time);

void ttd_ccorr3_write_csv(ttd_ccorr3_t *ccorr, char *file_name);

void ttd_ccorr3_write_times_csv(ttd_ccorr3_t *ccorr, char *file_name);

void ttd_ccorr3_cleanup(ttd_ccorr3_t *ccorr);

char* append_before_extension(char* to_append, char* old_filename);


#endif // _TTD_CROSSCORR3_HEADER
