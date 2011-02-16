/*
 * To do: 
 *
 * We've got in-place operators on DynamicField, such as *=
 *
 * implement arithmetic operators on DynamicFieldElement 
 *
 * implement = operators for DynamicField: require same length and
 * and instead delegate such things to a .reset()?
 *
 * implmement = and arithmetic operators working between DynamicFields and
 * DynamicFieldElements
 *
 * What about multiplying different fields?
 *
 * what about operations like this:
 *   ds["field1"] = ds2["field2"]*ds3["field3"]*somevector*scalar1 
 *     + scalar2*ds4["field4"] + scalar3 + vector2
 *
 *
 * Might want to make a new type Vector or something that will do all the 
 * element-by-element stuff, and use that internally.    That way someone can
 * declare a Vector that will have element-by-element properties.  It should
 * support operations with std::vector too and return Vector when needed.
 *
 * either that or always allow conversion to vector.  Does that make an extra
 * copy though?  I need to explore that and see when a copy is made. Might
 * only not make a copy on construction.
 *
 */

#include <iostream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <map>
#include <typeinfo>
#include <stdint.h>
#include <algorithm>

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


// forward declarations
class DynamicField;
class ConstantDynamicField;
class DynamicFieldElement;
class ConstantDynamicFieldElement;



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


				tname = typeid(std::string).name();
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








// use this base class with no type, gets inherited by Field<T>
// This way we can keep pointers like this:
// FieldBase* ptr = new Field<T>(..);
// and these can be cast to Field<T>* to get the data.
class FieldBase
{
	public:
		FieldBase() {};
		virtual ~FieldBase() {};

		// This virtual function is the key
		virtual int type_id() {
			return -1;
		};
		virtual std::string type_name() {
			return "";
		};


		virtual void write(std::ostream& os) const {
			os<<"NULL";
		}
		virtual void write(size_t index, std::ostream& os) const {
			os<<"NULL";
		}
		virtual void print() const { 
			write(std::cout); 
			std::cout<<std::endl; 
		}
	private:
		int mTypeId;
};

inline std::ostream& operator<<( std::ostream& os, const FieldBase& field)
{
	field.write(os);
	return os;
}


class StringField: public FieldBase
{
	public:
		
		StringField(size_t n, const std::string& value = "") {
			mTypeInfo.init();

			mData.resize(n, value);
			mTypeName = typeid(std::string).name();
			mTypeId = mTypeInfo.id[mTypeName];
		}
		// is this making two copies?
		StringField(std::vector<std::string> tdata) {
			mData = tdata;
			mTypeInfo.init();

			mData = tdata;

			mTypeName = typeid(std::string).name();
			mTypeId = mTypeInfo.id[mTypeName];
		}


		// These two virtual functions are the key
		virtual int type_id() {
			return mTypeId;
		};
		virtual std::string type_name() {
			return mTypeName;
		};



		virtual inline std::string& operator[](const size_t index)  {
			// at() does bounds checking but not []
			// This returns a reference
			return mData.at(index);
		}

		virtual void write(std::ostream& os) const { 
			if (0 == mData.size()) {
				os<<"NULL";
			} else if (1 == mData.size()) {
				os<<"'"<<mData[0]<<"'";
			} else {
				os<<"{";
				for (size_t i=0;i<mData.size(); i++) {
					if (i > 0) { os<<", "; }
					os<<"'"<<mData[i]<<"'";
				}
				os<<"}";
			}
		}
		virtual void write(size_t index, std::ostream& os) const { 
			os<<"'"<<mData.at(index)<<"'";
		}



		// copy scalar string into each element
		void set(std::string& s) {
			std::fill( mData.begin(), mData.end(), s);
		}
		// convert a scalar to a string and copy into each element of this
		// field
		template <typename T> void set(T& val) {
			std::stringstream ss;
			ss << val;
			std::string tmp = ss.str();

			std::fill( mData.begin(), mData.end(), tmp);
		}

		// copy in the vector of type T as strings
		template <typename T> void set(std::vector<T>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}

			mIter = mData.begin();
			typename std::vector<T>::iterator vec_iter = vec.begin();

