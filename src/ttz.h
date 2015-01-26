#ifndef _TTZ_HEADER
#define _TTZ_HEADER

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#  include <fcntl.h>
#  include <io.h>
#  define SET_BINARY_MODE(file) setmode(fileno(file), O_BINARY)
#else
#  define SET_BINARY_MODE(file)
#endif

//int def(FILE *source, FILE *dest, int level);
int def_init(int level, z_stream *strm);

int def_update(z_stream* strm, FILE *dest, int flush);

int def_cleanup(z_stream *strm);

int inf(FILE *source, FILE *dest);
void zerr(int ret);
#define TTZ_CHUNK 65536


#endif
