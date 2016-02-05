#ifndef _SCITOLL_HEADER_SEEN
#define _SCITOLL_HEADER_SEEN

#define SCITOLL_MULTIPLE_E -1
#define SCITOLL_NO_MANTISSA -2
#define SCITOLL_NO_EXPONENT -3
#define SCITOLL_NEGATIVE -4
#define SCITOLL_DECIMAL -5
    
long long scitoll(char* scinum, int* retcode);

#endif // _SCITOLL_HEADER_SEEN