			while (mIter != mData.end()) {
				std::stringstream ss;

				ss << *vec_iter;
				ss >> *mIter;

				++mIter;
				++vec_iter;
			}

		}

		// copy scalar string into each element
		void set(size_t index, std::string& s) {
			mData.at(index) = s;
		}
		// convert a scalar to a string and copy into each element of this
		// field
		template <typename T> void set(size_t index, T& val) {
			std::stringstream ss;
			ss << val;

			mData.at(index) = ss.str();
		}


		// implementation is later
		void set(DynamicField& df);
		void copy(DynamicField& df);



		// copy element 'index' from the data and store in the input
		// string variable
		void copy(size_t index, std::string& s) {
			s = mData.at(index);
		}
		// copy element 'index' from the data and store in the input
		// variable, performing conversion using a string stream
		// this may very well fail in many cases
		template <typename T>
		void copy(size_t index, T& val) {
			std::stringstream ss;
			ss << mData.at(index);

			if ( !(ss >> val) ) {
				std::stringstream err;
				err<<"Could not convert string value '"
					<<ss.str()<<"' to type '"<<typeid(T).name()<<"'";
				throw std::runtime_error(err.str());
			}

		}

		// copy the data from this field into the scalar string
		// only will work if this is a "scalar", which for our
		// current setup means a vector of length 1
		void copy(std::string& s) {
			if (mData.size() != 1) {
				std::ostringstream err;
				err<<
				"Attempt to copy to scalar from field of length "<<mData.size();
				throw std::runtime_error(err.str());
			}
			s=mData[0];
		}
		template <typename T> void copy(T& scalar) {
			if (mData.size() != 1) {
				std::ostringstream err;
				err<<
				"Attempt to copy to scalar from field of length "<<mData.size();
				throw std::runtime_error(err.str());
			}
			std::stringstream ss;
			ss << mData[0];
			ss >> scalar;
		}

		// copy the data from this field into a vector of strings
		void copy(std::vector<std::string>& vec) {
			vec = mData;
		}
		void set(std::vector<std::string>& vec) {
			mData=vec;
		}

		// copy the data into a vector, performing conversion using a
		// string stream.  This may fail of course.
		template <typename T> void copy(std::vector<T>& vec) {
			vec.clear();
			vec.resize(mData.size(),0);

			mIter = mData.begin();
			typename std::vector<T>::iterator vec_iter = vec.begin();

			while (mIter != mData.end()) {
				std::stringstream ss;

				ss << *mIter;

				if ( !(ss >> *vec_iter) ) {
					std::stringstream err;
					err<<"Could not convert string value '"
						<<ss.str()<<"' to type '"<<typeid(T).name()<<"'";
					throw std::runtime_error(err.str());
				}

				++mIter;
				++vec_iter;
			}
		}


	private:
		std::vector<std::string> mData;
		std::vector<std::string>::iterator mIter;

		//std::string mTypeName;
		const char* mTypeName;
		int mTypeId;

		TypeInfo mTypeInfo;

};


// This is for built-int types such as int, double, etc.
template <typename T>
class Field: public FieldBase
{
	public:
		
		/*
		 
		Field(T tval) {
			mData.resize(1);
			mData[0]=tval;
		}
		Field(std::vector<T> tdata) {
			mData.resize(tdata.size());
			std::copy(tdata.begin(), tdata.end(), mData.begin() );
		}
		*/

		Field(size_t n, const T& value = T()) {
			mTypeInfo.init();
			string_type_id = mTypeInfo.id[typeid(std::string).name()];

			mData.resize(n, value);
			mTypeName = typeid(T).name();
			mTypeId = mTypeInfo.id[mTypeName];
		}
		// is this making two copies?
		Field(std::vector<T> tdata) {
			mTypeInfo.init();
			string_type_id = mTypeInfo.id[typeid(std::string).name()];

			mData = tdata;

			mTypeName = typeid(T).name();
			mTypeId = mTypeInfo.id[mTypeName];
		}


		// These two virtual functions are the key
		virtual int type_id() {
			return mTypeId;
		};
		virtual std::string type_name() {
			return mTypeName;
		};


		virtual inline T& operator[](const size_t index)  {
			// at() does bounds checking but not []
			// This returns a reference
			return mData.at(index);
		}


		virtual void write(std::ostream& os) const { 
			if (0 == mData.size()) {
				os<<"NULL";
			} else if (1 == mData.size()) {
				os<<mData[0];
			} else {
				os<<"{";
				for (size_t i=0;i<mData.size(); i++) {
					if (i > 0) { os<<", "; }
					os<<mData[i];
				}
				os<<"}";
			}
		}
		virtual void write(size_t index, std::ostream& os) const { 
			os<<mData.at(index);
		}

