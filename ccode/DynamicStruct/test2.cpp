#include "testclass.h"
#include <string>
#include <stdio.h>
#include <boost/numeric/ublas/vector.hpp>


#define SLEEPLEN 10

using namespace std;

template <class T>
class Elements {
	public:
		Elements() {}
		Elements(size_t n) {
			mData.resize(n);
		}
		Elements(std::vector<T>& vec) {
			mData = vec;
		}
		virtual ~Elements() {}


		// access to elements.  Reference, so for setting and getting
		virtual inline T& operator[](const size_t index)  {
			// at() does bounds checking but not []
			// This returns a reference
			return mData.at(index);
		}





		//
		// copying in data
		// 

		//
		// Copying in scalars to all elements
		//

		// Set all elements to the input scalar
		template <class T2> Elements<T>& operator=(const T2 scalar) {
			std::fill( mData.begin(), mData.end(), scalar);
			return *this;
		}
		template <class T2> void set(const T2 scalar) {
			std::fill( mData.begin(), mData.end(), scalar);
		}
		Elements<T>& operator=(const std::string s) {
			T tmp;
			std::stringstream ss;
			ss << s;
			ss >> tmp;
			std::fill( mData.begin(), mData.end(), tmp);
			return *this;
		}
		void set(const std::string s) {
			T tmp;
			std::stringstream ss;
			ss << s;
			ss >> tmp;
			std::fill( mData.begin(), mData.end(), tmp);
		}



		// 
		// copying in std::vectors
		//

		// copy in same type as this vector
		Elements<T>& operator=(const std::vector<T> vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			mData = vec;
			return *this;
		}
		void set(const std::vector<T> vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			mData = vec;
		}
		// copy in vector of different type, but non-string type
		template <class T2> Elements<T>& operator=(const std::vector<T2> vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			std::copy( vec.begin(), vec.end(), mData.begin() );
			return *this;
		}
		template <class T2> void set(const std::vector<T2> vec) {
			if (vec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<vec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			std::copy( vec.begin(), vec.end(), mData.begin() );
		}
		// copy in vector string type
		Elements<T>& operator=(const std::vector<std::string> svec) {
			if (svec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<svec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << svec[i];
				ss >> mData[i];
			}
			return *this;
		}
		void set(const std::vector<std::string> svec) {
			if (svec.size() != mData.size()) {
				std::ostringstream err;
				err<<"Input vector length "<<svec.size()
					<<" does not match size of field "<<mData.size();
				throw std::runtime_error(err.str());
			}
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << svec[i];
				ss >> mData[i];
			}
		}



		//
		// Copying in Elements
		//
		template <class T2> Elements<T>& operator=(const Elements<T2>& el) {
			el.copy(mData);
			return *this;
		}
		template <class T2> void set(const Elements<T2>& el) {
			el.copy(mData);
		}







		//
		// Copy out 
		//

