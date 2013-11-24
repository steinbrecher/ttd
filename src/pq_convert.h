#ifndef PQ_CONVERT_HEADER_SEEN
#define PQ_CONVERT_HEADER_SEEN

#define PHOTONBLOCK 32768
#define HT2WRAPAROUND 33554432 // 2^25
#define OLDHT2WRAPAROUND 33552000 // 2^25 - 2,432
#define HT3WRAPAROUND 1024 // 2^10

int ht2_v1_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, uint64_t *overflow_correction);

int ht2_v2_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, uint64_t *overflow_correction);

int ht3_v1_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, uint64_t *overflow_correction);

int ht3_v2_to_ttd(pq_hh_rec_t TRec, ttd_t *ttd_rec, uint64_t *overflow_correction);

uint64_t run_hh_convert(FILE *fpin);

#endif // HT_CONVERT_SEEN
