#ifndef _TTD_CROSSCORR4_HEADER
#define _TTD_CROSSCORR4_HEADER

typedef struct {
  uint64_t rbs_counts[4];
  ttd_t most_common_time;
} ttd_ccorr4_stats_t;

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int num_bins;
  int center_bin;

  int64_t total;

  ttd_ccorr4_stats_t stats;

  int rbs_allocated[4];
  ttd_rb_t *rbs[4];

  int hist_allocated;
  ttd_t *hist;

} ttd_ccorr4_t;

void ttd_ccorr4_init(ttd_ccorr4_t *ccorr, ttd_t bin_time, ttd_t window_time, 
		     ttd_rb_t *rb1, ttd_rb_t *rb2, ttd_rb_t *rb3, ttd_rb_t *rb4);

ttd_ccorr4_t *ttd_ccorr4_build(ttd_t bin_time, ttd_t window_time, int rb_size);

void ttd_ccorr4_update(ttd_ccorr4_t *ccorr, int rb_num, ttd_t time);

void ttd_ccorr4_write_csv(ttd_ccorr4_t *ccorr, char *file_name);

void ttd_ccorr4_write_times_csv(ttd_ccorr4_t *ccorr, char *file_name);

void ttd_ccorr4_cleanup(ttd_ccorr4_t *ccorr);

char* append_before_extension(char* to_append, char* old_filename);


#endif // _TTD_CROSSCORR3_HEADER