		// 
		// Vectors
		//
		template <class T2> void copy(std::vector<T2>& vec) const {
			vec.resize(mData.size());
			std::copy( mData.begin(), mData.end(), vec.begin() );
		}
		template <class T2> operator std::vector<T2>() { 
			std::vector<T2> tmp;
			copy(tmp);
			return tmp;
		}
		void copy(std::vector<std::string> & svec) const {
			svec.resize(mData.size());
			for (size_t i=0; i< mData.size(); i++) {
				std::stringstream ss;
				ss << mData[i];
				ss >> svec[i];
			}
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

		std::string str() const {
			std::stringstream os;
			write(os);
			std::string s = os.str();
			return s;
		}


	private:
		std::vector<T> mData;
};


template <class T>
inline std::ostream& operator<<( std::ostream& os, const Elements<T>& data)
{
	data.write(os);
	return os;
}

template <class T>
inline std::ostream& operator<<( std::ostream& os, const boost::numeric::ublas::vector<T>& data)
{

	if (0 == data.size()) {
		os<<"NULL";
	} else if (1 == data.size()) {
		os<<data[0];
	} else {
		os<<"{";
		for (size_t i=0;i<data.size(); i++) {
			if (i > 0) { os<<", "; }
			os<<data[i];
		}
		os<<"}";
	}

	return os;
}

inline std::ostream& operator<<( std::ostream& os, const boost::numeric::ublas::vector<string>& data)
{
	if (0 == data.size()) {
		os<<"NULL";
	} else if (1 == data.size()) {
		os<<"'"<<data[0]<<"'";
	} else {
		os<<"{";
		for (size_t i=0;i<data.size(); i++) {
			if (i > 0) { os<<", "; }
			os<<"'"<<data[i]<<"'";
		}
		os<<"}";
	}
	return os;
}



/*
template <> inline mydata::operator std::string() const
{
	std::ostringstream ss;
	ss << mData;
	return ss.str();
}
*/

int main(int argc, char **argv)
{

	boost::numeric::ublas::vector<double> bv(3);
	bv[0] = 10.25; bv[1] = 27.232; bv[2] = 883282.25;
	cout<<"bv = "<<bv<<endl;

	boost::numeric::ublas::vector<double> bv2(3);
	bv2[0] = 10.25; bv2[1] = 27.232; bv2[2] = 883282.25;
	cout<<"bv2 = "<<bv<<endl;

	bv += bv2;
	cout<<"bv += bv2 gives: "<<bv<<endl;

	bv = bv + bv2;
	cout<<"bv = bv + bv2 gives: "<<bv<<endl;


	boost::numeric::ublas::vector<string> bsvec(3);
	bsvec = bv;
	cout<<"bsvec = bv gives: "<<bsvec<<endl;








	return 0;
	Elements<double> del(3);
	Elements<float> fel(3);

	del[0] = 2.5; del[1] = 33.3; del[2] = -15.0;
	cout<<"del = "<<del<<endl;

	fel[0] = 3.3; fel[1] = 4.4; fel[2] = 5.5;
	cout<<"fel = "<<fel<<endl;


	del = 1.0;
	cout<<"del = 1 gives: "<<del<<endl;

	std::string two("2");
	del = two;
	cout<<"del = string('2') gives: "<<del<<endl;

	del = fel;
	cout<<"del = fel gives: "<<del<<endl;

	std::vector<std::string> svec = del;
	cout<<"svec = del gives: {'"<<svec[0]<<"', '"<<svec[1]<<"', '"<<svec[2]<<"'}"<<endl;




	return 0;
	DynamicStruct ds;


	// adding scalar fields.  That gets copied into a single element
	// vector internally
	ds.addfield("scalar_double", 2.5);
	cout<<"ds['scalar_double'] = "<<ds["scalar_double"]<<endl;

	ds.addfield("scalar_string", "hello");
	cout<<"ds['scalar_string'] = "<<ds["scalar_string"]<<endl;



	// Adding vector fields
	vector<float> tfloat(3);
	tfloat[0] = 1.2; tfloat[1]=8.3; tfloat[2]=-25.3;
	ds.addfield("testfloat", tfloat);
	cout<<"ds['testfloat']: "<<ds["testfloat"]<<endl;

	vector<double> tdouble(3);
	tdouble[0] = 33.3; tdouble[1]=88.8; tdouble[2]=99.9;
	ds.addfield("testdouble", tdouble);
	cout<<"ds['testdouble']: "<<ds["testdouble"]<<endl;

	vector<string> tstring(3);
	tstring[0] = "1 hello"; tstring[1]="2 world"; tstring[2]="3 goodbye";
	ds.addfield("teststring", tstring);
	cout<<"ds['teststring']: "<<ds["teststring"]<<endl;



	cout<<endl;


	// test copying out a scalar to a scalar.  
	double test_double;// = ds["scalar_double"];
	test_double = ds["scalar_double"];
	cout<<"test_double=ds['scalar_double'] gives "<<test_double<<endl;
	cout<<endl;


	// copy in and out scalar string
	std::string s="goodbye";
	ds["scalar_string"] = s;
	cout<<"ds['scalar_string'] = 'goodbye' gives: "
		<<ds["scalar_string"]<<endl;
	
	s="test copy";
	ds["scalar_string"]=s;
	s="";

	// must either construct or use .str() method
	s = ds["scalar_string"].str();
	std::string ss= ds["scalar_string"];
	cout<<"copy to s from 'scalar_string' gives: '"<<s<<"'"<<endl;
	cout<<"copy to ss from 'scalar_string' gives: '"<<ss<<"'"<<endl;

	cout<<endl;



	// Copying in and out an element of a non-string vector
	float tmp_float=0;
	tmp_float = ds["testfloat"][1];
	cout<<"ds['testfloat'][1]: "<<ds["testfloat"][1] <<endl;
	cout<<"setting tmp_float = ds['testfloat'] gives: "<< tmp_float <<endl;

	// Copying out an element
	float tmp_double = ds["testfloat"][1];
	cout<<"setting tmp_double = ds['testfloat'] gives: "<<tmp_double<<endl;

	// copying in an element
	tmp_float = -1234.1234;
	ds["testfloat"][1] = tmp_float;
	cout<<"Setting ds['testfloat'][1] = "<<tmp_float;
	cout<<" gives ds['testfloat'][1]: "<<ds["testfloat"][1] <<endl;

	cout<<endl;

	// copying in and out an element of a string vector 
	s = ds["teststring"][1].str();	
	cout<<"Copying out ds['teststring'][1].str() gives: '"<<s<<"'"<<endl;
	s="blah blah";
	ds["teststring"][1] = s;
	cout<<"Copying in '"<<s<<"' to element 1 gives: "<<ds["teststring"][1]<<endl;
	



	// copying in a scalar to all the elements of a vector
	ds["testfloat"] = -5.71;
	cout<<"copying -5.71 to elements of 'testfloat' gives: "
		<<ds["testfloat"]<<endl;

	// now for a string
	s="all same";
	// This works
	ds["teststring"] = s;
	cout<<"copying 'all same' string var using = to elements of 'teststring' gives: "
		<<ds["teststring"]<<endl;
	
	// this fails
	//ds["teststring"] = "same";
	ds["teststring"].set("same");
	cout<<"copying 'same' from bar const char* using set() to elements of 'teststring' gives: "
		<<ds["teststring"]<<endl;


	cout<<endl;

	// copying out vectors, with conversion, using the explicity copy()
	// method
	cout<<"Copying out vectors using copy()"<<endl;

	ds["testfloat"][0] = -1;ds["testfloat"][0] = -2;ds["testfloat"][0] = -3;
	// have to use the set method since c++ gets confused with = operators
	ds["teststring"][0].set("1 stuff");
	ds["teststring"][1].set("2 things");
	ds["teststring"][2].set("3 blah");
	std::vector<float> fcopy;
	std::vector<double> dcopy;
	std::vector<string> scopy;
	ds.copy("testfloat", fcopy);
	ds["testfloat"].copy(dcopy);
	ds["testfloat"].copy(scopy);

	cout<<"Using copy() to copy out vectors"<<endl;
	for (size_t i=0; i<tfloat.size(); i++) {
		cout
			<<"  testfloat: "<<(float) ds["testfloat"][i]
			<<" fcopy: "<<fcopy[i]
			<<" dcopy: "<<dcopy[i]
			<<" scopy: "<<scopy[i]
			<<endl;
	}


	ds.copy("teststring", scopy);
	// this one requires a number at the front of the strings
	ds.copy("teststring", fcopy);

	for (size_t i=0; i<tstring.size(); i++) {
		cout
			<<" scopy:\t"<<scopy[i]
			<<"\t\tfcopy:\t"<<fcopy[i]
			<<endl;
	}

	cout<<endl;

	// copying out full vector with conversion, using the [] method
	std::vector<double> dset = ds["testfloat"];
	cout<<"Doing copy on construction: "
		<<"vector<double> dset=ds['testfloat'] gives: {";
	for (size_t i=0;i<dset.size();i++) {
		if (i != 0) cout<<", ";
		cout<<dset[i];
	}
	cout<<"}\n";

	vector<double> dset2;
	dset2 = ds["testfloat"];
	cout<<"Doing copy after construction: "
		<<"dset2 = ds['testfloat'] gives: {";
	for (size_t i=0;i<dset2.size();i++) {
		if (i != 0) cout<<", ";
		cout<<dset2[i];
	}
	cout<<"}\n";


	cout<<endl;

	// Copying in vectors using the [] method
	tfloat[0] = -1.5; tfloat[1]=-2.5; tfloat[2]=-3.5;
	ds["testfloat"] = tfloat;
	cout<<"Copying ds['testfloat']=tfloat gives "<<ds["testfloat"]<<endl;

	tstring[0] = "-30 test0"; tstring[1]="-20 test"; tstring[2]="-10 stuff";
	ds["teststring"] = tstring;
	cout<<"Copying ds['teststring']=tstring gives "<<ds["teststring"]<<endl;
	cout<<endl;

	return 0;

	// copying between DynamicFields
	ds["testfloat"] = ds["testdouble"];
	cout<<"Copying ds['testfloat']=ds['testdouble'] gives "<<ds["testfloat"]<<endl;
	ds["teststring"] = ds["testdouble"];
	cout<<"Copying ds['teststring']=ds['testdouble'] gives "<<ds["teststring"]<<endl;

	ds["testfloat"] /= ds["testdouble"];
	cout<<"Copying ds['testfloat']/=ds['testdouble'] gives "<<ds["testfloat"]<<endl;
	ds["testfloat"] *= ds["testdouble"];
	cout<<"Copying ds['testfloat']*=ds['testdouble'] gives "<<ds["testfloat"]<<endl;

	cout<<endl;

	// some arithmetic operators on the full vector fields
	ds["testfloat"] *= tfloat;
	cout<<"ds['testfloat'] *= tfloat: "<<ds["testfloat"]<<endl;

	ds["testfloat"] /= tfloat;
	cout<<"ds['testfloat'] /= tfloat: "<<ds["testfloat"]<<endl;


	cout<<endl;


	// resetting the vector, including resize
	tfloat.resize(4);
	tfloat[0] = -1.5; tfloat[1]=-2.5; tfloat[2]=-3.5;; tfloat[3] = -4.5;
	ds["testfloat"].reset(tfloat);
	cout<<"Resetting ds['testfloat'].reset(tfloat) gives "<<ds["testfloat"]<<endl;

	ds["testfloat"].reset(ds["testdouble"]);
	cout<<"Resetting ds['testfloat'].reset(ds['testdouble']) gives "<<ds["testfloat"]<<endl;







	return 0;
	int n=10000000;
	vector<double> bigvec(n, 1);
	ds.addfield("bigdouble", bigvec);

	double scalar=3.5;
	if (0) {
		//ds.multiply("bigdouble", scalar);
		ds["bigdouble"] *= scalar;
	} else {
		for (int i=0; i<n; i++) {
			bigvec[i] *= scalar;
			//double d = ds["bigdouble"][i];
			//double d = ds.get<double>("bigdouble",i);
		}
	}

	/*
	return 0;
	
	std::vector<int> x(3, int());

	test(int());
	test(double());


	//FieldBase* fb = new Field<double>(25); 
	FieldBase* fb = new Field<int>(25); 
	cout<<"fb->type_name(): "<<fb->type_name()<<endl;
	*/
}
