//#     Filename:       timeHTM.cpp
//#
//#     Build the index and time it
//#
//#
//#     Author:         Peter Z. Kunszt
//#
//#     Date:           October 15, 1999
//#
//#
//#
//# (c) Copyright The Johns Hopkins University 1999
//# All Rights Reserved
//#
//# The software and information contained herein are proprietary to The
//# Johns Hopkins University, Copyright 1999.  This software is furnished
//# pursuant to a written license agreement and may be used, copied,
//# transmitted, and stored only in accordance with the terms of such
//# license and with the inclusion of the above copyright notice.  This
//# software and information or any other copies thereof may not be
//# provided or otherwise made available to any other person.
//#
//#
#include "SpatialIndex.h"
#include <time.h>

/*******************************************************
 
  DESCRIPTION
 
  This example code just builds the index using exception handling
  and times its initialization.
 
  It can be invoked by
 
  	timeHTM level savelevel
 

  where

     level     : the level depth to build the index (2 - 14)
     savelevel : the level depth to keep index in memory ( <= level)

*******************************************************/

main(int argc,char ** argv) {

  	// initialize
	if(argc < 3) {
	  cout << "Usage: timeHTM level savelevel" << endl;
	  return -1;
	}
	try {
	  // build the index and time it
	  time_t t0 = clock();
	  SpatialIndex index(atoi(argv[1]),atoi(argv[2]));
	  time_t t1 = clock();
	  printf(" Time to build index: %f sec\n",
		 (double) (t1-t0) / (double)CLOCKS_PER_SEC);
	}
	catch (SpatialException x)
	  {
	  }

	return 0;
}
