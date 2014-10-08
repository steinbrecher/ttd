#ifndef PQ_RECORDS_HEADER_SEEN
#define PQ_RECORDS_HEADER_SEEN

#define PQ_HT2_V1_WRAP (uint64_t)33552000
#define PQ_HT3_V1_WRAP (uint64_t)1024 // 2^10

#define PQ_HT2_V2_WRAP (uint64_t)33554432 // 2^25
#define PQ_HT3_V2_WRAP (uint64_t)1024

#define PQ_PT2_V2_WRAP (uint64_t)210698240
#define PQ_PT3_V2_WRAP (uint64_t)65536

typedef union { 
  uint32_t allbits;
  struct {
    unsigned timetag :28;
    unsigned channel :4;
  } ph_t2_bits; 
  struct {
    unsigned nsync :16;
    unsigned timetag :12;
    unsigned channel :4;
  } ph_t3_bits;
  struct {
    unsigned timetag  :25;
    unsigned channel  :6;
    unsigned special  :1; // or sync, if channel==0
  } hh_t2_bits; 
  struct {
    unsigned nsync		:10; 	// numer of sync period
    unsigned dtime		:15;    // delay from last sync in units of chosen resolution 
    unsigned channel	:6;
    unsigned special	:1;
  } hh_t3_bits;
} pq_rec_t;


#endif // PQ_RECORDS_HEADER_SEEN