		// copy the built-in types from this field to a string
		void copy(std::string& s) {
			if (mData.size() != 1) {
				std::ostringstream err;
				err<<
				"Attempt to copy to scalar from field of length "<<mData.size();
				throw std::runtime_error(err.str());
			}
			std::stringstream ss;
			ss << mData[0];
			ss >> s;
		}
		void copy(std::vector<std::string>& svec) {
			svec.clear();
			svec.resize(mData.size(),"");
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << mData[i];
				svec[i] = ss.str();
			}
		}

		void copy(std::vector<T>& vec) {
			vec = mData;
		}
		void set(std::vector<T>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			mData = vec;
		}


		// this will copy between all built-in types, such as int, double, ...
		template <typename T2> void copy(std::vector<T2>& vec) {
			vec.clear();
			vec.resize(mData.size(),0);
			std::copy( mData.begin(), mData.end(), vec.begin() );
		}
		template <typename T2> void copy(T2& scalar) {
			if (mData.size() != 1) {
				std::ostringstream err;
				err<<
				"Attempt to copy to scalar from field of length "<<mData.size();
				throw std::runtime_error(err.str());
			}
			scalar = mData[0];
		}

		void multiply(std::string& str) {
			throw std::runtime_error("Cannot multiply strings");
		}
		void multiply(std::vector<std::string>& svec) {
			throw std::runtime_error("Cannot multiply string vector");
		}
		void multiply_out(std::vector<std::string>& svec) {
			throw std::runtime_error("Cannot multiply string vector");
		}
		void divide(std::string& str) {
			throw std::runtime_error("Cannot divide strings");
		}
		void divide(std::vector<std::string>& svec) {
			throw std::runtime_error("Cannot divide string vector");
		}
		void divide_out(std::vector<std::string>& svec) {
			throw std::runtime_error("Cannot divide string vector");
		}

		// implementation later
		void set(DynamicField& df);
		void copy(DynamicField& df);
		void reset(DynamicField& df);
		void multiply(DynamicField& df);
		void divide(DynamicField& df);



		template <typename T2> void multiply(T2 scalar) {
			for (size_t i=0; i<mData.size(); i++) {
				mData[i] *= scalar;
			}
		}

		template <typename T2> void multiply(std::vector<T2>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i<mData.size(); i++) {
				mData[i] *= vec[i];
			}
		}
		template <typename T2> void multiply_out(std::vector<T2>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i<mData.size(); i++) {
				vec[i] *= mData[i];
			}
		}



		template <typename T2> void divide(T2 scalar) {
			for (size_t i=0; i<mData.size(); i++) {
				mData[i] /= scalar;
			}
		}
		template <typename T2> void divide(std::vector<T2>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i<mData.size(); i++) {
				mData[i] /= vec[i];
			}
		}
		template <typename T2> void divide_out(std::vector<T2>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i<mData.size(); i++) {
				vec[i] /= mData[i];
			}
		}






		// copy the vector of strings into this field's data, converting
		// using a string stream
		void set(std::vector<std::string>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}

			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << vec[i];
				ss >> mData[i];
			}
		}

		void reset(std::vector<std::string>& vec) {
			mData.resize( vec.size() );
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << vec[i];
				ss >> mData[i];
			}
		}
		void reset_out(std::vector<std::string>& vec) {
			vec.resize( mData.size() );
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << mData[i];
				ss >> vec[i];
			}
		}



		// copy the scalar into all the elements of the field's data, 
		// converting using a string stream
		void set(std::string& s) {
			T tmp;
			std::stringstream ss;
			ss << s;
			ss >> tmp;
			for (size_t i=0; i< mData.size(); i++) {
				mData[i] = tmp;
			}
		}




		// copy the scalar into this field's n'th data element, performing
		// automatic type conversion
		template <typename T2> void set(size_t index, T2& scalar) {
			mData.at(index) = scalar;
		}
		// copy from index to scalar
		template <typename T2> void copy(size_t index, T2& scalar) {
			scalar = mData.at(index);
		}

		// copy the scalar into this field's n'th data element, performing
		// automatic type conversion
		void set(size_t index, std::string& s) {
			std::stringstream ss;
			ss << s;
			ss >> mData.at(index);
		}
		// copy from index to scalar
		void copy(size_t index, std::string& s) {
			std::stringstream ss;
			ss << mData.at(index);
			s = ss.str();
		}



		// copy the scalar into all the elements of the field's data, 
		// performing automatic type conversion
		template <typename T2> void set(T2 scalar) {
			std::fill( mData.begin(), mData.end(), scalar);
		}
		// copy the vector element-by-element into the field's data, 
		// performing automatic type conversion
		template <typename T2> void set(std::vector<T2>& vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}

			std::copy( vec.begin(), vec.end(), mData.begin() );
		}

		// reset the size and copy the vector element by element
		void reset(std::vector<T>& vec) {
			mData = vec;
		}
		template <typename T2> void reset(std::vector<T2>& vec) {
			mData.resize(vec.size());
			std::copy( vec.begin(), vec.end(), mData.begin() );
		}
		// reset the size and copy the vector element by element
		template <typename T2> void reset_out(std::vector<T2>& vec) {
			vec.resize(mData.size());
			std::copy( mData.begin(), mData.end(), vec.begin() );
		}




	private:
		std::vector<T> mData;
		//std::string mTypeName;
		const char* mTypeName;
		int mTypeId;

		TypeInfo mTypeInfo;

		int string_type_id;
};


class DynamicStruct {
	public:
		// Only support scalar types right now for simplicity
		//DynamicStruct();
		DynamicStruct()  {
			// re-implement this later (copy from DynamicStruct.h)
			mTypeInfo.init();
		}

