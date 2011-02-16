#include "stomp.h"
#include "NumpyVector.h"

StompMap::StompMap(
		std::string map_file,
		std::string system) throw (const char *) {
	// DONT FORGET THIS!!!!
	import_array();

	mMapFile = map_file;

	set_system(system);

	// For now, not hpixel (false) but *with* weight in third column, always
	// 1.0 for these old maps
	mMap.Read(map_file, false, true);

   mMaxResolution = mMap.MaxResolution();
}

void StompMap::set_system(std::string system) throw (const char *) {
	mSystem = system;
	mSystemId = get_system_id(system);
}

PyObject* StompMap::genrand(
		unsigned long long numrand, 
		std::string system_input) throw (const char *) {


	Stomp::AngularCoordinate::Sphere system = get_system_id(system_input);

	// The python objects to hold the output data
	PyArrayObject* coord1_array;
	PyArrayObject* coord2_array;

	// We'll return a tuple (coord1,coord2)
	PyObject* output_tuple = PyTuple_New(2);

	// Create the output arrays
	int ndim=1;
	npy_intp intpsize = numrand;
	double *coord1_ptr;
	double *coord2_ptr;

	coord1_array = (PyArrayObject* ) PyArray_ZEROS(
			ndim, 
			&intpsize,
			NPY_DOUBLE,
			NPY_FALSE);
	if (coord1_array ==NULL) {
		throw "Could not allocate output array";
	}
	
	coord2_array = (PyArrayObject* ) PyArray_ZEROS(
			ndim, 
			&intpsize,
			NPY_DOUBLE,
			NPY_FALSE);
	if (coord2_array ==NULL) {
		throw "Could not allocate output array";
	}

	// Get pointers for copying in data
	coord1_ptr = (double* ) PyArray_DATA(coord1_array);
	coord2_ptr = (double* ) PyArray_DATA(coord2_array);

	Stomp::AngularVector ang;

	mMap.GenerateRandomPoints(ang, numrand);

	for (unsigned long i=0; i<ang.size(); i++) {
		if (system == Stomp::AngularCoordinate::Survey) {
		   *coord1_ptr = ang[i].Lambda();
		   *coord2_ptr = ang[i].Eta();
		} else {
		   *coord1_ptr = ang[i].RA();
		   *coord2_ptr = ang[i].DEC();
		}

		++coord1_ptr;
		++coord2_ptr;
	}

	// steals a reference
	PyTuple_SetItem(output_tuple, 0, (PyObject* ) coord1_array);
	PyTuple_SetItem(output_tuple, 1, (PyObject* ) coord2_array);

	return (PyObject* ) output_tuple;

}

int StompMap::wedge_contained(
		Stomp::AngularCoordinate& ang, 
		double radius,
		double position_angle_min, 
		double position_angle_max,
		Stomp::AngularCoordinate::Sphere coord_system) {

	// Now we assemble a Map for the wedge and see if it is inside our
	// reference Map. Note using weight of 1.0
	
	std::cout<<"making wedge_bound in system: "<<coord_system<<"\n";
	std::cout<<"   min pos angle: "<<position_angle_min<<"\n";
	std::cout<<"   max pos angle: "<<position_angle_max<<"\n";
	std::cout<<"   radius: "<<radius<<"\n";
	Stomp::WedgeBound wedge_bound(
			ang, 
			radius, 
			position_angle_min, 
			position_angle_max, 
			1.0, 
			coord_system);

	std::cout<<"setting max res: "<<mMaxResolution<<"\n";
	wedge_bound.SetMaxResolution(mMaxResolution);

	int res=0;
	std::cout<<"pixelizing\n";
	if (wedge_bound.Pixelize()) {
		std::cout<<"OK, making wedge_map\n";
		Stomp::Map* wedge_map = wedge_bound.ExportMap();

		std::cout<<"OK, seeing if contained...";
		if (mMap.Contains(*wedge_map)) {
			std::cout<<"yes\n";
			res=1;
		} else {
			std::cout<<"no\n";
		}

		delete wedge_map;
	}

	return res;

}


