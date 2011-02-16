#ifndef _stomp_map_h
#define _stomp_map_h

#include <Python.h>
#include <iostream>
#include <sstream>
#include <string>
#include "numpy/arrayobject.h"
#include "stomp_util.h"

// These are "yes" flags.  E.g., it *is* inside the map, and the second
// quadrant around this object *is* contained in the map.

#define INSIDE_MAP 1
#define FIRST_QUADRANT_OK 2
#define SECOND_QUADRANT_OK 4
#define THIRD_QUADRANT_OK 8
#define FOURTH_QUADRANT_OK 16

/*
 * this is sort of wrapper class for Stomp subclasses.  It will provide an
 * interfact between the data structures used in Stomp and numpy arrays
 */

class StompMap {
	public:
		// system can be "eq" or "sdss".  This will be the default
		// coordinates returned or assumed, but can always be 
		// over-ridden in the method calls.
		StompMap(
				std::string map_file, 
				std::string system="eq") throw (const char *);


		void set_system(std::string system) throw (const char *);

		double area()  throw (const char *) { 
			return mMap.Area();
		}
		std::string system() throw (const char *) {
			return mSystem;
		}

		// Generate random points within map.  Supports equatorial and sdss
		// coordinates (csurvey)
		PyObject* genrand(
				unsigned long long numrand, 
				std::string system="") throw (const char *);

		// Check if points are contained within the map.  If radius is sent,
		// also check if quadrants of the associated disk are contained.

		PyObject* contains(
				PyObject* coord1, 
				PyObject* coord2, 
				PyObject* radius=NULL,
				std::string system="") throw (const char *);


		PyObject* TestNumpyVector(PyObject* obj) throw (const char*);

		~StompMap() {};
	
	private:

		// we don't want SWIG to try to wrap this
		int wedge_contained(
				Stomp::AngularCoordinate& ang, 
				double radius,
				double position_angle_min, 
				double position_angle_max,
				Stomp::AngularCoordinate::Sphere coord_system);


		PyObject* Object2Array(
				PyObject* obj, 
				int typenum)  throw (const char *);


		Stomp::AngularCoordinate::Sphere
			get_system_id(std::string& system) throw (const char*);

		void cleanup3(PyObject* obj1, PyObject* obj2, PyObject* obj3);

		std::string mSystem;
		Stomp::AngularCoordinate::Sphere mSystemId;

		std::string mMapFile;
		Stomp::Map mMap;

		uint16_t mMaxResolution;

};

#endif
