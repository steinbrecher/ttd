#ifndef _TTD_CROSSCORR3_HEADER
#define _TTD_CROSSCORR3_HEADER

typedef struct {
  uint64_t rbs_counts[3];
  ttd_t most_common_time;
} ttd_ccorr3_stats_t;

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int num_bins;
  int center_bin;

  int64_t total;

  ttd_ccorr3_stats_t stats;

  int rbs_allocated[3];
  ttd_rb_t *rbs[3];

  int hist_allocated;
  ttd_t *hist;

} ttd_ccorr3_t;

void ttd_ccorr3_init(ttd_ccorr3_t *ccorr, ttd_t bin_time, ttd_t window_time, ttd_rb_t *rb1, ttd_rb_t *rb2, ttd_rb_t *rb3);

ttd_ccorr3_t *ttd_ccorr3_build(ttd_t bin_time, ttd_t window_time, int rb_size);

void ttd_ccorr3_update(ttd_ccorr3_t *ccorr, int rb_num, ttd_t time);

void ttd_ccorr3_write_csv(ttd_ccorr3_t *ccorr, char *file_name);

void ttd_ccorr3_write_times_csv(ttd_ccorr3_t *ccorr, char *file_name);

void ttd_ccorr3_cleanup(ttd_ccorr3_t *ccorr);

char* append_before_extension(char* to_append, char* old_filename);


#endif // _TTD_CROSSCORR3_HEADER
