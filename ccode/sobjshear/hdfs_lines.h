#include <stdlib.h>

#include <hdfsJniHelper.h>
#include <hdfs.h>

#ifndef _HDFS_LINES
#define _HDFS_LINES


ssize_t hdfs_getline(hdfsFile f, char **lineptr, size_t *length);

// these are just simple helper functions with error handling
hdfsFS hdfs_connect(void);
hdfsFile hdfs_open(hdfsFS fs, const char* url, int mode, int buffsize);

int is_in_hdfs(const char* name);

#endif
