%module stomp
%include std_string.i
%{
#include "stomp.h"
%}
%feature("kwargs");

// must you declare with throw (const char *)?
%typemap(throws) const char * %{
    PyErr_SetString(PyExc_RuntimeError, $1);
    SWIG_fail;
%}


%include "stomp.h"

