#include "util.h"
#include <sys/stat.h> 
#include <sstream>
#include <stdexcept>

bool file_readable(string strFilename) {
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  // Attempt to get the file attributes
  intStat = stat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0) {
    // We were able to get the file attributes
    // so the file obviously exists.
    blnReturn = true;
  } else {
    // We were not able to get the file attributes.
    // This may mean that we don't have permission to
    // access the folder which contains this file. If you
    // need to do that level of checking, lookup the
    // return values of stat which will give you
    // more details on why stat failed.
    blnReturn = false;
  }
  
  return(blnReturn);
}

void ensure_file_readable(string file) throw (ios_base::failure) {
    if (!file_readable(file)) {
        throw_fileio("File not found:",file);
    }
}

size_t nlines(string filename) {
    ensure_file_readable(filename);
    ifstream file(filename.c_str());

    size_t count=0;
    string line;
    while( getline( file, line ) ) {
        count ++; 
    }
    file.close();
    return count;
}

void throw_runtime(string arg1, string arg2, string arg3, string arg4) {
    flush(cout);
    stringstream ss;
    ss<<arg1;
    if (arg2 != "") {
        ss<<" "<<arg2;
    }
    if (arg3 != "") {
        ss<<" "<<arg3;
    }
    if (arg4 != "") {
        ss<<" "<<arg4;
    }

    throw runtime_error(ss.str());
}
void throw_fileio(string arg1, string arg2, string arg3, string arg4) {
    flush(cout);
    stringstream ss;

    ss<<arg1;
    if (arg2 != "") {
        ss<<" "<<arg2;
    }
    if (arg3 != "") {
        ss<<" "<<arg3;
    }
    if (arg4 != "") {
        ss<<" "<<arg4;
    }

    throw ios_base::failure(ss.str());
}

