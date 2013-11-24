#ifndef _TTD_CROSSCORR_HEADER
#define _TTD_CROSSCORR_HEADER

typedef struct {
  uint64_t rbs_counts[2];
  ttd_t most_common_time;
} ttd_ccorr_stats_t;

typedef struct {
  ttd_t bin_time;
  ttd_t window_time;

  int num_bins;
  int center_bin;

  int64_t total;

  ttd_ccorr_stats_t stats;

  int rbs_allocated[2];
  ttd_rb_t *rbs[2];

  int hist_allocated;
  ttd_t *hist;
} ttd_ccorr_t;

void ttd_ccorr_init(ttd_ccorr_t *ccor, ttd_rb_t *rb1, ttd_rb_t *rb2);

ttd_ccorr_t *ttd_ccorr_build(int rb_size, ttd_t rb_duration);

void ttd_ccorr_update(ttd_ccorr_t *ccorr, int rb_num, ttd_t time);

void ttd_ccorr_write_csv(ttd_ccorr_t *ccorr, char *file_name);

void ttd_ccorr_cleanup(ttd_ccorr_t *ccorr);


#endif // _TTD_CROSSCORR_HEADER
