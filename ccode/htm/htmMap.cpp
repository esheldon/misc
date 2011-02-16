//#     Filename:       htmMap.cpp
//#
//#     on a given level, print the id and the center of each triangle.
//#
//#
//#     Author:         Peter Z. Kunszt
//#
//#     Date:           March 31, 2000
//#
//#
//#
//# (c) Copyright The Johns Hopkins University 2000
//# All Rights Reserved
//#
//# The software and information contained herein are proprietary to The
//# Johns Hopkins University, Copyright 1995-2000.  This software is furnished
//# pursuant to a written license agreement and may be used, copied,
//# transmitted, and stored only in accordance with the terms of such
//# license and with the inclusion of the above copyright notice.  This
//# software and information or any other copies thereof may not be
//# provided or otherwise made available to any other person.
//#
//#
#include "SpatialVector.h"
#include "SpatialIndex.h"
#include <stdlib.h>
#include "fstream.h"
#include "CartesianConverter.h"

int
main(int argc, char *argv[]) {

/*******************************************************
/
/ Initialization
/
/******************************************************/

  int depth;
  // read command line arguments
  if(argc <2) {
    cout << "Usage: " << argv[0] << " depth" << endl;
    return -1;
  }
  depth = atoi(argv[1]);

  SpatialIndex index(depth);
  SpatialVector v1,v2,v3,v;
  CartesianConverter conv;
  double ra,dec,gl,gb;

/*******************************************************
/
/ Print Map
/
/******************************************************/

  char fname[30];
  sprintf(fname,"htmMap_%d.dat",depth);
  ofstream out(fname);
  out.precision(15);

  for(size_t i = 0; i < index.leafCount(); i++) {
    index.nodeVertex(i,v1,v2,v3);
    v = v1 + v2 + v3;
    v.normalize();
    conv.toRaDec2000(v.x(),v.y(),v.z(),&ra,&dec);
    conv.toGalactic(v.x(),v.y(),v.z(),&gl,&gb);
    out	<< index.idByLeafNumber(i) << " " 
	<< ra << " "
	<< dec << " " 
	<< gl << " "
	<< gb << " "
	<< endl;
  }
  
  return 0;
}
