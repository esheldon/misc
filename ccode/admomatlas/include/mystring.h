#if !defined(_mystring_h)
#define _mystring_h

#include <string.h>
#include <iostream.h>
#include <stdlib.h>
const int max_len=255;

class string {
 public:                       // universal access
  string(int n) { s = new char[n+1]; len=n; } // make constructors
  string(const char* p) {
    len = strlen(p);
    s = new char[len+1];
    strcpy(s, p);
  }
  string() { len = max_len; s = new char[max_len]; } //default constructor;
  void assign(const char * st) { strcpy(s, st); len = strlen(st); }
  int length() { return len; }
  void print() { cout << s << "\n"; }
  int i() { return atoi(s); }
  char* s;      //implementation by character array
  friend string operator+(const string& a, const string& b);
  
 private:         // restricted access
  int len;
};

string operator+(const string& a, const string& b);

#endif
