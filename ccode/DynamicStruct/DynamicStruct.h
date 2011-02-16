#ifndef _DYNAMIC_STRUCT_H
#define _DYNAMIC_STRUCT_H

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;

#include <iomanip>
using std::setw;

#include <sstream>
using std::ostringstream;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include <map>
using std::map;
using std::make_pair;
#include <stdexcept>
#include <typeinfo>

#include <stdint.h>

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned long long longlong;
typedef unsigned long long ulonglong;


typedef int8_t int8;
typedef uint8_t uint8;
typedef int16_t int16;
typedef uint16_t uint16;
typedef int32_t int32;
typedef uint32_t uint32;
typedef int64_t int64;
typedef uint64_t uint64;
typedef float float32;
typedef double float64;

namespace std {

	class TypeInfo {
		public:
			TypeInfo() {
				init();
			}
			void init() {
				// static class members, only create once and shared by all
				// instances
				if (name.empty()) {

					const char* tname;


					tname = typeid(char).name();
					id[tname] = 0;
					name["char"] = tname;

					tname = typeid(uchar).name();
					id[tname] = 1;
					name["uchar"] = tname;

					tname = typeid(short).name();
					id[tname] = 2;
					name["short"] = tname;

					tname = typeid(ushort).name();
					id[tname] = 3;
					name["ushort"] = tname;

					tname = typeid(int).name();
					id[tname] = 4;
					name["int"] = tname;

					tname = typeid(uint).name();
					id[tname] = 5;
					name["uint"] = tname;

					tname = typeid(long).name();
					id[tname] = 6;
					name["long"] = tname;

					tname = typeid(ulong).name();
					id[tname] = 7;
					name["ulong"] = tname;

					tname = typeid(long long).name();
					id[tname] = 8;
					name["longlong"] = tname;

					tname = typeid(ulonglong).name();
					id[tname] = 9;
					name["ulonglong"] = tname;

					tname = typeid(float).name();
					id[tname] = 10;
					name["float"] = tname;

					tname = typeid(double).name();
					id[tname] = 11;
					name["double"] = tname;


					tname = typeid(string).name();
					id[tname] = 12;
					name["string"] = tname;




					// We know these map onto basic types at some level
					tname = typeid(int8).name();
					name["int8"] = tname;

					tname = typeid(uint8).name();
					name["uint8"] = tname;

					tname = typeid(int16).name();
					name["int16"] = tname;

					tname = typeid(uint16).name();
					name["uint16"] = tname;

					tname = typeid(int32).name();
					name["int32"] = tname;

					tname = typeid(uint32).name();
					name["uint32"] = tname;

					tname = typeid(int64).name();
					name["int64"] = tname;

					tname = typeid(uint64).name();
					name["uint64"] = tname;

					tname = typeid(float32).name();
					name["float32"] = tname;

					tname = typeid(float64).name();
					name["float64"] = tname;


				}
			}
			~TypeInfo() {};

			static std::map<const char*,int> id;
			static std::map<const char*,const char*> name;
	};
	std::map<const char*,int> TypeInfo::id;
	std::map<const char*,const char*> TypeInfo::name;


	template <typename T> class cvector : public std::vector<T> {
		public:
			cvector() {}
			cvector(size_t n, const T& value = T()): std::vector<T>(n, value) {}
			template <typename _InputIterator> cvector(
					_InputIterator i, _InputIterator j):std::vector<T>(i, j) {}

		protected:
	};


	class DynamicStructField {
		public:
			DynamicStructField() {
				mTypeInfo.init();
			};
			~DynamicStructField() {};

			void init(
					const char* name,
					const char* type_name_in, 
					size_t nel=1) {

				if (0 == mTypeInfo.name.count(type_name_in)) {
					ostringstream err;
					err<<"Type name not found: '"<<type_name_in<<"'"<<endl;
					throw std::runtime_error(err.str());
				}
				const char* type_name = mTypeInfo.name[type_name_in];
				mTypeId = mTypeInfo.id[type_name];
				mNel = nel;
				mName = name;

				resize_data(mNel);
			}

