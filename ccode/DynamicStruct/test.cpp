#include <iostream>
#include "DynamicStruct.h"

#include <valarray>
using std::valarray;


template <class T> void print_vec(const vector<T>& vec, const char* name) {

	for (size_t i=0; i<vec.size(); i++) {
		cout<<"\t"<<name<<"["<<i<<"] = "<<vec[i]<<endl;
	}
}

template <class T> void vecinit(vector<T>& vec) {
	for (size_t i=0; i<vec.size(); i++) {
		vec[i] = i;
	}
}

template <class T> valarray<T> retvalarray() {
	valarray<T> va(5);
	for (size_t i=0; i<va.size(); i++) {
		va[i] = i;
	}
	return va;
}


int main(int argc, char **argv)
{


	// Some vector and scalar data to store in the dynamic structure
	string s="hello world";

	int num=5;
	vector<float> vf1(num);
	vector<float> vf2(num);
	vector<int> vf3(num);
	for (int i=0; i< vf1.size(); i++) {
		vf1[i] = i+0.1;
		vf2[i] = i+1;
		vf3[i] = (i+1)*10;
	}

	vector<string> svec(3);
	svec[0] = "3 stuff and things";
	svec[1] = "12 this is a longer string for testing";
	svec[2] = "8 things";

	//vector<string> svec_cpy( svec.begin()+1, svec.begin()+3 );
	//print_vec(svec_cpy, "svec_cpy");
	//return 0;

	std::DynamicStruct ds2;
	double ds2_d=221.2;
	ds2.addfield("dfield", ds2_d);
	ds2.addfield("vf1", vf1);
	ds2.addfield("string", s);
	ds2.addfield("stringvec", svec);

	print_vec(vf1,"vf1");

	cout<<"Adding uint16_fromstring"<<endl;
	ds2.addfield("uint16_fromstring", "uint16", 5);
	cout<<"Getting ref to uint16_fromstring"<<endl;
	vector<uint16>& uint16_fromstring = ds2["uint16_fromstring"];
	uint16_fromstring[0] = 10;
	uint16_fromstring[1] = 20;
	uint16_fromstring[2] = 30;
	uint16_fromstring[3] = 40;
	uint16_fromstring[4] = 50;

	vector<int32> uint16_as_int32;
	ds2["uint16_fromstring"].copy(uint16_as_int32);
	cout<<"uint16_as_int32 size = "<<uint16_as_int32.size()<<endl;
	print_vec(uint16_as_int32,"fromstring as int32");

	double& dfield_ref = ds2["dfield"];
	cout<<"dfield ref: "<<dfield_ref<<endl;
	dfield_ref = -0.7532;
	cout<<"Set dfield as "<<dfield_ref<<" through reference\n";
	double dfield_copy = ds2["dfield"];
	cout<<"now lookign at dfield copy: "<<dfield_copy<<endl;
	float dfield_asfloat = (float) ds2["dfield"];
	cout<<"dfield as float: "<<dfield_asfloat<<endl;

	string sfield = ds2["string"];
	cout<<"string field: "<<sfield<<endl;





	vector<string> svec_copy = ds2["stringvec"].copy<string>();
	print_vec(svec_copy, "svec_copy");
	vector<string>& svec_ref = ds2["stringvec"];
	print_vec(svec_ref, "svec_ref");
	vector<int> svec_copy_int = ds2["stringvec"].copy<int>();
	print_vec(svec_copy_int, "svec_copy_int");




	//vector<float>& dref = (vector<float>&) ds2["vf1"];
	vector<float>& fref = ds2["vf1"];
	print_vec(fref, "fref");

	fref[2] = -1;
	vector<float> fcopy = ds2["vf1"].copy<float>();

	vector<float> fcopy2;
	ds2["vf1"].copy(fcopy2);

	//vector<float64> dcopy = ds2["vf1"].copy<float64>();
	vector<float64> dcopy;
	ds2["vf1"].copy(dcopy);

	//vector<float64> dcopy_sub;
	//ds2["vf1"].copy(dcopy_sub, 2, 2);
	vector<float64> dcopy_sub = ds2["vf1"].copy<float64>(2, 2);


	print_vec(fref, "fref");
	print_vec(fcopy, "fcopy");

	//float tf = (float) ds2["vf1"][1];
	//cout<<"ds2[\"vf1\"][1] = "<<

	print_vec(fcopy2, "fcopy2");
	print_vec(dcopy, "dcopy");
	print_vec(dcopy_sub, "dcopy sub");

}
