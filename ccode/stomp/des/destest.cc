#include "des_stomp.h"
#include <sys/stat.h>

using namespace std;

int main() {

	mkdir("test_output",0777);

	// Some examples of working with des footprints

	// create an angular coordinate object
	double cenra=0.0, cendec=0.0;
	AngularCoordinate ang(cenra, cendec,AngularCoordinate::Equatorial);

	// And a DESFootprint at the location given by ang, with a weight of 1.0
	cout<<"\nCreating a DESFootprint at location: "<<ang.Repr()<<endl;
	DESFootprint des(ang,1.0);

	std::cout << "\tDES footprint area: " << des.Area() << "\n";

	// The maximum resolution for the resulting stomp map
	int hres = 2048;
	std::cout <<
		"\tpixelizing with a max resolution of "<<hres<<endl;
	des.SetMaxResolution(hres);

	// Pixelizing the internal analytic representation of the DESFootprint,
	// which is in terms of polygons
	std::cout << "\t\tStarting resolution: " <<
		des.FindStartingResolution() << "\n";
	if (des.Pixelize()) {
		std::cout << "\t\tPixelization success!\n\t\tArea Check: Real: " <<
			des.Area() << ", Pixelized: " << des.PixelizedArea() << "\n";
	} else {
		std::cout << "\t\tPixelization failed...\n";
	}

	// Make a stomp map from this pixelization.
	StompMap des_smap; 
	des.StompMap(des_smap);

	// Write it out to a file in the simplest format: pixel_index  resolution
	string fname;
	fname="test_output/des_footprint.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap.Write(fname, false, false);



	// Now create a stomp map representing a disk and exclude it from
	// the des map

	// Choose the central location somewhat offset from the des footprint
	// center
	AngularCoordinate diskang(cenra+0.1,cendec+0.1,
			AngularCoordinate::Equatorial);

	// Create a single pixel at the angular position with resolution hres
	StompPixel centerpixel(diskang, hres);

	// This will hold the disk pixels
	StompVector diskpix;

	// The angular extent of the disk in degres
	double theta = 0.2;
	// This generates a list of pixels within radius theta of the position
	// ang
	centerpixel.WithinRadius(theta,diskpix);

	// Now make a StompMap from that
	StompMap diskmap(diskpix);

	// Now exclude the disk from the des footprint
	cout<<"\nExcluding a disk at: "<<diskang.Repr()<<" radius: "<<theta<<endl;
	cout<<"\tArea before exclusioin: "<<des_smap.Area()<<" sq. deg.\n";
	cout<<"\tArea of disk: "<<diskmap.Area()<<endl;
	if (des_smap.ExcludeMap(diskmap,false)) {
		cout << "\t\tArea remaining after excluding disk: " <<
			des_smap.Area()<<endl;
	} else {
		cout << "\t\tThis is bad," <<
			" there should have been some area left over.\n";
	}


	fname="test_output/des_footprint_minusdisk.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap.Write(fname, false, false);


	// Now exclude a polygon

	// Create vertices, must move clockwise if looking from outside the 
	// sphere
	AngularVector polyvec;

	// Relative to a (0,0) coordinate system
	double ramin = -0.1;
	double ramax =  0.1;
	double decmin = -0.1;
	double decmax =  0.1;

	double poly_cenra = 0.0-0.3;
	double poly_cendec = 0.0-0.3;
	double tmp_ra, tmp_dec;

	tmp_ra = poly_cenra + ramin*AngularCoordinate::RAMultiplier(decmax);
	tmp_dec = poly_cendec + decmax;
	polyvec.push_back(AngularCoordinate(tmp_ra,tmp_dec,
				AngularCoordinate::Equatorial));

	tmp_ra = poly_cenra + ramin*AngularCoordinate::RAMultiplier(decmin);
	tmp_dec = poly_cendec + decmin;
	polyvec.push_back(AngularCoordinate(tmp_ra,tmp_dec,
				AngularCoordinate::Equatorial));

	tmp_ra = poly_cenra + ramax*AngularCoordinate::RAMultiplier(decmin);
	tmp_dec = poly_cendec + decmin;
	polyvec.push_back(AngularCoordinate(tmp_ra,tmp_dec,
				AngularCoordinate::Equatorial));

	tmp_ra = poly_cenra + ramax*AngularCoordinate::RAMultiplier(decmax);
	tmp_dec = poly_cendec + decmax;
	polyvec.push_back(AngularCoordinate(tmp_ra,tmp_dec,
				AngularCoordinate::Equatorial));


	cout<<"\nExcluding polygon at ("<<poly_cenra<<", "<<poly_cendec<<")"<<endl;
	PolygonBound poly(polyvec, 1.0);

	poly.SetMaxResolution(hres);

	std::cout << "\tPixelizing polygon: Starting resolution: " <<
		poly.FindStartingResolution() << "\n";
	if (poly.Pixelize()) {
		std::cout << "\t\tPixelization success!\n\t\tArea Check: Real: " <<
			poly.Area() << ", Pixelized: " << poly.PixelizedArea() << "\n";
	} else {
		std::cout << "\t\tPixelization failed...\n";
	}


	// Now exclude the polygon from the des footprint

	//StompMap *polymap = poly.StompMap();
	StompMap polymap;
	poly.StompMap(polymap);

	cout<<"\tArea before exclusioin: "<<des_smap.Area()<<" sq. deg.\n";
	cout<<"\tArea of disk: "<<polymap.Area()<<endl;
	if (des_smap.ExcludeMap(polymap,false)) {
		cout << "\t\tArea remaining after excluding disk: " <<
			des_smap.Area()<<endl;
	} else {
		cout << "\t\tThis is bad," <<
			" there should have been some area left over.\n";
	}


	fname="test_output/des_footprint_minusdiskpoly.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap.Write(fname, false, false);





	// Now create another disk and exclude it from the des map as well, leaving
	// an odd geometry

	diskang.SetEquatorialCoordinates(cenra-0.1, cendec-0.1);
	centerpixel.SetPixnumFromAng(diskang);

	theta = 0.25;
	centerpixel.WithinRadius(theta,diskpix);
	diskmap.Initialize(diskpix);
	cout<<"\nExcluding a second disk at: "<<diskang.Repr()<<" radius: "<<theta<<endl;
	cout<<"\tArea before exclusioin: "<<des_smap.Area()<<" sq. deg.\n";
	cout<<"\tArea of disk: "<<diskmap.Area()<<endl;
	if (des_smap.ExcludeMap(diskmap,false)) {
		cout << "\t\tArea remaining after excluding disk: " <<
			des_smap.Area()<<endl;
	} else {
		cout << "\t\tThis is bad," <<
			" there should have been some area left over.\n";
	}


	fname="test_output/des_footprint_minusdiskpoly2.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap.Write(fname, false, false);



	// Generate some random points from this map and write them out
	AngularVector randvec;
	int nrand=10000;
	cout<<endl<<"Generating random points\n";
	des_smap.GenerateRandomPoints(randvec, nrand);

	fname="test_output/des_footprint_minusdiskpoly2_rand.dat";
	cout<<"\nWriting random points to file: "<<fname<<endl;
	ofstream ofile;
	ofile.open(fname.c_str());

	for (unsigned int i=0; i<randvec.size(); i++) {
		ofile<<randvec[i].RA()<<" "<<randvec[i].DEC()<<endl;
	}
	ofile.close();




	// Add another full DES footprint nearby
	cout<<"\n\nAdding another footprint\n";
	AngularCoordinate ang2(0.5,1.0,AngularCoordinate::Equatorial);

	DESFootprint des2(ang2,1.0);

	std::cout <<
		"\tpixelizing with a max resolution of "<<hres<<endl;

	des2.SetMaxResolution(hres);

	std::cout << "\tStarting resolution: " <<
		des2.FindStartingResolution() << "\n";
	if (des2.Pixelize()) {
		std::cout << "\t\tPixelization success!\n\t\tArea Check: Real: " <<
			des2.Area() << ", Pixelized: " << des2.PixelizedArea() << "\n";
	} else {
		std::cout << "\t\tPixelization failed...\n";
	}


	// make a stomp map and write it to disk
	StompMap des_smap2;
	des2.StompMap(des_smap2);

	fname="test_output/des_extrafoot.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap2.Write(fname, false, false);

	// Add the maps. False means keep the areas not overlapping 
	des_smap.AddMap(des_smap2, false);

	fname="test_output/des_twofeet.dat";
	cout<<"Writing to file: "<<fname<<endl;
	des_smap.Write(fname, false, false);

	// generate some random points from this combined map not accounting
	// for the weights, just uniformly over the area
	randvec.clear();
	nrand = 20000;

	cout<<"\nGenerating random points\n";
	des_smap.GenerateRandomPoints(randvec, nrand);

	fname = "test_output/des_twofeet_rand.dat";
	cout<<"Writing random points to file: "<<fname<<endl;
	ofile.open(fname.c_str());

	for (unsigned int i=0; i<randvec.size(); i++) {
		ofile<<randvec[i].RA()<<" "<<randvec[i].DEC()<<endl;
	}
	ofile.close();


	// Now generate some randoms accouting for the weights: In our case this
	// means twice the density in the areas of overlap since we used weight
	// of 1.0 for both maps
	cout<<"\nGenerating weighted random points\n";
	des_smap.GenerateRandomPoints(randvec, nrand, true);

	fname = "test_output/des_twofeet_rand_weighted.dat";
	cout<<"Writing random points to file: "<<fname<<endl;
	ofile.open(fname.c_str());

	for (unsigned int i=0; i<randvec.size(); i++) {
		ofile<<randvec[i].RA()<<" "<<randvec[i].DEC()<<endl;
	}
	ofile.close();


	return 0;
}
