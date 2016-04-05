//
// Created by Greg Steinbrecher on 3/6/16.
//

#ifndef PICOQUANT_TTD_CROSSCORR_H
#define PICOQUANT_TTD_CROSSCORR_H

typedef struct {
    ttd_t *hist;
    ttd_t **rbs;

    _Bool hist_allocated;
    _Bool rbs_allocated;

    int16_t n; // Order of the cross-correlation
    int64_t total; // Total photons seen
    size_t num_bins; // Number of bins
    ttd_t bin_time;
    ttd_t window_time;

} ttd_ccorr_t;

void ttd_ccorr_init(ttd_ccorr_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size)

#endif //PICOQUANT_TTD_CROSSCORR_H
