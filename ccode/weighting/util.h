#ifndef _WEIGHTS_UTIL_H
#define _WEIGHTS_UTIL_H
#include <cstdio>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>

using namespace std;

template<typename T> T converter(const char *str) {
    stringstream ss;
    T val;

    ss<<str;
    ss>>val;
    return val;
}


bool file_readable(string strFilename);
void ensure_file_readable(string file) throw (ios_base::failure);
size_t nlines(string filename);

void throw_runtime(string arg1, string arg2="", string arg3="", string arg4="");
void throw_fileio(string arg1, string arg2="", string arg3="", string arg4="");

#endif
