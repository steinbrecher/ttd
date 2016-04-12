#ifndef _TTD_CROSSCORR2_HEADER
#define _TTD_CROSSCORR2_HEADER

#include "ttd.h"
#include "ttd_ringbuffer.h"

/** \brief Structure for coordinating second order cross-correlation calculation
 *
 * ttd_ccorr2_t stores the necessary information to calculate G(2) cross-correlations.
 * Functions are provided for initialization, updating (adding a new time record), writing
 * the result to a CSV file, and cleaning up memory allocations. Using the member parameter
 * `window_time`, the update function handles buffering of previously seen time records.
 *
 * The output format is a histogram, with binned coincidences (ttd_ccorr2_t#bin_time sets the size of these),
 * which is written to a csv file, and can be normalized for g(2) correlations.
 * */
typedef struct {
    ttd_t bin_time;
    /**< Bin width of histogram */
    ttd_t window_time;
    /**< The time centers of the histogram bins range over {`-window_time, -window_time + bin_time, ...,
    * -2 * bin_time, -bin_time, 0, bin_time, 2 * bin_time, ..., window_time - bin_time , window_time`} */

    size_t num_bins;
    /**< Number of elements in the `hist` array */
    size_t center_bin;
    /**< Precomputed center bin (bin with center at zero time delay) */

    int64_t total_coinc;
    /**< For normalization: Track total number of coincidences seen  */
    uint64_t rbs_counts[2];
    /**< For normalization: Track total number of counts seen on each channel */

    ttd_rb_t *rbs[2];
    /**< Pointers to ttd_rb_t ringbuffer structures; all differential times calculated as
     * (time in rbs[1]) - (time in rbs[0]) */

    // TODO: Look into changing datatype of this
    ttd_t *hist;
    /**< calloc'd array used for storing histogram counts */
} ttd_ccorr2_t;

void ttd_ccorr2_init(ttd_ccorr2_t *ccorr, ttd_t bin_time, ttd_t window_time, size_t rb_size);

ttd_ccorr2_t *ttd_ccorr2_build(ttd_t bin_time, ttd_t window_time, size_t rb_size);

void ttd_ccorr2_update(ttd_ccorr2_t *ccorr, size_t rb_num, ttd_t time);

void ttd_ccorr2_update_no_insert(ttd_ccorr2_t *ccorr, size_t rb_num, ttd_t time);

void ttd_ccorr2_write_csv(ttd_ccorr2_t *ccorr, char *file_name, int normalize, ttd_t int_time, ttd_t write_window);

void ttd_ccorr2_cleanup(ttd_ccorr2_t *ccorr);


#endif // _TTD_CROSSCORR2_HEADER
