#include "NumpyVector.h"

NumpyVector::NumpyVector()  throw (const char *) {
	// DONT FORGET THIS!!!!
	import_array();

	// don't forget to initialize
	mArray = NULL;
	mSize=0;
	mNdim=-1;
	mDimsPtr=NULL;
	mTypeNum=-1;
}



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



T* NumpyVector::get(npy_intp index) throw (const char *) {
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

// get from 2-d array.  The user is responsible for making sure
// this makes sense.
void* NumpyVector::get(npy_intp i1, npy_intp i2) throw (const char *) {
	if (mArray == NULL) {
		throw "Error: attempt to get pointer from an uninitialized array";
	}

	if (mNdim != 2) {
		std::stringstream err;
		err <<"Error: Indexing array of dimension "<<mNdim<<" with "
			<<"two subscripts";
		throw err.str().c_str();
	}

	return PyArray_GETPTR2((PyArrayObject*) mArray, i1, i2);
}

// get from 3-d array.  The user is responsible for making sure
// this makes sense.
void* NumpyVector::get(npy_intp i1, npy_intp i2, npy_intp i3) throw (const char *) {
	if (mArray == NULL) {
		throw "Error: attempt to get pointer from an uninitialized array";
	}

	if (mNdim != 3) {
		std::stringstream err;
		err <<"Error: Indexing array of dimension "<<mNdim<<" with "
			<<"three subscripts";
		throw err.str().c_str();
	}


	return PyArray_GETPTR3((PyArrayObject*) mArray, i1, i2, i3);
}

// get from 4-d array.  The user is responsible for making sure
// this makes sense.
void* NumpyVector::get(
		npy_intp i1, 
		npy_intp i2, 
		npy_intp i3, 
		npy_intp i4) throw (const char *) {
	if (mArray == NULL) {
		throw "Error: attempt to get pointer from an uninitialized array";
	}

	if (mNdim != 4) {
		std::stringstream err;
		err <<"Error: Indexing array of dimension "<<mNdim<<" with "
			<<"four subscripts";
		throw err.str().c_str();
	}


	return PyArray_GETPTR4((PyArrayObject*) mArray, i1, i2, i3, i4);
}