			template <class T> void init(
					const char* name,
					const vector<T>& data) {

				mNel = data.size();
				const char* type_name = typeid(T).name();
				mTypeId = mTypeInfo.id[type_name];
				mName = name;

				vector<T>* dptr = (vector<T>* )&mData;

				(*dptr).resize(mNel);
				std::copy(data.begin(), data.end(), (*dptr).begin() );
			}

			/*
			template <class T> operator[](const size_t index) {
				vector<T>* dptr = (vector<T>* )&mData;
				return (*dptr)[index];
			}
			*/

			template <class T> operator T() {
				vector<T> temp(1);
				_copy_vector(temp, 0, 1);
				return temp[0];
			}

			template <class T> operator T&() const {
				// todo: add type checking
				T& temp = *(T* ) &mData[0];
				return temp;
			}

			template <class T> operator vector<T>&() const {
				// todo: add type checking
				vector<T>& temp = *(vector<T>* ) &mData;
				return temp;
			}

			/*
			   template <class T> vector<T>& getref() const {
			   vector<T>& temp = *(vector<T>* ) &mData;
			   return temp;
			   }
			   */

			/*
			   template <class T> operator vector<T>() const {
			   vector<T> temp;
			   _copy_vector(temp);
			   return temp;
			   }
			   */

			template <class T> vector<T> copy() const {
				vector<T> temp;
				size_t begin=0;
				_copy_vector(temp, begin, mNel);
				return temp;
			}
			template <class T> vector<T> copy(size_t begin, size_t ncopy) const {
				vector<T> temp;
				_copy_vector(temp, begin, ncopy);
				return temp;
			}


			template <class T> void copy(vector<T>& vec) {
				size_t begin=0;
				_copy_vector(vec, begin, mNel);
			}
			template <class T> void copy(vector<T>& vec, size_t begin, size_t ncopy) 
			{
				_copy_vector(vec, begin, ncopy);
			}


			// our field is a string.  Two cases: copying to another vector
			// of strings or to some other type
			void _copy_from_strings(
					vector<string>&vec, size_t begin, size_t ncopy) const {

				vector<string>* casted = (vector<string>* ) &mData;
				std::copy(
						(*casted).begin() + begin,
						(*casted).begin() + begin + ncopy,
						vec.begin() );

			}
			// copying from string to another type.  Use a stringstream
			template <class T> void _copy_from_strings(
					vector<T>&vec, size_t begin, size_t ncopy) const {

				vector<string>* casted = (vector<string>* ) &mData;
				vector<string>::iterator iter = (*casted).begin();
				iter = iter + begin;
				for (size_t i=0; i<ncopy; i++) {
					std::stringstream ss;
					ss << (*iter);
					ss >> vec[i];
					iter++;
				}
			}


			// Our field is a standard type
			template <class T1, class T2> void _tcopy(
					T1 t1, vector<T2>& vec, size_t begin, size_t ncopy) const 
			{
				vector<T1>* casted = (vector<T1>* ) &mData;
				std::copy(
						(*casted).begin()+begin,
						(*casted).begin()+begin+ncopy,
						vec.begin());
			}

