#ifndef _tester_h
#define _tester_h

#include <Python.h>

// doesn't seem to work to include it here with swig...
//#include "../NumpyVector.h"

class tester {
	public:
		tester() throw (const char *);
		~tester() {};

		void dotest_creation() throw (const char *);
        double test_sum(PyObject* obj) throw (const char* );
        double test_sum_iterator(PyObject* obj) throw (const char* );
        double test_sum_ptr(PyObject* obj) throw (const char* );

        void dotest_fromobj(PyObject* obj) throw (const char *);

        PyObject* dotest_output() throw (const char* );

        void testrec(PyObject* obj) throw (const char* );
};

#endif
