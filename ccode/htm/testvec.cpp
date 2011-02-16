#include "SpatialIndex.h"
#include "fstream.h"

main(int argc, char ** argv) {

  if(argc <3){
    cout << "Usage: testvec level divisions" << endl;
    return 0;
  }
  /*
  ifstream in(argv[2]);
  if(!in) {
    cout << "cannot open file! " << endl;
    return -1;
  }
  */

  SpatialIndex index(atoi(argv[1]));
  size_t div = atoi(argv[2]);
/*******************************************************
/
/ Domain Intersection Test
/
/******************************************************/

  float64 ra,dec;
  cout << "Input ra: " << flush;
  cin >> ra;
  cout << "Input dec: " << flush;
  cin >> dec;

  SpatialVector v1,v2,v3;
  uint32 id = index.idByPoint(ra,dec);
  cout << index.nameById(id) << endl;
  index.nodeVertex(index.leafNumberById(id),v1,v2,v3);
  cout << v1 << endl << v2 << endl << v3 << endl;

  // first calculate planes perpendicular to v1v2 v2v3.
  // then calculate the point where this plane intersects the
  // sides v1v2 (at a1), v2v3 (at a2)

  SpatialVector e1,e2,a1,a2;
  SpatialVector v12 = v1 ^ v2;
  SpatialVector v23 = v2 ^ v3;

  e1 = v12 ^ v3;
  e2 = v23 ^ v1;

  a1 = v12 ^ e1;
  a2 = v23 ^ e2;

  if(a1 * v2 < 0) a1 = (-1.0) * a1;
  if(a2 * v2 < 0) a2 = (-1.0) * a2;
  a1.normalize();
  a2.normalize();

  // get the tangent vector to the sphere at v1 that is in the
  // perpendicular plane
  SpatialVector b;
  b = v1 ^ (v3 ^ v1);
  b.normalize();
  // Now we have the angle that will be used as the coordinate system.

  float64 phi;
  phi = acos(v3*v1)/div;

  cout << "RESULT:" << endl;
  for(size_t i = 0; i <= div; i++) {
    SpatialVector p = v1 + tan(i*phi) * b;
    p.normalize();
    SpatialVector k1 = ((a1 * p) / (v3 * p)) * v3 - a1;
    SpatialVector k2 = a2 - ((a2 * p) / (v1 * p)) * v1;
    k1.normalize(); k2.normalize();
    cout << k1 << endl << k2 << endl;
    SpatialVector x = (v3 ^ v2) ^ k1;
    SpatialVector y = (v2 ^ v1) ^ k2;
    x.normalize(); y.normalize();
    cout << acos(v3 * v2) / acos(v3 * x) << endl;
    cout << acos(v2 * v1) / acos(v2 * y) << endl;
  }
}

