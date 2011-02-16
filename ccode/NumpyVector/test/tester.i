%module tester
//%include std_string.i
%{
#include "tester.h"
%}
//%feature("kwargs");

// must you declare with throw (const char *)?
%typemap(throws) const char * %{
    PyErr_SetString(PyExc_RuntimeError, $1);
    SWIG_fail;
%}


%include "tester.h"


