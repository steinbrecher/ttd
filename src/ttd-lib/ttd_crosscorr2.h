#ifndef _TTD_CROSSCORR2_HEADER
#define _TTD_CROSSCORR2_HEADER

#include "ttd.h"
#include "ttd_ringbuffer.h"

typedef struct {
    ttd_t bin_time;
    ttd_t window_time;

    size_t num_bins;
    size_t center_bin;

    int64_t total_coinc;
    uint64_t rbs_counts[2];

    _Bool rbs_allocated[2];
    ttd_rb_t *rbs[2];

    _Bool hist_allocated;
    ttd_t *hist;
} ttd_ccorr2_t;

void ttd_ccorr2_init(ttd_ccorr2_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size);

ttd_ccorr2_t *ttd_ccorr2_build(ttd_t bin_time, ttd_t window_time, size_t rb_size);

void ttd_ccorr2_update(ttd_ccorr2_t *ccorr, size_t rb_num, ttd_t time);

void ttd_ccorr2_write_csv(ttd_ccorr2_t *ccorr, char *file_name, int normalize, ttd_t int_time, ttd_t write_window);

void ttd_ccorr2_cleanup(ttd_ccorr2_t *ccorr);


#endif // _TTD_CROSSCORR2_HEADER
