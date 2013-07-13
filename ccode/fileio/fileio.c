#include <stdlib.h>
#include <stdio.h>

FILE *fileio_open_stream(const char *name, const char *mode)
{
    FILE *fobj=fopen(name,mode);
    if (!fobj) {
        fprintf(stderr,"error opening file with mode '%s': %s\n", mode, name);
    }
    return fobj;
}
FILE *fileio_open_or_die(const char *name, const char *mode)
{
    FILE *stream = fileio_open_stream(name,mode);
    if (!stream) {
        fprintf(stderr,"aborting\n");
        exit(1);
    }
    return stream;
}


long fileio_count_lines(FILE *stream)
{
    long nlines=0;
    int c=0;

    do {
        c = fgetc(stream);
        if (c == '\n') {
            nlines++;
        }
    } while (c != EOF);

    return nlines;
}

int fileio_skip_line(FILE *stream) {
    int c=0;
    do {
        c=fgetc(stream);
        if (c=='\n') {
            break;
        }
    } while (c != EOF);
    return c;
}
int fileio_skip_lines(FILE *stream, long nlines) {
    int c=0;
    long nfound=0;
    do {
        c=fgetc(stream);
        if (c=='\n') {
            nfound++;
        }
    } while (c != EOF && nfound < nlines);
    return c;
}
