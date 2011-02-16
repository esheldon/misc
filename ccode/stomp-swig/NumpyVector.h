#ifndef _numpy_vector_h
#define _numpy_vector_h

#include <Python.h>
#include <iostream>
#include <sstream>
#include "TypeInfo"
#include "numpy/arrayobject.h"




/*
 * this is simple wrapper class for 1-d and scalar numpy arrays.  Might be able
 * to easily expand this to n-d
 */

template <class T> class NumpyVector {
	public:

		NumpyVector() throw (const char *);

		// Create from existing python object
		//NumpyVector(PyObject* obj, int typenum) throw (const char *);

		// Create given the length and typenum
		//NumpyVector(npy_intp size, int typenum) throw (const char *);



		// Conver the input obj to an array of the right type and with
		// dimensions 0 or 1 and correct type. Also, must be the native
		// endianness.  If already the right type, etc. then no copy is made.
		//void setfromobj(PyObject* obj, int typenum)  throw (const char *);

		// Create from scratch based on size and typenum
		//void setfromtypesize(npy_intp size, int typenum)  throw (const char *);



		// Get a reference the underlying object and  incref the object.  This
		// is useful if you want to get a PyObject* that will be returned to
		// the outside world. The internal version will be decrefed when
		// the object is destructed or goes out of scope.

		/*
		PyObject* getref() throw (const char *);


		T* ptr() throw (const char *);

		T* ptr(npy_intp index) throw (const char *);
		*/


		std::string type_name() {
			return mTypeName;
		}
		npy_intp size() {
			return mSize;
		}
		npy_intp ndim() {
			return mNdim;
		}

		~NumpyVector() {
			Py_XDECREF(mArray);
		};
	
	private:
		int mTypeNum;
		npy_intp mSize;
		npy_intp mNdim;
		PyObject* mArray;

		// this will not copy
		npy_intp* mDimsPtr;

		std::string mTypeName;

		TypeInfo mTypeInfo;
};

template <class T>
NumpyVector<T>::NumpyVector()  throw (const char *) {
	// DONT FORGET THIS!!!!
	import_array();

	mTypeInfo.init();

	// don't forget to initialize
	mArray = NULL;
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;

	mTypeName = typeid(T).name();
	SetTypeId();
}

template <class T>
void NumpyVector<T>::SetTypeId() {
	if (mTypeName == "d") {
		mTypeNum = NPY_FLOAT64;
	} 
}

/*
NumpyVector::NumpyVector(PyObject* obj, int typenum)  throw (const char *) {
	// DONT FORGET THIS!!!!
	import_array();

	// don't forget to initialize
	mArray = NULL;
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;

	// Get the data.  This may or may not make a copy.
	setfromobj(obj, typenum);
}


// Create given the length and typenum
NumpyVector::NumpyVector(npy_intp size, int typenum) throw (const char *) {
	// DONT FORGET THIS!!!!
	import_array();

	// don't forget to initialize
	mArray = NULL;
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;

	setfromtypesize(size, typenum);
}

void NumpyVector::setfromobj(
		PyObject* obj, int typenum)  throw (const char *) {

	// clear any existing array
	Py_XDECREF(mArray);
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;


	if (obj == NULL || obj == Py_None) {
		throw "cannot convert the input object to an array: is NULL or None";
	}

	// set the type num
	mTypeNum = typenum;


	// can be scalar, but not higher dimensional than 4
	int min_depth=0, max_depth=4;

	// require the array is in native byte order
	int requirements = NPY_NOTSWAPPED;


	PyArray_Descr* descr=NULL;
	descr = PyArray_DescrNewFromType(typenum);

	if (descr == NULL) {
		throw "could not create array descriptor";
	}
	// This will steal a reference to descr, so we don't need to decref
	// descr as long as we decref the array!
	mArray = PyArray_CheckFromAny(
			obj, descr, min_depth, max_depth, requirements, NULL);

	if (mArray == NULL) {
		// this causes a segfault, don't do it
		//Py_XDECREF(descr);
		throw "Could not get input as array";
	}

	mSize = PyArray_SIZE(mArray);
	mNdim = PyArray_NDIM(mArray);
	mDimsPtr = PyArray_DIMS(mArray);
}


void NumpyVector::setfromtypesize(
		npy_intp size, int typenum)  throw (const char *) {

	// clear any existing array
	Py_XDECREF(mArray);
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;

	if (size < 1)  {
		throw "size must be >= 1";
	}

	// set the type num
	mTypeNum = typenum;

	// Create output flags array
	int ndim=1;
	mArray = PyArray_ZEROS(
			ndim, 
			&size,
			typenum,
			NPY_FALSE);

	if (mArray ==NULL) {
		throw "Could not allocate array";
	}

	mSize = PyArray_SIZE(mArray);
	mNdim = PyArray_NDIM(mArray);
	mDimsPtr = PyArray_DIMS(mArray);
}


// Get a reference the object.  incref the object.
// This is useful if you want to get a PyObject* that will be returned
// to the outside world
PyObject* NumpyVector::getref() throw (const char *) {
	Py_XINCREF(mArray);
	return mArray;
}


<template T> T* NumpyVector::ptr() throw (const char *) {
	if (mArray == NULL) {
		throw "Error: attempt to get pointer from an uninitialized array";
	}

	npy_intp index=0;
	return (T* ) PyArray_GetPtr((PyArrayObject*) mArray, &index);
}



T* NumpyVector::ptr(npy_intp index) throw (const char *) {
	if (mArray == NULL) {
		throw "Error: attempt to get pointer from an uninitialized array";
	}

	if (mNdim > 1) {
		std::stringstream err;
		err <<"Error: Indexing array of dimension "<<mNdim<<" with "
			<<"a single subscript";
		throw err.str().c_str();
	}

	return PyArray_GetPtr((PyArrayObject*) mArray, &index);
}

*/






#endif
