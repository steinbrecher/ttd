#ifndef _SCITOLL_HEADER_SEEN
#define _SCITOLL_HEADER_SEEN

#define SCITOLL_MULTIPLE_E -1
#define SCITOLL_NO_MANTISSA -2
#define SCITOLL_NO_EXPONENT -3
#define SCITOLL_NEGATIVE -4
#define SCITOLL_DECIMAL -5
#define SCITOLL_STRTOLL_FAILED -6
#define SCITOLL_OVERFLOW -7

    
int64_t sci_to_int64(const char *scinum, int *retcode);
void sci_to_int64_printerr(const char* scinum, int retcode);

#endif // _SCITOLL_HEADER_SEEN
