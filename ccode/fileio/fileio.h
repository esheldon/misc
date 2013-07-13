#ifndef _FILEIO_HEADER_GUARD
#define _FILEIO_HEADER_GUARD

FILE *fileio_open_stream(const char *name, const char *mode);
FILE *fileio_open_or_die(const char *name, const char *mode);

long fileio_count_lines(FILE *stream);
// last char is returned
int fileio_skip_line(FILE *stream);
// last char is returned
int fileio_skip_lines(FILE *stream, long nlines);

#endif