			template <class T> void _copy_vector(
					vector<T>& vec, size_t begin, size_t ncopy) const {

				vec.resize(ncopy);

				switch (mTypeId) {
					case 0: 
						{
							std::copy(
									mData.begin()+begin, 
									mData.begin()+begin+ncopy, 
									vec.begin() );
						}
						break;
					case 1: 
						{
							uchar t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 2: 
						{
							short t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 3: 
						{ 
							ushort t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 4:
						{
							int t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 5: 
						{
							uint t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 6: 
						{
							long t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 7: 
						{
							ulong t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 8: 
						{
							long long t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 9: 
						{
							ulonglong t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 10: 
						{
							float t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 11: 
						{
							double t1;
							_tcopy(t1, vec, begin, ncopy);
						}
						break;
					case 12:
						{
							_copy_from_strings(vec, begin, ncopy);
						}
						break;
					default:
						ostringstream err;
						err<<"Unknown type id: "<<mTypeId<<endl;
						throw std::runtime_error(err.str());

				}
			}


			// Resize the data to a given number of elements
			void resize_data(size_t nel) {

				switch (mTypeId) {
					case 0: 
						{
							mData.resize(nel);
						}
						break;
					case 1: 
						{
							vector<uchar>* dptr = 
								(vector<uchar>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 2: 
						{
							vector<short>* dptr = 
								(vector<short>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 3: 
						{ 
							vector<ushort>* dptr = 
								(vector<ushort>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 4:
						{
							vector<int>* dptr = 
								(vector<int>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 5: 
						{
							vector<uint>* dptr = 
								(vector<uint>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 6: 
						{
							vector<long>* dptr = 
								(vector<long>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 7: 
						{
							vector<ulong>* dptr = 
								(vector<ulong>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 8: 
						{
							vector<long long>* dptr = 
								(vector<long long>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 9: 
						{
							vector<ulonglong>* dptr = 
								(vector<ulonglong>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 10: 
						{
							vector<float>* dptr = 
								(vector<float>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 11: 
						{
							vector<double>* dptr = 
								(vector<double>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					case 12:
						{
							vector<string>* dptr = 
								(vector<string>*) &mData;
							(*dptr).resize(nel);
						}
						break;
					default:
						ostringstream err;
						err<<"Unknown type id: "<<mTypeId<<endl;
						throw std::runtime_error(err.str());

				}
			}



		protected:
			vector<char> mData;
			int mTypeId;
			string mName;
			size_t mNel;

			TypeInfo mTypeInfo;

	};


	class DynamicStruct {
		public:
			// Only support scalar types right now for simplicity
			//DynamicStruct();
			DynamicStruct()  {
				mTypeInfo.init();
			}

			// Everything is std containers so no need for memory management here
			~DynamicStruct() {}

			// Return a reference to the field object associated with name
			DynamicStructField& operator[] (const char* name) {
				_ensure_field_exists(name);
				DynamicStructField& tf = mFieldMap[name];
				return tf;
			}

			bool field_exists(const char* name) {
				return (mFieldMap.count(name) > 0) ? true : false;
			}

			// Create from the input vector.  All data are copied
			template <class T>
				void addfield(const char* name, const vector<T>& v)
				{
					_ensure_field_doesnt_exist(name);
					DynamicStructField fnew;
					mFieldMap.insert( make_pair(name, fnew) );
					mFieldMap[name].init(name, v);
				}

			// Create from a scalar.  Data are copied to a vector and input
			// using the above addfield for vectors
			template <class T>
				void addfield(const char* name, const T& s)
				{
					_ensure_field_doesnt_exist(name);
					vector<T> v(1);
					v[0] = s;
					addfield(name, v);
				}

			// Create from a type name
			void addfield(const char* name, const char* type_name, size_t nel=1) {
				_ensure_field_doesnt_exist(name);
				DynamicStructField fnew;
				mFieldMap.insert( make_pair(name, fnew) );
				mFieldMap[name].init(name, type_name, nel);
			}

			// Add multiple scalars
			void addfields(vector<string>& names, vector<string>& type_names) {
				if (names.size() != type_names.size() ) {
					throw std::runtime_error("names and types must be same size\n");
				}
				for (size_t i=0; i<names.size(); i++) {
					addfield(names[i].c_str(), type_names[i].c_str());
				}
			}
			// Add multiple fields with sizes
			void addfields(
					vector<string>& names, 
					vector<string>& type_names,
					vector<size_t>& sizes) {
				if ( (names.size() != type_names.size()) || 
						(names.size() != sizes.size()) ) {
					throw 
						std::runtime_error("names,types,lengths must be same size\n");
				}
				for (size_t i=0; i<names.size(); i++) {
					addfield(names[i].c_str(), type_names[i].c_str(), sizes[i]);
				}
			}


			void _ensure_field_exists(const char* name) {
				if (!field_exists(name)) {
					ostringstream err;
					err<<"Field '"<<name<<"' does not exist"<<endl;
					throw std::runtime_error(err.str());
				}
			}
			void _ensure_field_doesnt_exist(const char* name) {
				if (field_exists(name)) {
					ostringstream err;
					err<<"Field '"<<name<<"' already exists"<<endl;
					throw std::runtime_error(err.str());
				}
			}


		private:

			std::map<const char*,DynamicStructField> mFieldMap;
			TypeInfo mTypeInfo;

	};


}

#endif //_DYNAMIC_STRUCT_H
