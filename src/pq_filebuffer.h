#ifndef _PQ_FILEBUFFER_HEADER_SEEN
#define _PQ_FILEBUFFER_HEADER_SEEN

#include "ttd.h"
#include "pq_parse.h"
#include "pq_ttd.h"
#include "pq_records.h"

typedef struct {
    int16_t channel;
    ttd_t record;
} pq_chanrec_t;

typedef struct {
    uint64_t buffer_fill;
    uint64_t num_read;
    int empty;

    pq_chanrec_t *buffered_records;
    int buffer_allocated;
    int buffered_records_count;
    int buffered_records_idx;

    char *filename;
    int filename_allocated;

    int file_open;
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
    int file_block_allocated;
} pq_fb_t;

int pq_fb_init(pq_fb_t *buffer, char* filename);
int pq_fb_openfile(pq_fb_t *buffer);
int pq_fb_cleanup(pq_fb_t *buffer);
int pq_fb_pop(pq_fb_t *buffer, ttd_t *time, int16_t *channel);
uint64_t pq_fb_get_block(pq_fb_t *buffer);
int pq_fb_closefile(pq_fb_t *buffer);

#define PQ_FB_MALLOC_ERROR -1;
#define PQ_FB_FILE_OPEN_ERROR -2;
#define PQ_RB_EMPTY_ERROR -3;

#endif //_PQ_FILEBUFFER_HEADER_SEEN
