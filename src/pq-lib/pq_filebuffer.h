#ifndef _PQ_FILEBUFFER_HEADER_SEEN
#define _PQ_FILEBUFFER_HEADER_SEEN

#include "ttd.h"
#include "pq_parse.h"
#include "pq_ttd.h"
#include "pq_records.h"
#include "ttd_ringbuffer.h"

typedef struct {
    int16_t channel;
    ttd_t record;
} pq_chanrec_t;

typedef struct {
    uint64_t buffer_fill;
    uint64_t num_read;
    _Bool empty;

    pq_chanrec_t *buffered_records;
    size_t buffered_records_count;
    size_t buffered_records_idx;

    char *filename;

    _Bool file_open;
    FILE *fp;

    ////  Special pieces for the PicoQuant file buffer ////
    // PicoQuant-specific file information
    pq_fileinfo_t file_info;

    // Record conversion function (specialized per filetype)
    pq_to_ttd_t to_ttd;

    // Statistics
    uint64_t num_photons;
    uint64_t total_read;
    uint64_t overflow_correction;

    // File block
    pq_rec_t *file_block;

    // Channel offset processing
    ttd_t channel_offsets[PQ_HH_MAX_CHANNELS];
    _Bool channel_active[PQ_HH_MAX_CHANNELS];
    size_t active_channels[PQ_HH_MAX_CHANNELS];
    ttd_rb_t *active_rbs[PQ_HH_MAX_CHANNELS];
    size_t num_active_channels;
    size_t num_read_per_channel[PQ_HH_MAX_CHANNELS];

    // Ring buffers for offsets
    ttd_rb_t rbs[PQ_HH_MAX_CHANNELS];

    ttd_t lastTime;


} pq_fb_t;

int pq_fb_init(pq_fb_t *buffer, char *filename);

int pq_fb_openfile(pq_fb_t *buffer);

int pq_fb_cleanup(pq_fb_t *buffer);

int pq_fb_pop(pq_fb_t *buffer, ttd_t *time, size_t *channel);

int pq_fb_get_next(pq_fb_t *buffer, ttd_t *recTime, size_t *recChannel);

uint64_t pq_fb_get_block(pq_fb_t *buffer);

int pq_fb_closefile(pq_fb_t *buffer);

void pq_fb_update_active(pq_fb_t *buffer);

void pq_fb_enable_channel(pq_fb_t *buffer, size_t channel);

void pq_fb_disable_channel(pq_fb_t *buffer, size_t channel);

#define PQ_FB_MALLOC_ERROR -1;
#define PQ_FB_FILE_OPEN_ERROR -2;
#define PQ_RB_EMPTY_ERROR -3;

#endif //_PQ_FILEBUFFER_HEADER_SEEN