PyObject* StompMap::contains(
		PyObject* coord1_obj, 
		PyObject* coord2_obj, 
		PyObject* radius_obj, // optional
		std::string system_input) /* optional */ throw (const char *)   {

	// TODO:  type checking and byte order checking
	
	Stomp::AngularCoordinate::Sphere system = get_system_id(system_input);

	// output flags array
	PyArrayObject* flags_array=NULL;

	// might hold radius if input
	PyObject* radius_array=NULL;
	double *radius=NULL, *coord1=NULL, *coord2=NULL;
	npy_intp nrad=0;

	// get double arrays for coords.  This may or may not make a copy.
	// We MUST decref!
	PyObject* coord1_array = Object2Array(coord1_obj,NPY_DOUBLE);
	PyObject* coord2_array = Object2Array(coord2_obj,NPY_DOUBLE);

	// some basic setup
	npy_intp num1 = PyArray_Size(coord1_array);
	npy_intp num2 = PyArray_Size(coord2_array);


	std::cout<<"num1: "<<num1<<"\n";
	std::cout<<"num2: "<<num2<<"\n";
	if (num1 != num2 || num1 == 0) {
		cleanup3(coord1_array,coord2_array,radius_array);
		throw "input coordinate arrays must be same length and > 0";
	}

	if (radius_obj != NULL) {
		// Must decref!!
		radius_array = Object2Array(radius_obj,NPY_DOUBLE);
		nrad = PyArray_Size(radius_array);

		if (num1 != nrad && nrad != 1) {
			cleanup3(coord1_array,coord2_array,radius_array);
			throw "input radius must be scalar or same length as coords";
		}
	}


	// Create output flags array
	int ndim=1;
	flags_array = (PyArrayObject* ) PyArray_ZEROS(
			ndim, 
			&num1,
			NPY_INTP,
			NPY_FALSE);
	if (flags_array ==NULL) {
		cleanup3(coord1_array,coord2_array,radius_array);
		throw "Could not allocate output flags array";
	}

	// for a scalar, we might as well get the value now
	if (nrad == 1) {
		npy_intp zero=0;
		radius = 
			(double *) PyArray_GetPtr((PyArrayObject*) radius_array, &zero);
	}


	for (npy_intp i=0; i< num1; i++) {
		
		std::cout<<"hello0\n";

		// this takes care of the strides
		coord1 = 
			(double *) PyArray_GetPtr((PyArrayObject*) coord1_array, &i);
		coord2 = 
			(double *) PyArray_GetPtr((PyArrayObject*) coord2_array, &i);

		std::cout<<"hello1\n";

		// this is zero at construction above
		npy_intp* flags = 
			(npy_intp* ) PyArray_GETPTR1(flags_array,i);

		std::cout<<"i="<<i<<": "<<*coord1<<", "<<*coord2<<"\n";
		Stomp::AngularCoordinate ang(*coord1, *coord2, system);

		// Check if the point is contained within the map
		if (mMap.FindLocation(ang)) {
			*flags += INSIDE_MAP;
		}

		// If the radius was sent, check quadrants
		if ( (*flags > 0) && (nrad > 0)) {

			// we have already got radius if it was a scalar, otherwise 
			// this will get us the data and take care of the strides.
			if (nrad == num1) {
				radius = 
					(double *) PyArray_GetPtr((PyArrayObject*) radius_array, &i);
			}

			std::cout<<"radius = "<<*radius<<"\n";

			// check first quadrant
			if ( wedge_contained(ang, *radius, 0.0, 90.0, system) ) {
				*flags += FIRST_QUADRANT_OK; 
			}
			// second
			if ( wedge_contained(ang, *radius, 90.0, 180.0, system) ) {
				*flags += SECOND_QUADRANT_OK; 
			}
			// third
			if ( wedge_contained(ang, *radius, 180.0, 270.0, system) ) {
				*flags += THIRD_QUADRANT_OK; 
			}
			// fourth
			if ( wedge_contained(ang, *radius, 270.0, 360.0, system) ) {
				*flags += FOURTH_QUADRANT_OK; 
			}

		} // radius sent and central point was contained
	} // loop over elements

	// We must decref the views into the arrays
	cleanup3(coord1_array,coord2_array,radius_array);

	return (PyObject* ) flags_array;


}


