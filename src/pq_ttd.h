#ifndef _PQ_CONVERT_HEADER_SEEN
#define _PQ_CONVERT_HEADER_SEEN

#define PHOTONBLOCK 32768

int pt2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info);
int pt3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, ttd_t *overflow_correction, pq_fileinfo_t *file_info);
int ht2_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, uint64_t *overflow_correction, pq_fileinfo_t *file_info);
int ht2_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, uint64_t *overflow_correction, pq_fileinfo_t *file_info);
int ht3_v1_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, uint64_t *overflow_correction, pq_fileinfo_t *file_info);
int ht3_v2_to_ttd(pq_rec_t pq_rec, ttd_t *ttd_rec, uint64_t *overflow_correction, pq_fileinfo_t *file_info);

int read_header(FILE *fpin);

void write_convert_propeties();

uint64_t run_hh_convert(FILE *fpin, pq_fileinfo_t *file_info);

#endif // HT_CONVERT_SEEN
