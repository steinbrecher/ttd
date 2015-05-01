#ifndef _TTD_CROSSCORR2_HEADER
#define _TTD_CROSSCORR2_HEADER

typedef struct {
  // Statistic tracking how many have been seen on each channel
  // Used in normalization code
  uint64_t rbs_counts[2]; 
  // Unused currently
  ttd_t most_common_time;
} ttd_ccorr2_stats_t;

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int64_t num_bins;
  int64_t center_bin;

  int64_t total;

  ttd_ccorr2_stats_t stats;

  int rbs_allocated[2];
  ttd_rb_t *rbs[2];

  int hist_allocated;
  ttd_t *hist;
} ttd_ccorr2_t;

void ttd_ccorr2_init(ttd_ccorr2_t *ccorr, ttd_t bin_time, ttd_t window_time, ttd_rb_t *rb1, ttd_rb_t *rb2);

ttd_ccorr2_t *ttd_ccorr2_build(ttd_t bin_time, ttd_t window_time, int rb_size);

void ttd_ccorr2_update(ttd_ccorr2_t *ccorr, int rb_num, ttd_t time);

void ttd_ccorr2_write_csv(ttd_ccorr2_t *ccorr, char *file_name, int normalize, int int_time);

void ttd_ccorr2_cleanup(ttd_ccorr2_t *ccorr);


#endif // _TTD_CROSSCORR2_HEADER
