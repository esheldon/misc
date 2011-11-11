/*
 * This could be sped up using memchr and memcpy, but presumably the
 * bottlneck is reading from hdfs in the first place.
 */
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include "hdfs_lines.h"

struct hdfs_lines* hdfs_lines_new(hdfsFS fs, hdfsFile file, size_t buffsize) {
    struct hdfs_lines* l=NULL;

    if (buffsize <= 0) {
        fprintf(stderr,"buffer size must be > 0");
        exit(EXIT_FAILURE);
    }

    l = calloc(1, sizeof(struct hdfs_lines));
    if (l == NULL) {
        fprintf(stderr,"Failed to allocate line buffer");
        exit(EXIT_FAILURE);
    }


    l->buff=calloc(buffsize+1, sizeof(char));
    l->line=calloc(buffsize+1, sizeof(char));

    if (l->buff == NULL || l->line == NULL) {
        fprintf(stderr,"Failed to allocate line buffer");
        exit(EXIT_FAILURE);
    }

    l->buffsize=buffsize;
    l->totsize=buffsize;
    l->size=0;

    // means read next time we enter next()
    l->curpos=buffsize;

    l->fs = fs;
    l->file = file;

    return l;
}

// usage:  hl = hdfs_lines_delete(hl);
struct hdfs_lines* hdfs_lines_delete(struct hdfs_lines* hl) {
    if (hl != NULL) {
        free(hl->buff);
        free(hl->line);
    }
    free(hl);
    return NULL;
}


size_t hdfs_lines_read(struct hdfs_lines* hl) {
    size_t nread=0;

    nread=hdfsRead(hl->fs, hl->file, hl->buff, hl->buffsize);
    hl->curpos=0;
    hl->nread=nread;
    return nread;
}

char hdfs_lines_nextchar(struct hdfs_lines* hl) {
    size_t nread=0;
    char c='\0';

    if (hl->curpos == hl->buffsize) {
        nread = hdfs_lines_read(hl);
        if (nread == 0) {
            return c;
        }
    }
    c = hl->buff[hl->curpos];
    hl->curpos++;

    return c;
}

void hdfs_lines_realloc(struct hdfs_lines* hl) {
    size_t newsize=0;

    // realloc line to twice the size
    newsize = 2*hl->totsize;
    hl->line = realloc(hl->line, newsize);
    if (hl->line == NULL) {
        fprintf(stderr,"failed to realloce line to size %ld\n", newsize);
        exit(EXIT_FAILURE);
    }
    hl->totsize=newsize;
}
void hdfs_lines_pushchar(struct hdfs_lines* hl, char c) {
    if (hl->size == hl->totsize) {
        // make twice as big
        hdfs_lines_realloc(hl);
    }
    hl->line[hl->size] = c;
    // if we push a null char, size doesn't change
    if (c != '\0') {
        hl->size++;
    }
}

void hdfs_lines_clear(struct hdfs_lines* hl) {
    hl->size=0;
    hl->line[0] = '\0';
}

/* get characters until we hit a newline or we run out of data
 * data are read into the buffer in buffsize chunks as needed
 */
size_t hdfs_lines_next(struct hdfs_lines* hl) {
    char c='\0';
    
    hdfs_lines_clear(hl);

    while (c != '\n') {
        c = hdfs_lines_nextchar(hl);
        hdfs_lines_pushchar(hl, c);
        if (c == '\0') {
            return hl->size;
        }
    }
    hdfs_lines_pushchar(hl, '\0');

    return hl->size;
}



// convenience functions with error handling
hdfsFS hdfs_connect(void) {
    hdfsFS fs = hdfsConnect("default", 0);
    if (!fs) {
        fprintf(stderr, "Failed to connect to hdfs\n");
        exit(EXIT_FAILURE);
    } 
    return fs;
}
hdfsFile hdfs_open(hdfsFS fs, const char* url, int mode, int buffsize) {
    hdfsFile hdfs_file = hdfsOpenFile(fs, url, mode, buffsize, 0, 0);
    if (!hdfs_file) {
        fprintf(stderr, "Failed to open hdfs url %s\n", url);
        exit(EXIT_FAILURE);
    }
    return hdfs_file;
}