		~DynamicStruct() {
			// second element in map is pointers to FieldBase created with new
			// free with delete
			std::map<const char*,FieldBase*>::iterator iter;
			for (iter=mFieldMap.begin(); iter != mFieldMap.end(); ++iter) {
				delete iter->second;
			}
		}

		/*
		// Return a reference to the field object associated with name
		FieldBase& operator[] (const char* name) {
			_ensure_field_exists(name);
			FieldBase& tf = *mFieldMap[name];
			return tf;
		}
		*/

		// these must be specified below DynamicField
		DynamicField operator[](const char* fieldname);
		// having trouble with this
		const ConstantDynamicField operator[](const char* fieldname) const;

		bool field_exists(const char* name) const {
			return (mFieldMap.count(name) > 0) ? true : false;
		}

		void addfield(const char* name, const std::vector<std::string>& v)
		{
			_ensure_field_doesnt_exist(name);
			mFieldMap.insert( std::make_pair(name,new StringField(v)) );
		}
		// Create from the input vector.  All data are copied
		template <class T> void addfield(
				const char* name, const std::vector<T>& v)
		{
			_ensure_field_doesnt_exist(name);
			mFieldMap.insert( std::make_pair(name, new Field<T>(v) ) );
		}

		// Create from a scalar.  Data are copied to a vector and input
		// using the above addfield for vectors
		void addfield(const char* name, const char* s)
			{
				_ensure_field_doesnt_exist(name);
				std::vector<std::string> v(1);
				v[0] = s;
				addfield(name, v);
			}

		void addfield(const char* name, std::string s)
			{
				_ensure_field_doesnt_exist(name);
				std::vector<std::string> v(1);
				v[0] = s;
				addfield(name, v);
			}

		template <class T> void addfield(const char* name, const T& s)
			{
				_ensure_field_doesnt_exist(name);
				std::vector<T> v(1);
				v[0] = s;
				addfield(name, v);
			}

		/*
		// Create from a type name
		void addfield(const char* name, const char* type_name, size_t nel=1) {
			_ensure_field_doesnt_exist(name);
			DynamicStructField fnew;
			mFieldMap.insert( make_pair(name, fnew) );
			mFieldMap[name].init(name, type_name, nel);
		}

		// Add multiple scalars
		void addfields(vector<std::string>& names, vector<std::string>& type_names) {
			if (names.size() != type_names.size() ) {
				throw std::runtime_error("names and types must be same size\n");
			}
			for (size_t i=0; i<names.size(); i++) {
				addfield(names[i].c_str(), type_names[i].c_str());
			}
		}
		// Add multiple fields with sizes
		void addfields(
				vector<std::string>& names, 
				vector<std::string>& type_names,
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

		*/
		void _ensure_field_exists(const char* name) const {
			if (!field_exists(name)) {
				std::ostringstream err;
				err<<"Field '"<<name<<"' does not exist"<<std::endl;
				throw std::runtime_error(err.str());
			}
		}
		void _ensure_field_doesnt_exist(const char* name) const {
			if (field_exists(name)) {
				std::ostringstream err;
				err<<"Field '"<<name<<"' already exists"<<std::endl;
				throw std::runtime_error(err.str());
			}
		}



