#ifndef _TTD_CROSSCORR4_HEADER
#define _TTD_CROSSCORR4_HEADER

#include "ttd_ringbuffer.h"

typedef struct {
    ttd_t bin_time;
    ttd_t window_time;

    // Note: this is per-axis, so actual number of bins is cube of num_bins
    size_t num_bins;
    size_t center_bin;

    int64_t total;

    int64_t total_coinc;
    uint64_t rbs_counts[4];

    _Bool rbs_allocated[4];
    ttd_rb_t *rbs[4];

    _Bool hist_allocated;
    ttd_t *hist;
} ttd_ccorr4_t;

void ttd_ccorr4_init(ttd_ccorr4_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size);

ttd_ccorr4_t *ttd_ccorr4_build(ttd_t bin_time, ttd_t window_time, size_t rb_size);

void ttd_ccorr4_update(ttd_ccorr4_t *ccorr, size_t rb_num, ttd_t time);
void ttd_ccorr4_update_no_insert(ttd_ccorr4_t *ccorr, size_t rb_num, ttd_t time);

void ttd_ccorr4_write_csv(ttd_ccorr4_t *ccorr, char *file_name);

//void ttd_ccorr4_write_times_csv(ttd_ccorr4_t *ccorr, char *file_name);

void ttd_ccorr4_cleanup(ttd_ccorr4_t *ccorr);

//char *append_before_extension(char *to_append, char *old_filename);


#endif // _TTD_CROSSCORR4_HEADER