// This will make a copy of the right type if the requirements are not met or
// the data type is not the one requested.  Otherwise it is just a view on the
// underlying data.
//
// You must decref the array no matter what. Use Py_XDECREF in case it is NULL
// In order to work with both scalars and arrays, you should use something like
// this to get data
//
//		ptr = (double *) PyArray_GetPtr((PyArrayObject*) array, &i);
//
//	This is less convenient that PyArray_GETPTR1 and those kinds, but it works
//	for scalars if i=0.
//

PyObject* StompMap::Object2Array( PyObject* obj, int typenum)  throw (const
char *) {

	int min_depth=0, max_depth=1;
	// require the array is in native byte order
	int requirements = NPY_NOTSWAPPED;

	PyObject* arr=NULL;

	if (obj == NULL || obj == Py_None) {
		return NULL;
	}

	PyArray_Descr* descr=NULL;
	descr = PyArray_DescrNewFromType(typenum);

	if (descr == NULL) {
		throw "could not create array descriptor";
	}
	// This will steal a reference to descr, so we don't need to decref
	// descr as long as we decref the array!
	arr = PyArray_CheckFromAny(
			obj, descr, min_depth, max_depth, requirements, NULL);
	if (arr == NULL) {
		// this causes a segfault, don't do it
		//Py_XDECREF(descr);
		throw "Could not get input as array";
	}
	return arr;
}


void StompMap::cleanup3(
		PyObject* obj1, PyObject* obj2, PyObject* obj3) {

	Py_XDECREF(obj1);
	Py_XDECREF(obj2);
	Py_XDECREF(obj3);
}


Stomp::AngularCoordinate::Sphere
StompMap::get_system_id(std::string& system) throw (const char*) {

	std::stringstream err;

	Stomp::AngularCoordinate::Sphere sys;

	if (system == "eq") {
		sys = Stomp::AngularCoordinate::Equatorial;
	} else if (system == "sdss") {
		sys = Stomp::AngularCoordinate::Survey;
	} else if (system == "") {
		// use default
		sys = mSystemId;
	} else {
		err<<"bad coord system indicator '"<<system<<"'";
		throw err.str().c_str();
	}

	return sys;
}


PyObject* StompMap::TestNumpyVector(PyObject* obj) throw (const char*) {

	NumpyVector<double> vec;

	std::cout<<"type name: "<<vec.type_name()<<"\n";

	PyObject* tmp=Py_None;
	return tmp;
	/*
	NumpyVector vec(obj, NPY_DOUBLE);
	npy_intp size=vec.size();
	std::cout<<"size: "<<size<<"\n";
	std::cout<<"ndim: "<<vec.ndim()<<"\n";

	// output to copy into
	NumpyVector newvec(2*size, NPY_INT32);
	npy_int32* val_int32 = (npy_int32* )newvec.get();

	double* val;

	val = (double *) vec.get(1000,2000);
	if (vec.ndim() == 1) {
		for (npy_intp i=0; i<size; i++) {
			val = (double *) vec.get(i);
			std::cout<<"vec["<<i<<"] = "<<*val<<"\n";


			// convert
			*val_int32 = (npy_int32) (*val);
			++val_int32;

		}
	} else if (vec.ndim() == 2) {
		npy_intp* dims=PyArray_DIMS(obj);
		for (npy_intp i=0; i<dims[0]; i++) {
			for (npy_intp j=0; j<dims[1]; j++) {
				val = (double *) vec.get(i,j);
				std::cout<<"vec["<<i<<"]["<<j<<"] = "<<*val<<"\n";


				// convert
				*val_int32 = (npy_int32) (*val);
				++val_int32;
			}
		}
	}




	// This will increment the references internally.  The
	// object decrefs once when deconstructed.
	PyObject* outarr = newvec.getref();
	return outarr;
	*/
}