		template <typename T> void operate_element(
				StringField* field,size_t index,T& scalar,const char opcode) {

			switch (opcode)
			{
				case '<': field->set(index, scalar); break;
				case '>': field->copy(index, scalar); break;
				default: {
							 std::stringstream err;
							 err<<"Bad string op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}

		template <typename T1, typename T2> void operate_element(
				Field<T1>* field,size_t index,T2& scalar,const char opcode) {

			switch (opcode)
			{
				case '<': field->set(index, scalar); break;
				case '>': field->copy(index, scalar); break;
				default: {
							 std::stringstream err;
							 err<<"Bad string op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}




		template <typename T> void operate_scalar(
				StringField* field, T& scalar, const char opcode) {

			switch (opcode)
			{
				case '<': field->set(scalar); break; // < means copy in
				case '>': field->copy(scalar); break; // > means copy out
				default: {
							 std::stringstream err;
							 err<<"Bad string op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}

		template <typename T1, typename T2> void operate_scalar(
				Field<T1>* field, T2& scalar, const char opcode) {

			switch (opcode)
			{
				case '<': field->set(scalar); break; // < means copy in
				case '>': field->copy(scalar); break; // > means copy out
				case '*': field->multiply(scalar); break;
				case '/': field->divide(scalar); break;
				default: {
							 std::ostringstream err;
							 err<<"Bad op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}



		template <typename T> void operate_vector(
				StringField* field,std::vector<T>& vec,const char opcode) {

			switch (opcode)
			{
				case '<': field->set(vec); break;
				case '>': field->copy(vec); break;
				default: {
							 std::stringstream err;
							 err<<"Bad string op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}

		template <typename T1, typename T2> void operate_vector(
				Field<T1>* field, std::vector<T2>& vec, const char opcode) {

			switch (opcode)
			{
				case '=': field->reset(vec); break;// = field can get resized
				case 'r': field->reset_out(vec); break;// = vec reset
				case '>': field->copy(vec); break; // > means copy "out"
				case '<': field->set(vec); break;  // > means copy "in"
				case '*': field->multiply(vec); break; // this field multiplied
				case 't': field->multiply_out(vec); break; // vec multiplied
				case '/': field->divide(vec); break; // this field divided
				case 'd': field->divide_out(vec); break; // vec is divided
				default: {
							 std::ostringstream err;
							 err<<"Bad op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }
			}

		}



		void operate_dfield(
				StringField* field, DynamicField& df, const char opcode) {

			switch(opcode)
			{
				//case '=': field->reset(vec); break;// = field can get resized
				case '>': field->copy(df); break; // > means copy "out"
				case '<': field->set(df); break;  // < means copy "in" from df
				//case '*': field->multiply(vec); break;
				//case '/': field->divide(vec); break;
				default: {
							 std::ostringstream err;
							 err<<"Bad op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }

			}
		}


		template <typename T> void operate_dfield(
				Field<T>* field, DynamicField& df, const char opcode) {

			switch(opcode)
			{
				case '=': field->reset(df); break;// = field can get resized
				case '>': field->copy(df); break; // > means copy "out"
				case '<': field->set(df); break;  // < means copy "in" from df
				case '*': field->multiply(df); break;
				case '/': field->divide(df); break;
				default: {
							 std::ostringstream err;
							 err<<"Bad op code "<<opcode;
							 throw std::runtime_error(err.str());
						 }

			}
		}

		void cast_and_operate_dfield(FieldBase* fieldbase, DynamicField& df, const char opcode) {

			switch (fieldbase->type_id()) {
				case 0: 
					{
						Field<char>* field = (Field<char>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 1: 
					{
						Field<uchar>* field = (Field<uchar>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 2: 
					{
						Field<short>* field = (Field<short>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 3: 
					{ 
						Field<ushort>* field = (Field<ushort>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 4:
					{
						Field<int>* field = (Field<int>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 5: 
					{
						Field<uint>* field = (Field<uint>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 6: 
					{
						Field<long>* field = (Field<long>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 7: 
					{
						Field<ulong>* field = (Field<ulong>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 8: 
					{
						Field<long long>* field = (Field<long long>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 9: 
					{
						Field<ulonglong>* field = (Field<ulonglong>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 10: 
					{
						Field<float>* field = (Field<float>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 11: 
					{
						Field<double>* field = (Field<double>* ) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				case 12:
					{
						StringField* field = (StringField*) fieldbase;
						operate_dfield(field, df, opcode);
					}
					break;
				default:
					std::ostringstream err;
					err<<"Unknown type id: "<<fieldbase->type_id()<<std::endl;
					throw std::runtime_error(err.str());

			}

		}




		template <typename T> void cast_and_operate_element(
				FieldBase* fieldbase, size_t index, 
				T& scalar, const char opcode) {


			switch (fieldbase->type_id()) {
				case 0: 
					{
						Field<char>* field = (Field<char>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 1: 
					{
						Field<uchar>* field = (Field<uchar>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 2: 
					{
						Field<short>* field = (Field<short>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 3: 
					{ 
						Field<ushort>* field = (Field<ushort>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 4:
					{
						Field<int>* field = (Field<int>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 5: 
					{
						Field<uint>* field = (Field<uint>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 6: 
					{
						Field<long>* field = (Field<long>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 7: 
					{
						Field<ulong>* field = (Field<ulong>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 8: 
					{
						Field<long long>* field = (Field<long long>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 9: 
					{
						Field<ulonglong>* field = (Field<ulonglong>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 10: 
					{
						Field<float>* field = (Field<float>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 11: 
					{
						Field<double>* field = (Field<double>* ) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				case 12:
					{
						StringField* field = (StringField*) fieldbase;
						operate_element(field, index, scalar, opcode);
					}
					break;
				default:
					std::ostringstream err;
					err<<"Unknown type id: "<<fieldbase->type_id()<<std::endl;
					throw std::runtime_error(err.str());

			}


		}




		template <typename T> void cast_and_operate_scalar(
				FieldBase* fieldbase, T& scalar, const char opcode) {


			switch (fieldbase->type_id()) {
				case 0: 
					{
						Field<char>* field = (Field<char>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 1: 
					{
						Field<uchar>* field = (Field<uchar>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 2: 
					{
						Field<short>* field = (Field<short>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 3: 
					{ 
						Field<ushort>* field = (Field<ushort>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 4:
					{
						Field<int>* field = (Field<int>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 5: 
					{
						Field<uint>* field = (Field<uint>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 6: 
					{
						Field<long>* field = (Field<long>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 7: 
					{
						Field<ulong>* field = (Field<ulong>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 8: 
					{
						Field<long long>* field = (Field<long long>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 9: 
					{
						Field<ulonglong>* field = (Field<ulonglong>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 10: 
					{
						Field<float>* field = (Field<float>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 11: 
					{
						Field<double>* field = (Field<double>* ) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				case 12:
					{
						StringField* field = (StringField*) fieldbase;
						operate_scalar(field, scalar, opcode);
					}
					break;
				default:
					std::ostringstream err;
					err<<"Unknown type id: "<<fieldbase->type_id()<<std::endl;
					throw std::runtime_error(err.str());

			}


		}


		template <typename T> void cast_and_operate_vector(
				FieldBase* fieldbase,std::vector<T>& vec, const char opcode) {


			switch (fieldbase->type_id()) {
				case 0: 
					{
						Field<char>* field = (Field<char>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 1: 
					{
						Field<uchar>* field = (Field<uchar>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 2: 
					{
						Field<short>* field = (Field<short>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 3: 
					{ 
						Field<ushort>* field = (Field<ushort>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 4:
					{
						Field<int>* field = (Field<int>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 5: 
					{
						Field<uint>* field = (Field<uint>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 6: 
					{
						Field<long>* field = (Field<long>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 7: 
					{
						Field<ulong>* field = (Field<ulong>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 8: 
					{
						Field<long long>* field = (Field<long long>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 9: 
					{
						Field<ulonglong>* field = (Field<ulonglong>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 10: 
					{
						Field<float>* field = (Field<float>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 11: 
					{
						Field<double>* field = (Field<double>* ) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				case 12:
					{
						StringField* field = (StringField*) fieldbase;
						operate_vector(field, vec, opcode);
					}
					break;
				default:
					std::ostringstream err;
					err<<"Unknown type id: "<<fieldbase->type_id()<<std::endl;
					throw std::runtime_error(err.str());

			}


		}



		template <typename T>
		void multiply(const char* name, T& scalar) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_scalar(fb, scalar, '*');
		}
		
		template <typename T>
		void multiply(const char* name, std::vector<T>& vec) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, '*');
		}
		template <typename T>
		void multiply_out(const char* name, std::vector<T>& vec) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, 't');
		}


		template <typename T>
		void divide(const char* name, T& scalar) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_scalar(fb, scalar, '/');
		}
		
		template <typename T>
		void divide(const char* name, std::vector<T>& vec) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, '/');
		}
		template <typename T>
		void divide_out(const char* name, std::vector<T>& vec) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, 'd');
		}




		//
		// working with DynamicFields
		//

		// copy in from a dynamic field
		void set(const char* name, DynamicField& df ) {
			_ensure_field_exists(name);
			FieldBase* fb=mFieldMap[name];
			cast_and_operate_dfield(fb, df, '<');
		}
		// copy out from dynamic field
		void copy(const char* name, DynamicField& df ) {
			_ensure_field_exists(name);
			FieldBase* fb=mFieldMap[name];
			cast_and_operate_dfield(fb, df, '>');
		}
		void reset(const char* name, DynamicField& df ) {
			_ensure_field_exists(name);
			FieldBase* fb=mFieldMap[name];
			cast_and_operate_dfield(fb, df, '=');
		}

		void multiply(const char* name, DynamicField& df) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_dfield(fb, df, '*');
		}
		void divide(const char* name, DynamicField& df) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_dfield(fb, df, '/');
		}





		// working with individual elements
		template <typename T> 
		void copy(const char* name, const size_t index, T& scalar) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_element(fb, index, scalar, '>');
		}
		template <typename T>
		void set(const char* name, const size_t index, T& scalar)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_element(fb, index, scalar, '<');
		}

		void copy(const char* name, const size_t index, std::string& s)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_element(fb, index, s, '>');
		}
		void set(const char* name, const size_t index, std::string& s)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_element(fb, index, s, '<');
		}

		// copying in and out scalars
		/*
		void copy(const char* name, std::string& s) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_string(fb, s, '>');
		}
		void set(const char* name, std::string& s) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_string(fb, s, '<');
		}
		*/



		// copying out and copying in vectors
		template <typename T>
		void copy(const char* name, std::vector<T>& vec)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, '>');
		}

		template <typename T>
		void set(const char* name, std::vector<T>& vec)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, '<');
		}

		// This is setting all elements to the scalar value.  Note 
		// signature
		template <typename T>
		void set(const char* name, T& val)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_scalar(fb, val, '<');
		}
		template <typename T> 
		void copy(const char* name, T& val) {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_scalar(fb, val, '>');
		}



		template <typename T>
		void reset(const char* name, std::vector<T>& vec)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, '=');
		}
		template <typename T>
		void reset_out(const char* name, std::vector<T>& vec)  {
			_ensure_field_exists(name);
			FieldBase* fb = mFieldMap[name];
			cast_and_operate_vector(fb, vec, 'r');
		}




		void write(const char* name, std::ostream& os) const {
			_ensure_field_exists(name);
			std::map<const char*, FieldBase* >::const_iterator iter = 
				mFieldMap.find(name);
			FieldBase* field = iter->second;
			field->write(os);
		}

		void write(const char* name, size_t index, std::ostream& os) const {
			_ensure_field_exists(name);
			std::map<const char*, FieldBase* >::const_iterator iter = 
				mFieldMap.find(name);
			FieldBase* field = iter->second;
			field->write(index, os);
		}



	private:

		// can't do auto_ptr here, have to free later by hand
		std::map<const char*, FieldBase* > mFieldMap;
		TypeInfo mTypeInfo;

};

class ConstantDynamicFieldElement
{
	const DynamicStruct& ds;
	const char* fieldname;
	const size_t n;


	public:

	ConstantDynamicFieldElement(const DynamicStruct& _ds, const char* _fn, size_t _n) :
		ds(_ds), fieldname(_fn), n(_n) {}

	template <class T> operator T() const { 
		T tmp;
		ds.copy(fieldname,n,tmp);
		return tmp;
	}

	/*
	template <class T> void operator=(const T& x) { 
		ds.set(fieldname,n,x); 
	}
	*/

	void write(std::ostream& os) const {
		ds.write(fieldname, n, os);
	}
};


class DynamicFieldElement
{
	DynamicStruct& ds;
	const char* fieldname;
	const size_t n;


	public:

	DynamicFieldElement(DynamicStruct& _ds, const char* _fn, size_t _n) :
		ds(_ds), fieldname(_fn), n(_n) {}

	template <class T> operator T() const {
		T tmp;
		ds.copy(fieldname,n,tmp);
		return tmp;
	}

	std::string str() {
		std::string s;
		ds.copy(fieldname, n, s);
		return s;
	}	

	template <class T> void operator=(T x)
	{ ds.set(fieldname,n,x); }




	void copy(std::string& s) {
		ds.copy(fieldname, n, s);
	}
	template <typename T> void copy(T& scalar) {
		ds.copy(fieldname, n, scalar);
	}
	/*
	void copy(DynamicFieldElement& dfe) {
		ds.copy(fieldname, n, dfe);
	}
	*/


	void set(std::string& s) {
		ds.set(fieldname, n, s);
	}
	void set(const char* cch) {
		std::string s=cch;
		ds.set(fieldname, n, s);
	}
	template <typename T> void set(T& scalar) {
		ds.set(fieldname, n, scalar);
	}
	template <typename T> void set(T scalar) {
		ds.set(fieldname, n, scalar);
	}
	/*
	void set(DynamicFieldElement& dfe) {
		ds.set(fieldname,n, dfe);
	}
	*/




	void write(std::ostream& os) const {
		ds.write(fieldname, n, os);
	}


};
inline std::ostream& 
operator<<( std::ostream& os, const DynamicFieldElement& dfe ) {
	dfe.write(os);
	return os;
}
inline std::ostream& 
operator<<( std::ostream& os, const ConstantDynamicFieldElement& dfe ) {
	dfe.write(os);
	return os;
}



class ConstantDynamicField
{
	const DynamicStruct& ds;
	const char* fieldname;


	public:

	ConstantDynamicField(const DynamicStruct& _ds, const char* _fn) :
		ds(_ds), fieldname(_fn) {}


	const ConstantDynamicFieldElement operator[](size_t n) const
	{ return ConstantDynamicFieldElement(ds,fieldname,n); }

	//DynamicFieldElement operator[](int n)
	//{ return DynamicFieldElement(ds,fieldname,n); }
	
	/*
	template <typename T>
	void copy(std::vector<T>& vec) {
		ds.copy(fieldname, vec);
	}

	operator std::string() {
		std::string s;
		copy(s);
		return s;
	}
	void copy(std::string& s) {
		ds.copy(fieldname, s);
	}
	*/


	void write(std::ostream& os) const {
		ds.write(fieldname, os);
	}

};


class DynamicField
{
	DynamicStruct& ds;
	const char* fieldname;


	public:

	DynamicField(DynamicStruct& _ds, const char* _fn) :
		ds(_ds), fieldname(_fn) {}



	//const DynamicFieldElement operator[](int n) const
	//{ return DynamicFieldElement(ds,fieldname,n); }

	DynamicFieldElement operator[](size_t n)
	{ return DynamicFieldElement(ds,fieldname,n); }



	template <typename T> void operator*=(T& scalar) {
		ds.multiply(fieldname, scalar);
	}
	template <typename T> void operator*=(std::vector<T>& vec) {
		ds.multiply(fieldname, vec);
	}
	template <typename T> void multiply_out(std::vector<T>& vec) {
		ds.multiply_out(fieldname, vec);
	}
	// why only passing by value works?
	void operator*=(DynamicField df) {
		ds.multiply(fieldname, df);
	}

	template <typename T> void operator/=(T& scalar) {
		ds.divide(fieldname, scalar);
	}
	template <typename T> void operator/=(std::vector<T>& vec) {
		ds.divide(fieldname, vec);
	}
	template <typename T> void divide_out(std::vector<T>& vec) {
		ds.divide_out(fieldname, vec);
	}
	// why only passing by value works?
	void operator/=(DynamicField df) {
		ds.divide(fieldname, df);
	}


	DynamicField& operator=(std::vector<std::string>& svec) {
		ds.set(fieldname,svec);
		return *this;
	}
	/*
	 * called ambiguous, must use set
	DynamicField& operator=(std::string& s) {
		ds.set(fieldname,s);
		return *this;
	}
	*/

	// This copies in a vector
	template <typename T> DynamicField& operator=(std::vector<T>& vec) {
		ds.set(fieldname,vec);
		return *this;
	}
	// This copies in a scalar
	/*
	template <typename T> DynamicField& operator=(T& val) {
		ds.set(fieldname,val);
		return *this;
	}
	*/
	template <typename T> DynamicField& operator=(T val) {
		ds.set(fieldname,val);
		return *this;
	}


	// This copies in from a DynamicField
	// NOTE: I had to not use a reference, why?
	DynamicField& operator=(DynamicField df) {
		ds.set(fieldname, df);
		return *this;
	}

	// This doesn't seem to help with copying out strings
	// get same error even if not overloaded
#if 0
	operator std::string() {
		std::string s;
		copy(s);
		return s;
	}
#endif

	std::string str() {
		std::string s;
		copy(s);
		return s;
	}

	template <typename T> operator T() {
		T tmp;
		copy(tmp);
		return tmp;
	}

	template <typename T> operator std::vector<T>() {
		// make a copy of the data and return it
		// Note this method might use more memory than if the user just
		// called copy()
		std::vector<T> tmp;
		copy(tmp);
		return tmp;
	}





	template <typename T> void reset(std::vector<T>& vec) {
		ds.reset(fieldname,vec);
	}
	template <typename T> void reset_out(std::vector<T>& vec) {
		ds.reset_out(fieldname,vec);
	}
	// why only pass by value works?
	void reset(DynamicField df) {
		ds.reset(fieldname,df);
	}


	void copy(std::string& s) {
		ds.copy(fieldname, s);
	}
	void copy(std::vector<std::string>& svec) {
		ds.copy(fieldname, svec);
	}
	template <typename T> void copy(T& scalar) {
		ds.copy(fieldname, scalar);
	}
	template <typename T> void copy(std::vector<T>& vec) {
		ds.copy(fieldname, vec);
	}
	void copy(DynamicField& df) {
		ds.copy(fieldname,df);
	}


	void set(std::string& s) {
		ds.set(fieldname, s);
	}
	void set(const char* cch) {
		std::string s=cch;
		ds.set(fieldname, s);
	}
	void set(std::vector<std::string>& svec) {
		ds.set(fieldname, svec);
	}
	template <typename T> void set(T& scalar) {
		ds.set(fieldname, scalar);
	}
	template <typename T> void set(T scalar) {
		ds.set(fieldname, scalar);
	}
	template <typename T> void set(std::vector<T>& vec) {
		ds.set(fieldname, vec);
	}
	void set(DynamicField& df) {
		ds.set(fieldname,df);
	}







	void write(std::ostream& os) const {
		ds.write(fieldname, os);
	}


};



inline std::ostream& operator<<( std::ostream& os, const DynamicField& df ) {
	df.write(os);
	return os;
}
inline std::ostream& 
operator<<( std::ostream& os, const ConstantDynamicField& df ) {
	df.write(os);
	return os;
}





DynamicField DynamicStruct::operator[](const char* fieldname)
{ return DynamicField(*this,fieldname); }

const ConstantDynamicField DynamicStruct::operator[](const char* fieldname) const
{ return ConstantDynamicField(*this,fieldname); }


/*
 * Note things get reversed since for example set is on the field not the
 * DynamicField
 *
 * Note T appears twice
 */

template <typename T>
void Field<T>::set(DynamicField& df) {
	df.copy(mData);
}
void StringField::set(DynamicField& df) {
	df.copy(mData);
}

template <typename T>
void Field<T>::copy(DynamicField& df) {
	df = mData;
}
void StringField::copy(DynamicField& df) {
	df = mData;
}


// reset the data based on the input dynamic field
template <typename T>
void Field<T>::reset(DynamicField& df) {
	df.reset_out(mData);
}


template <typename T>
void Field<T>::multiply(DynamicField& df) {
	df.multiply_out(mData);
}
template <typename T>
void Field<T>::divide(DynamicField& df) {
	df.divide_out(mData);
}
