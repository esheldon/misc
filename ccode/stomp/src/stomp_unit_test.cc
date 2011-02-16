#include "stomp_util.h"

using namespace std;

int main() {

  // First things first.  Let's check to make sure that our constants are
  // all in place.

  cout << "STOMP Constants check:\n";
  cout << "\tpi: " << Stomp::Pi() <<
      "\t\t\tdeg2Rad: " << Stomp::Deg2Rad() << "\n";
  cout << "\trad2Deg: " << Stomp::Rad2Deg() <<
      "\t\tstrad2Deg: " << Stomp::Strad2Deg() << "\n";
  cout << "\tnx0: " << Stomp::Nx0() <<
      "\t\t\t\tny0: " << Stomp::Ny0() << "\n";
  cout << "\tetaOffSet: " << Stomp::EtaOffSet() <<
      "\t\tsurveyCenterRA: " << Stomp::SurveyCenterRA() << "\n";
  cout << "\tsurveyCenterDEC: " << Stomp::SurveyCenterDEC() <<
      "\t\tnode: " << Stomp::Node() << "\n";
  cout << "\tetaPole: " << Stomp::EtaPole() <<
      "\t\thpix_resolution: " << Stomp::HPixResolution() << "\n";
  cout << "\tmax_resolution: " << Stomp::MaxPixelResolution() <<
      "\t\thpix_area: " << Stomp::HPixArea() << "\n";
  cout << "\tmax_pixnum: " << Stomp::MaxPixnum() <<
      "\t\tmax_superpixnum: " << Stomp::MaxSuperpixnum() << "\n";


  // Ok, some basic routines.  We're going to declare an angular
  // position, convert it from Survey to Equatorial to Galactic coordinates
  // and make sure that it gives us the same pixel position regardless.

  cout << "\nAngular transforms check:\n";

  double lambda, eta, ra, dec, gal_lat, gal_lon;

  lambda = 10.0;
  eta = 10.0;

  AngularCoordinate ang(lambda,eta,AngularCoordinate::Survey);

  StompPixel tmp_pix(ang,256);

  cout << "\tLambda: " << ang.Lambda() << ", Eta: " << ang.Eta() << "; " <<
      tmp_pix.HPixnum() << " " << tmp_pix.Superpixnum() << "\n";

  AngularCoordinate::SurveyToEquatorial(lambda,eta,ra,dec);

  cout << "\tRA: " << ang.RA() << " (" << ra << "), " <<
      "DEC: " << ang.DEC() <<  " (" << dec << "); ";

  tmp_pix.SetPixnumFromAng(ang);

  cout << tmp_pix.HPixnum() << " " << tmp_pix.Superpixnum() << "\n";

  AngularCoordinate::SurveyToGalactic(lambda,eta,gal_lat,gal_lon);

  cout << "\tGalLat: " << ang.GalLat() << " (" << gal_lat << "), "
      "GalLon: " << ang.GalLon() <<  " (" << gal_lon << "); ";

  tmp_pix.SetPixnumFromAng(ang);

  cout << tmp_pix.HPixnum() << " " << tmp_pix.Superpixnum() << "\n";


  // Ok, now we'll try moving around a bit in resolution space.

  cout << "\nPixel Resolution scaling:\n";
  cout << "Setting index at each step manually:\n";
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetResolution(resolution);
    tmp_pix.SetPixnumFromAng(ang);
    cout << "\tHPixnum, Superpixnum, Resolution: " <<
        tmp_pix.HPixnum() << ", " << tmp_pix.Superpixnum() <<
        ", " << tmp_pix.Resolution() << "\n";
  }
  cout << "Now scaling with the Superpix function:\n";
  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);

  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);
    cout << "\tHPixnum, Superpixnum, Resolution: " <<
        tmp_pix.HPixnum() << ", " << tmp_pix.Superpixnum() <<
        ", " << tmp_pix.Resolution() << "\n";
  }

  //  Ok, now we go the other way.

  cout << "\nSub-pixel index checking:\n";

  tmp_pix.SetResolution(Stomp::HPixResolution());
  tmp_pix.SetPixnumFromAng(ang);

  for (int resolution=Stomp::HPixResolution();
       resolution<=Stomp::MaxPixelResolution();resolution*=2) {
    unsigned long x_min, x_max, y_min, y_max, n_pixel;
    tmp_pix.SubPix(resolution,x_min,x_max,y_min,y_max);
    n_pixel = (x_max - x_min + 1)*(y_max - y_min + 1);
    cout << "\t" << resolution <<
        ", X: " << x_min << " - " << x_max <<
        ", Y: " << y_min << " - " << y_max <<
        ", " << n_pixel << " pixels\n";
  }

  // Now, we'll do some spherical geometry tests.  First up, we check the
  // multi-resolution stripe function.

  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);

  cout << "\nStripe test:\n";
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    cout << "\tResolution " << resolution << ": Stripe = " <<
        tmp_pix.Stripe(resolution) << "\n";
  }

  // Now, the XY bounds for a given angular range around a given point.  There
  // are two things to note here.  First, finding all of the pixels within
  // a given angular radius means looking at many more pixels at high
  // latitude than at low latitude.  Second, if we use the flexible box version
  // of XYBounds (where the x bounds are calculated separately for each value
  // of y), then the number of pixels that we'd examine is much smaller at
  // high latitudes.  Hence, even though this version of XYBounds is slower,
  // you may save time (and memory) on the back end if you have to do
  // several operations on the pixels you get back.

  cout << "\nX-Y bounds test:\n";

  double theta = 10.0;  //  Look for all pixels within 10 degrees.

  cout << "Low latitude (Lambda = 0.0), fixed box\n";

  ang.SetSurveyCoordinates(0.0,0.0);
  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long x_min, x_max, y_min, y_max, n_pixel;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = (x_max - x_min + 1)*(y_max - y_min + 1);
    cout << "\t" << resolution <<
        ", X: " << x_min << " - " << x_max <<
        ", Y: " << y_min << " - " << y_max <<
        ", " << n_pixel << " pixels\n";
  }

  cout << "High latitude (Lambda = 60.0), fixed box\n";

  ang.SetSurveyCoordinates(60.0,0.0);
  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long x_min, x_max, y_min, y_max, n_pixel;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = (x_max - x_min + 1)*(y_max - y_min + 1);
    cout << "\t" << resolution <<
        ", X: " << x_min << " - " << x_max <<
        ", Y: " << y_min << " - " << y_max <<
        ", " << n_pixel << " pixels\n";
  }

  cout << "Low latitude (Lambda = 0.0), flexible box\n";

  ang.SetSurveyCoordinates(0.0,0.0);
  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long y_min, y_max, n_pixel;
    vector<unsigned long> x_min, x_max;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = 0;
    for (unsigned long y=y_min,m=0;y<=y_max;y++,m++)
      n_pixel += x_max[m] - x_min[m] + 1;
    cout << "\t" << resolution <<
        ", Y: " << y_min << " - " << y_max <<
        ", " << n_pixel << " pixels\n";
  }

  cout << "High latitude (Lambda = 60.0), flexible box\n";

  ang.SetSurveyCoordinates(60.0,0.0);
  tmp_pix.SetResolution(Stomp::MaxPixelResolution());
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=Stomp::MaxPixelResolution();
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long y_min, y_max, n_pixel;
    vector<unsigned long> x_min, x_max;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = 0;
    for (unsigned long y=y_min,m=0;y<=y_max;y++,m++)
      n_pixel += x_max[m] - x_min[m] + 1;
    cout << "\t" << resolution <<
        ", Y: " << y_min << " - " << y_max <<
        ", " << n_pixel << " pixels\n";
  }

  // Now we check our WithinRadius routine.  We'll stick with a 10 degree
  // radius, but cut the maximum resolution to check to 256 to save time.

  cout << "\nReturning pixels within an angular radius test:\n";
  cout << "Low latitude (Lambda = 0.0), flexible box\n";

  ang.SetSurveyCoordinates(0.0,0.0);
  tmp_pix.SetResolution(128);
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=128;
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long y_min, y_max, n_pixel;
    vector<unsigned long> x_min, x_max;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = 0;
    for (unsigned long y=y_min,m=0;y<=y_max;y++,m++)
      n_pixel += x_max[m] - x_min[m] + 1;

    StompVector annulus_pix;
    tmp_pix.WithinRadius(theta,annulus_pix);
    cout << "\t" << resolution <<
        ", Checked " << n_pixel << " pixels, kept " <<
        annulus_pix.size() << "\n";
  }

  cout << "High latitude (Lambda = 60.0), flexible box\n";

  ang.SetSurveyCoordinates(60.0,0.0);
  tmp_pix.SetResolution(128);
  tmp_pix.SetPixnumFromAng(ang);
  for (int resolution=128;
       resolution>=Stomp::HPixResolution();resolution/=2) {
    tmp_pix.SetToSuperPix(resolution);

    unsigned long y_min, y_max, n_pixel;
    vector<unsigned long> x_min, x_max;
    tmp_pix.XYBounds(theta,x_min,x_max,y_min,y_max,true);
    n_pixel = 0;
    for (unsigned long y=y_min,m=0;y<=y_max;y++,m++)
      n_pixel += x_max[m] - x_min[m] + 1;

    StompVector annulus_pix;
    tmp_pix.WithinRadius(theta,annulus_pix);
    cout << "\t" << resolution <<
        ", Checked " << n_pixel << " pixels, kept " <<
        annulus_pix.size() << "\n";
  }

  // Ok, now we're ready to start playing with the StompMap interfaces.  We'll
  // start by making a vector of pixels at the same resolution that cover a
  // patch of sky.  Then, we invoke the StompMap constructor, which should
  // resolve that vector into a smaller group of pixels at different
  // resolutions that cover the same area.

  theta = 3.0;
  double true_circle_area =
      (1.0 - cos(theta*Stomp::Deg2Rad()))*2.0*Stomp::Pi()*Stomp::Strad2Deg();
  ang.SetSurveyCoordinates(60.0,0.0);
  tmp_pix.SetResolution(256);
  tmp_pix.SetPixnumFromAng(ang);

  StompVector annulus_pix;
  tmp_pix.WithinRadius(theta,annulus_pix);

  unsigned long original_annulus_size = annulus_pix.size();
  cout << "\nStompMap routines\n";
  StompMap* stomp_map = new StompMap(annulus_pix);
  cout << "\tMade StompMap with " << stomp_map->Size() <<
      " pixels from initial vector of " <<
      original_annulus_size << " pixels.\n";

  string output_file_name = "StompMap.pix";

  cout << "\t Writing StompMap to " << output_file_name.c_str() << "\n";
  stomp_map->Write(output_file_name);

  StompVector superpix;
  stomp_map->Coverage(superpix);
  cout << "\tStompMap covers " << superpix.size() << " superpixels:\n";
  for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    unsigned long k = iter->Superpixnum();
    cout << "\t\t" << k << ": " << stomp_map->Area(k) << " sq. degrees, " <<
      stomp_map->MinResolution(k) << " <= resolution <= " <<
      stomp_map->MaxResolution(k) << "\n";
  }

  cout << "\tTotal Area: " << stomp_map->Area() << " square degrees.\n";
  cout << "\tOriginal Area: " << tmp_pix.Area()*original_annulus_size <<
      " square degrees.\n";
  cout << "\tTrue Annulus Area: " << true_circle_area << " square degrees.\n";

  // We won't match the true area since we're using relatively coarse pixels,
  // but it should be within a tenth of a square degree or so.

  // Ok, now we'll check some points that may or may not be in the map.

  cout << "\nFind location tests:\n";

  ang.SetSurveyCoordinates(60.0,0.0);
  double weight = 0.0;

  if (stomp_map->FindLocation(ang,weight)) {
    cout << "\tGood. Found point at (60,0) which was the center of the map\n";
    cout << "\tThe weight here is " << weight << " (1.0).\n";
  } else {
    cout << "\tNot good. That point (60,0) was the center of the map\n";
  }

  ang.SetSurveyCoordinates(0.0,0.0);

  if (stomp_map->FindLocation(ang,weight)) {
    cout << "\tNot good. That point (0,0) was well outside the map\n";
  } else {
    cout << "\tGood. That point (0,0) was well outside the map\n";
    cout << "\t\tThe weight here is " << weight << " (1.0).\n";
  }

  ang.SetSurveyCoordinates(59.0,1.0);

  if (stomp_map->FindLocation(ang,weight)) {
    cout << "\tGood. Found point at (59,1) which was within the map\n";
    cout << "\t\tThe weight here is " << weight << " (1.0).\n";
  } else {
    cout << "\tNot good. That point (59,1) was within the map\n";
  }

  ang.SetSurveyCoordinates(60.0,10.0);

  if (stomp_map->FindLocation(ang,weight)) {
    cout << "\tNot good. That point (60,10) was outside the map\n";
  } else {
    cout << "\tGood. That point (60,10) was well outside the map\n";
    cout << "\t\tThe weight here is " << weight << " (1.0).\n";
  }

  // Ok, now to test the unmasked fraction code.  We'll scroll through the
  // the superpixels and make sure that the area returned matches the area
  // we've already established for them.

  cout << "\nUnmasked area check:\n";

  for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    double unmasked_fraction = stomp_map->FindUnmaskedFraction(*iter);
    cout << "\t" << iter->Superpixnum() << ": " <<
        unmasked_fraction*Stomp::HPixArea() << " (" <<
        stomp_map->Area(iter->Superpixnum()) << ") square degrees.\n";
  }

  // Alright, now we check the random position generator.  This should give
  // us back a fixed number of randomly selected positions within our original
  // map, so we'll check that that's happening.

  cout << "\nRandom position test:\n";

  unsigned long n_random = 10000;

  cout << "\tGenerating " << n_random << " points...\n";

  AngularVector rand_ang;

  stomp_map->GenerateRandomPoints(rand_ang,n_random);

  if (rand_ang.size() != n_random) {
    cout << "\t\tRandom point array size doesn't match requested size!\n";
    exit(1);
  }

  unsigned long n_found = 0;
  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (stomp_map->FindLocation(*iter,weight)) n_found++;

  cout << "\tVerified that " << n_found << "/" << n_random <<
      " points within map.\n";

  output_file_name = "RandomPoints.dat";
  cout << "\tWriting random points to " << output_file_name.c_str() << "\n";

  ofstream outputFile(output_file_name.c_str());

  if (outputFile.is_open()) {
    for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
      outputFile << iter->Lambda() << " " << iter->Eta() << "\n";

    outputFile.close();
  } else {
    cout << "Failed to open " << output_file_name.c_str() << "\n";
  }

  // Ok, now we want to test the various routines for working with multiple
  // stomp maps.  We'll use the same basic routines to generate two new maps,
  // one that should partially overlap our original map and one that is well
  // away from the other two.


  ang.SetSurveyCoordinates(62.0,2.0);
  tmp_pix.SetResolution(256);
  tmp_pix.SetPixnumFromAng(ang);

  tmp_pix.WithinRadius(theta,annulus_pix);

  StompMap* stomp_map_nearby = new StompMap(annulus_pix);


  ang.SetSurveyCoordinates(0.0,0.0);
  tmp_pix.SetResolution(256);
  tmp_pix.SetPixnumFromAng(ang);

  tmp_pix.WithinRadius(theta,annulus_pix);

  StompMap* stomp_map_faraway = new StompMap(annulus_pix);

  // Now we'll check the random points from our original map against the two
  // new maps and see how much overlap we've got.

  cout << "Multi-map tests:\n";

  cout << "\tTesting against random positions from original map:\n";
  n_found = 0;
  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (stomp_map_nearby->FindLocation(*iter,weight)) n_found++;

  cout << "\t\tFound " << n_found << "/" << n_random <<
      " points within nearby map.\n";

  n_found = 0;
  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (stomp_map_faraway->FindLocation(*iter,weight)) n_found++;

  cout << "\t\tFound " << n_found << "/" << n_random <<
      " points within far away map.\n";


  // Now, we'll test the routines for doing logical AND, OR and NOT with the
  // various maps.

  cout << "\tIngestion test:\n";

  ang.SetSurveyCoordinates(60.0,0.0);
  tmp_pix.SetResolution(256);
  tmp_pix.SetPixnumFromAng(ang);

  tmp_pix.WithinRadius(theta,annulus_pix);

  StompMap* tmp_map = new StompMap(annulus_pix);

  tmp_map->IngestMap(*stomp_map_nearby,false);

  cout << "\t\tOriginal map area: " << stomp_map->Area() << " sq. degrees.\n";
  cout << "\t\tNearby: Adding " << stomp_map_nearby->Area() <<
      " sq. degree map to original map\n";
  cout << "\t\t\tNew Map: " << tmp_map->Area() << " sq. degrees.\n";

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);

  tmp_map->IngestMap(*stomp_map_faraway,false);

  cout << "\t\tFar Away: Adding " << stomp_map_faraway->Area() <<
      " sq. degree map to original map.\n";
  cout << "\t\t\tNew Map: " << tmp_map->Area() << " (" <<
      stomp_map->Area()+stomp_map_faraway->Area() << ") sq. degrees.\n";

  delete tmp_map;



  cout << "\tIntersection:\n";

  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->IntersectMap(*stomp_map_nearby)) {
    cout << "\t\tNearby intersection area: " <<
        tmp_map->Area() << " sq. degrees.\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some intersecting area.\n";
  }

  // Quick test of our weighted random point generating.

  tmp_map->GenerateRandomPoints(rand_ang,n_random);

  if (rand_ang.size() != n_random) {
    cout << "\t\tRandom point array size doesn't match requested size!\n";
    exit(1);
  }

  n_found = 0;
  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (tmp_map->FindLocation(*iter,weight)) n_found++;

  cout << "\tVerified that " << n_found << "/" << n_random <<
      " points within map.\n";

  output_file_name = "RandomPointsIntersect.dat";
  cout << "\tWriting random points to " << output_file_name.c_str() << "\n";

  ofstream outputFile3(output_file_name.c_str());

  if (outputFile3.is_open()) {
    for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
      outputFile3 << iter->Lambda() << " " << iter->Eta() << "\n";

    outputFile3.close();
  } else {
    cout << "Failed to open " << output_file_name.c_str() << "\n";
  }

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->IntersectMap(*stomp_map_faraway)) {
    cout << "\t\tBad. Far away intersection area: " <<
        tmp_map->Area() << " sq. degrees.  Should be 0 and leave unchanged.\n";
  } else {
    cout << "\t\tGood, no intersecting area as expected for Far Away map.\n";
  }

  delete tmp_map;



  cout << "\tExclusion:\n";

  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->ExcludeMap(*stomp_map_nearby,false)) {
    cout << "\t\tArea remaining after excluding nearby map: " <<
        tmp_map->Area() << " sq. degrees.\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some area left over.\n";
  }

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->ExcludeMap(*stomp_map_faraway,false)) {
    cout << "\t\tArea remaining after excluding faraway map: " <<
        tmp_map->Area() << " sq. degrees.\n";
  } else {
    cout << "\t\tBad, we should have recovered all of the original area.\n";
  }

  delete tmp_map;


  cout << "\tAddition:\n";

  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->AddMap(*stomp_map_nearby)) {
    cout << "\t\tNearby overlapping area: " <<
        tmp_map->Area() << " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() <<
        " (" << tmp_map->MinWeight() << " - " << tmp_map->MaxWeight() << ")\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some area left over.\n";
  }

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);
  tmp_map->ScaleWeight(2.0);
  if (tmp_map->AddMap(*stomp_map_nearby,false)) {
    cout << "\t\tNearby combined area: " <<
        tmp_map->Area() << " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() <<
        " (" << tmp_map->MinWeight() << " - " << tmp_map->MaxWeight() << ")\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some area left over.\n";
  }

  // Quick test of our weighted random point generating.

  tmp_map->GenerateRandomPoints(rand_ang,n_random,true);

  if (rand_ang.size() != n_random) {
    cout << "\t\tRandom point array size doesn't match requested size!\n";
    exit(1);
  }

  n_found = 0;
  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (tmp_map->FindLocation(*iter,weight)) n_found++;

  cout << "\tVerified that " << n_found << "/" << n_random <<
      " points within map.\n";

  output_file_name = "RandomPointsWeighted.dat";
  cout << "\tWriting random points to " << output_file_name.c_str() << "\n";

  ofstream outputFile2(output_file_name.c_str());

  if (outputFile2.is_open()) {
    for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
      outputFile2 << iter->Lambda() << " " << iter->Eta() << "\n";

    outputFile2.close();
  } else {
    cout << "Failed to open " << output_file_name.c_str() << "\n";
  }

  output_file_name = "StompAddedMap.pix";
  tmp_map->Write(output_file_name);

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->AddMap(*stomp_map_faraway,false)) {
    cout << "\t\tFaraway combined area: " << tmp_map->Area() <<
        " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() <<
        " (" << tmp_map->MinWeight() << " - " << tmp_map->MaxWeight() << ")\n";
  } else {
    cout << "\t\tBad, we should have recovered all of the original area.\n";
  }

  delete tmp_map;


  cout << "\tMultiplication:\n";

  tmp_map = new StompMap(annulus_pix);
  if (tmp_map->MultiplyMap(*stomp_map_nearby)) {
    cout << "\t\tNearby overlapping area: " <<
        tmp_map->Area() << " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() << "\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some area left over.\n";
  }

  delete tmp_map;


  tmp_map = new StompMap(annulus_pix);
  tmp_map->ScaleWeight(2.0);
  if (tmp_map->MultiplyMap(*stomp_map_nearby,false)) {
    cout << "\t\tNearby combined area: " <<
        tmp_map->Area() << " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() << "\n";
  } else {
    cout << "\t\tThis is bad," <<
        " there should have been some area left over.\n";
  }

  delete tmp_map;

  tmp_map = new StompMap(annulus_pix);
  tmp_map->ScaleWeight(3.0);
  if (tmp_map->MultiplyMap(*stomp_map_faraway,false)) {
    cout << "\t\tFaraway combined area: " << tmp_map->Area() <<
        " sq. degrees.\n";
    cout << "\t\tAverage weight: " << tmp_map->AverageWeight() << "\n";
  } else {
    cout << "\t\tBad, we should have recovered all of the original area.\n";
  }

  delete tmp_map;

  delete stomp_map_nearby;
  delete stomp_map_faraway;

  // Before moving on to the density map tests, we need to verify that the
  // StompDensityPixel, a derived class from StompPixel, is working properly.
  //  Let's start with some initialization.

  ang.SetSurveyCoordinates(60.0,0.0);
  StompDensityPixel* tmp_density = new StompDensityPixel(ang,256,1.0,1.0);
  tmp_pix.SetPixnumFromAng(ang);

  cout << "\nStompDensityPixel tests:\n";
  cout << "\tAngular Initialization: " <<
    tmp_density->Resolution() << " (" << tmp_pix.Resolution() << "), " <<
    tmp_density->PixelX() << " (" << tmp_pix.PixelX() << "), " <<
    tmp_density->PixelY() << " (" << tmp_pix.PixelY() << "), " <<
    tmp_density->Weight() << " (" << tmp_pix.Weight() << "), " <<
    tmp_density->Density() << "\n";

  delete tmp_density;

  tmp_density = new StompDensityPixel(tmp_pix.PixelX(),tmp_pix.PixelY(),
				      tmp_pix.Resolution(),1.0,1.0);

  cout << "\tIndex Initialization: " <<
    tmp_density->Resolution() << " (" << tmp_pix.Resolution() << "), " <<
    tmp_density->PixelX() << " (" << tmp_pix.PixelX() << "), " <<
    tmp_density->PixelY() << " (" << tmp_pix.PixelY() << "), " <<
    tmp_density->Weight() << " (" << tmp_pix.Weight() << "), " <<
    tmp_density->Density() << "\n";

  delete tmp_density;

  tmp_density = new StompDensityPixel(tmp_pix.Resolution(),tmp_pix.Pixnum(),
				      1.0,1.0);

  cout << "\tPixnum Initialization: " <<
    tmp_density->Resolution() << " (" << tmp_pix.Resolution() << "), " <<
    tmp_density->PixelX() << " (" << tmp_pix.PixelX() << "), " <<
    tmp_density->PixelY() << " (" << tmp_pix.PixelY() << "), " <<
    tmp_density->Weight() << " (" << tmp_pix.Weight() << "), " <<
    tmp_density->Density() << "\n";

  delete tmp_density;

  tmp_density = new StompDensityPixel(tmp_pix.Resolution(),tmp_pix.HPixnum(),
				      tmp_pix.Superpixnum(),1.0,1.0);

  cout << "\tHPixnum Initialization: " <<
    tmp_density->Resolution() << " (" << tmp_pix.Resolution() << "), " <<
    tmp_density->PixelX() << " (" << tmp_pix.PixelX() << "), " <<
    tmp_density->PixelY() << " (" << tmp_pix.PixelY() << "), " <<
    tmp_density->Weight() << " (" << tmp_pix.Weight() << "), " <<
    tmp_density->Density() << "\n";

  delete tmp_density;

  tmp_density = new StompDensityPixel(ang,32768,1.0,1.0);
  cout << "\tSpherical Coordinate tests:\n";
  cout <<
    "\t\tLambda: " << tmp_density->Lambda() << " (" << ang.Lambda() <<
    "), Eta: " << tmp_density->Eta() << " (" << ang.Eta() << ")\n";
  cout << "\t\tX: " << tmp_density->UnitSphereX() <<
    " (" << ang.UnitSphereX() << ")\n";
  cout << "\t\tY: " << tmp_density->UnitSphereY() <<
    " (" << ang.UnitSphereY() << ")\n";
  cout << "\t\tZ: " << tmp_density->UnitSphereZ() <<
    " (" << ang.UnitSphereZ() << ")\n";
  cout <<
    "\t\tRA: " << tmp_density->RA() << " (" << ang.RA() <<
    "), DEC: " << tmp_density->DEC() << " (" << ang.DEC() << ")\n";
  cout <<
    "\t\tGalLat: " << tmp_density->GalLat() << " (" << ang.GalLat() <<
    "), GalLon: " << tmp_density->GalLon() << " (" << ang.GalLon() << ")\n";


  // Now we start testing the density map functions.  First we make a density
  // map at resolution 128 using our previous StompMap (recall that it was
  // created at resolution 256).  There will be more pixels in the density
  // map, but we should find that their areas are equal.

  StompDensityMap* density_map = new StompDensityMap(*stomp_map,128);

  cout << "\nDensity Map tests:\n";

  cout << "\t" << density_map->Size() << " pixels in the density map.\n";
  cout << "\t" << stomp_map->Size() << " pixels in the source map.\n";

  cout << "\t" << density_map->Area() << " sq. degrees in the density map.\n";
  cout << "\t" << stomp_map->Area() << " sq. degrees in the source map.\n";

  // Now in more detail on the area:

  cout << "\tIn more detail:\n";

  density_map->Coverage(superpix);

  for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    unsigned long n_pixel =
      density_map->End(iter->Superpixnum()) -
      density_map->Begin(iter->Superpixnum());
    cout << "\t\t" << iter->Superpixnum() << ": " <<
      density_map->Area(iter->Superpixnum()) << " square degrees, " <<
      n_pixel << " pixels.\n";
    cout << "\t\t\tOrginal map: " << stomp_map->Area(iter->Superpixnum()) <<
        " square degrees, " << stomp_map->Size(iter->Superpixnum()) <<
        " pixels.\n";
    cout << "\t\t\tResampled map: " <<
        density_map->FindUnmaskedFraction(*iter)*iter->Area() <<
      " square degrees\n";
  }

  // Now, let's add those random points to the current map.  Since they were
  // both created with the same source map, they should all find a home.

  cout << "\tAttempting to add random points to density map\n";
  n_found = 0;

  for (AngularIterator iter=rand_ang.begin();iter!=rand_ang.end();++iter)
    if (density_map->AddToMap(*iter)) n_found++;

  cout << "\t\tPut " << n_found << "/" << rand_ang.size() <<
    " points in map.\n";
  cout << "\t\t\t" << density_map->MeanDensity() << " points/sq. degree.\n";

  cout << "\t\tChecking distribution:\n";
  for (StompIterator iter=superpix.begin();iter!=superpix.end();++iter) {
    cout << "\t\t\t" << iter->Superpixnum() << ": " <<
        density_map->Density(iter->Superpixnum()) <<
        " objects/square degree\n";
  }

  output_file_name = "StompDensityMap.pix";

  cout << "\tWriting StompDensityMap to " << output_file_name.c_str() << "\n";
  density_map->Write(output_file_name,true,true);



  // Alright, now let's test the code's ability to find the area and density
  // contained within an annulus projected on the survey area.  This is the
  // forerunner to the code that'll eventually do the correlation function
  // measurements.

  cout << "\t1 degree circle around map origin:\n";
  cout << "\t\t" << density_map->FindLocalArea(ang,1.0) << " sq. degrees.\n";
  cout << "\t\t\t" << density_map->FindLocalDensity(ang,1.0) <<
    " objects/sq. degree.\n";

  ang.SetSurveyCoordinates(62.0,2.0);
  cout << "\t1 degree circle around nearby map origin:\n";
  cout << "\t\t" << density_map->FindLocalArea(ang,1.0) << " sq. degrees.\n";
  cout << "\t\t\t" << density_map->FindLocalDensity(ang,1.0) <<
    " objects/sq. degree.\n";

  ang.SetSurveyCoordinates(0.0,0.0);
  cout << "\t1 degree circle around faraway map origin:\n";
  cout << "\t\t" << density_map->FindLocalArea(ang,1.0) << " sq. degrees.\n";
  cout << "\t\t\t" << density_map->FindLocalDensity(ang,1.0) <<
    " objects/sq. degree.\n";

  // Now let's check our re-sampling code.

  cout << "\tResampling tests:\n";

  for (int resolution=density_map->Resolution()/2;
       resolution>=Stomp::HPixResolution();resolution /= 2) {
    StompDensityMap* sub_density_map =
        new StompDensityMap(*density_map,resolution);
    double total_density = 0.0, total_area = 0.0;;
    for (StompDensityIterator iter=sub_density_map->Begin();
         iter!=sub_density_map->End();++iter) {
      total_density += iter->Density();
      total_area += iter->Weight()*iter->Area();
    }

    cout << "\t\t" << resolution << ": Stored object total = " <<
        sub_density_map->Density()*sub_density_map->Area() <<
        " (" << sub_density_map->Area() <<
        ")\n";
    cout << "\t\t    Calculated = " << total_density << " (" << total_area <<
        ")\n";
    cout << "\t\t    Should be " <<
        density_map->Density()*density_map->Area() <<
        "(" << density_map->Area() << ")\n";
    cout << "\t\t\tMean pixel density: " << sub_density_map->MeanDensity() <<
        " (" << density_map->MeanDensity() << ")\n";
    delete sub_density_map;
  }

  cout << "\tResampling tests (post-overdensity translation):\n";

  density_map->ConvertToOverDensity();

  for (int resolution=density_map->Resolution()/2;
       resolution>=Stomp::HPixResolution();resolution /= 2) {
    StompDensityMap* sub_density_map =
        new StompDensityMap(*density_map,resolution);
    double total_density = 0.0, total_area = 0.0;;
    for (StompDensityIterator iter=sub_density_map->Begin();
         iter!=sub_density_map->End();++iter) {
      total_density += iter->Density();
      total_area += iter->Weight()*iter->Area();
    }

    cout << "\t\t" << resolution << ": Stored object total = " <<
        sub_density_map->Density()*sub_density_map->Area() <<
        " (" << sub_density_map->Area() <<
        ")\n";
    cout << "\t\t    Calculated = " << total_density << " (" << total_area <<
        ")\n";
    cout << "\t\t    Should be " <<
        density_map->Density()*density_map->Area() <<
        "(" << density_map->Area() << ")\n";
    cout << "\t\t\tMean pixel density: " << sub_density_map->MeanDensity() <<
        " (" << density_map->MeanDensity() << ")\n";
    delete sub_density_map;
  }


  // Another important step is checking to make sure that we can break
  // the map up into roughly equal chunks for jack-knife error calculations.
  // This first call probably won't work very well since we used a fairly
  // coarse resolution when we initialized this map.

  cout << "\tTrying to regionate the density map into 10 pieces...\n";
  density_map->InitializeRegions(10);


  // A better result should be found as we increase the resolution for the
  // region map.

  cout << "\tNow doing it with finer resolution...\n";
  StompDensityMap* hires_density_map = new StompDensityMap(*stomp_map,128,
							   0.00001,32);

  hires_density_map->InitializeRegions(10);

  delete hires_density_map;

  // Of course, we're limited in this direction by the maximum resolution of
  // our map.  These two things are separated so that we can do several maps
  // of the same data at different resolution (which speeds up the
  // auto-correlation measurement), but use the same regionated map (which is
  // necessary if we want our jack-knife errors to be meaningful.

  cout << "\tNow doing it with full resolution...\n";
  hires_density_map = new StompDensityMap(*stomp_map,128,0.00001,128);

  hires_density_map->InitializeRegions(10);

  delete hires_density_map;


  // Almost there.  Now we break out the angular bin code.  This class
  // lets you define either a linear or logarithmic binning, depending on
  // how you instantiate it.  We'll use the latter here.

  cout << "Angular Bin routines:\n";

  double theta_min = 0.01;
  double theta_max = 10.0;

  cout << "Begin by setting up a log-space binning from " <<
    theta_min << " to " << theta_max << "...\n";

  AngularCorrelation *wtheta = new AngularCorrelation(theta_min,theta_max,
						      6.0,false);

  cout << "\t" << wtheta->NBins() << " angular bins:\n";
  for (ThetaIterator iter=wtheta->Begin();iter!=wtheta->End();++iter)
    cout << "\t\t" << iter->ThetaMin() << " - " << iter->ThetaMax() <<
      " (" << iter->SinThetaMin() << " - " << iter->SinThetaMax() << ")\n";


  // Of course, just as important as the binning is knowing what resolution
  // map we need to use to calculate the auto-correlation on those scales.
  // This method figures that out.

  cout << "\tSetting up resolution values for each angular bin...\n";

  wtheta->AssignBinResolutions();

  for (ThetaIterator iter=wtheta->Begin();iter!=wtheta->End();++iter)
    cout << "\t\t" << iter->Theta() << ": " << iter->Resolution() << "\n";


  // Now we test our methods for figuring out the angular bin extent we'd use
  // for a given map resolution.

  cout << "\tChecking to see which angular bins to use" <<
    " for our " << density_map->Resolution() << " density map...\n";

  cout << "\t\tAngular Range: " <<
    wtheta->ThetaMin(density_map->Resolution()) << " - " <<
    wtheta->ThetaMax(density_map->Resolution()) << "\n";

  cout << "\t\tIterator Test: " <<
    wtheta->Begin(density_map->Resolution())->ThetaMin() << " - " <<
    wtheta->End(density_map->Resolution())->ThetaMin() << "\n";


  // We've deviated a bit here from the STL to do the angular bin searching.
  // Instead of using the standard algorithms for figuring out which bin a
  // given angular separation (here given in terms of sin^2(theta) for reasons
  // which are clear if you look at the AutoCorrelate code), we use our own
  // binary search.  Given that, we need to make sure that it's doing the right
  // things.

  cout << "\tAngular Bin Search:\n";

  theta = 0.05;
  double sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  ThetaIterator theta_iter = wtheta->Find(wtheta->Begin(),wtheta->End(),
					  sintheta);
  cout << setprecision(6) <<
      "\t\t" << theta << ": " << theta_iter->ThetaMin() <<
      " - " << theta_iter->ThetaMax() << "\n";

  theta = 0.5;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(),wtheta->End(),sintheta);
  cout << "\t\t" << theta << ": " << theta_iter->ThetaMin() <<
    " - " << theta_iter->ThetaMax() << "\n";

  theta = 5.0;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(),wtheta->End(),sintheta);
  cout << setprecision(6) <<
      "\t\t" << theta << ": " << theta_iter->ThetaMin() <<
      " - " << theta_iter->ThetaMax() << "\n";



  // Now, restrict our search to just those bins that we'd use with our
  // density map and make sure that it's doing the right things.

  cout << "\tAngular Bin Search within the density map matching bins:\n";

  theta = 0.02;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(density_map->Resolution()),
			    wtheta->End(density_map->Resolution()),sintheta);
  if (theta_iter == wtheta->End(density_map->Resolution())) {
    cout << "\t\tGood.\n" <<
      "\t\t\tTried to search for an angular bin outside the range" <<
      "\n\t\t\tand got back the end iterator.\n";
  } else {
    cout << "\t\tBad: " << theta << ": " << theta_iter->ThetaMin() <<
      " - " << theta_iter->ThetaMax() << "\n";
  }


  theta = 0.15;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(density_map->Resolution()),
			    wtheta->End(density_map->Resolution()),sintheta);
  cout << "\t\t" << theta << ": " << theta_iter->ThetaMin() <<
    " - " << theta_iter->ThetaMax() << "\n";


  theta = 0.25;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(density_map->Resolution()),
			    wtheta->End(density_map->Resolution()),sintheta);
  cout << "\t\t" << theta << ": " << theta_iter->ThetaMin() <<
    " - " << theta_iter->ThetaMax() << "\n";


  theta = 2.0;
  sintheta = sin(theta*Stomp::Deg2Rad())*sin(theta*Stomp::Deg2Rad());
  theta_iter = wtheta->Find(wtheta->Begin(density_map->Resolution()),
			    wtheta->End(density_map->Resolution()),sintheta);
  if (theta_iter == wtheta->End(density_map->Resolution())) {
    cout << "\t\tGood.\n" <<
      "\t\t\tTried to search for an angular bin outside the range" <<
      "\n\t\t\tand got back the end iterator.\n";
  } else {
    cout << "\t\tBad: " << theta << ": " << theta_iter->ThetaMin() <<
      " - " << theta_iter->ThetaMax() << "\n";
  }



  // Ok, finally, we get to try out the auto-correlation code.  It's really
  // just this easy to invoke.

  cout << "AutoCorrelation Test:\n";

  density_map->AutoCorrelate(*wtheta);

  for (ThetaIterator iter=wtheta->Begin(density_map->Resolution());
       iter!=wtheta->End(density_map->Resolution());++iter)
    cout << "\tw(" << iter->Theta() << ") = " <<
        iter->Wtheta()/iter->Counter() << " (" << iter->Wtheta() << "," <<
        iter->Counter() << ")\n";


  // Alrighty, now for our grand finale, let's lay out the code that we'd
  // use to do the auto-correlation for all scales, using our current
  // density map as the highest resolution map.

  for (int resolution=density_map->Resolution()/2;
       resolution>=wtheta->MinResolution();resolution/=2) {

    StompDensityMap* sub_density_map =
        new StompDensityMap(*density_map,resolution);

    cout << "\t" << resolution <<
        ": Original Map Density: " << density_map->Density() <<
        ": New Map Density: " << sub_density_map->Density() << "\n";

    sub_density_map->AutoCorrelate(*wtheta);

    delete sub_density_map;
  }

  for (ThetaIterator iter=wtheta->Begin(density_map->Resolution());
       iter!=wtheta->End(wtheta->MinResolution());++iter)
    cout << "\tw(" << iter->Theta() << "," << iter->Resolution() << ") = " <<
        iter->Wtheta()/iter->Counter() << " (" << iter->Wtheta() << "," <<
        iter->Counter() << ")\n";

  // Testing the basic pixelization routines.

  ang.SetEquatorialCoordinates(20.0,0.0);

  CircleBound* circ = new CircleBound(ang,3.0,1.0);

  cout << "Circle Area: " << circ->Area() << " (" <<
      (1.0 - cos(3.0*Stomp::Deg2Rad()))*2.0*Stomp::Pi()*Stomp::Strad2Deg() <<
       ")\n";

  cout << "Pixelizing...\n";

  cout << "\tStarting resolution: " << circ->FindStartingResolution() << "\n";

  if (circ->FindXYBounds(circ->FindStartingResolution())) {
    cout << "\t\tFound X-Y bounds: " <<
        circ->XMin() << " - " << circ->XMax() << ", " <<
        circ->YMin() << " - " << circ->YMax() << "\n";
  } else {
    cout << "\t\tFindXYBounds failed...\n";
  }

  if (circ->Pixelize()) {
    cout << "Pixelization success!\nArea Check: Real: " <<
        circ->Area() << ", Pixelized: " << circ->PixelizedArea() << "\n";
  } else {
    cout << "Pixelization failed...\n";
  }

  // Trying again with coarser maximum resolution.

  circ->SetMaxResolution(2048);

  if (circ->Pixelize()) {
    cout << "Pixelization success!\nArea Check: Real: " <<
        circ->Area() << ", Pixelized: " << circ->PixelizedArea() << "\n";
  } else {
    cout << "Pixelization failed...\n";
  }

  AngularVector angVec;

  ang.SetEquatorialCoordinates(17.0,3.0);
  angVec.push_back(ang);
  ang.SetEquatorialCoordinates(17.0,-3.0);
  angVec.push_back(ang);
  ang.SetEquatorialCoordinates(23.0,-3.0);
  angVec.push_back(ang);
  ang.SetEquatorialCoordinates(23.0,3.0);
  angVec.push_back(ang);


  PolygonBound* poly = new PolygonBound(angVec,1.0);

  cout << "Polygon Area: " << poly->Area() << "\n";

  cout << "Pixelizing...\n";

  cout << "\tStarting resolution: " << poly->FindStartingResolution() << "\n";

  if (poly->FindXYBounds(poly->FindStartingResolution())) {
    cout << "\t\tFound X-Y bounds: " <<
        poly->XMin() << " - " << poly->XMax() << ", " <<
        poly->YMin() << " - " << poly->YMax() << "\n";
  } else {
    cout << "\t\tFindXYBounds failed...\n";
  }

  if (poly->Pixelize()) {
    cout << "Pixelization success!\nArea Check: Real: " <<
        poly->Area() << ", Pixelized: " << poly->PixelizedArea() << "\n";
  } else {
    cout << "Pixelization failed...\n";
  }

  // Trying again with coarser maximum resolution.

  poly->SetMaxResolution(2048);

  if (poly->Pixelize()) {
    cout << "Pixelization success!\nArea Check: Real: " <<
        poly->Area() << ", Pixelized: " << poly->PixelizedArea() << "\n";
  } else {
    cout << "Pixelization failed...\n";
  }



  return 0;
}
