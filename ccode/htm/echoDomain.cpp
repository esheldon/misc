//#     Filename:       echoDomain.cpp
//#
//#     Read a domain file and echo it to screen. Any special domain file
//#     keywords are converted to the standard convex format.
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
#include "SpatialDomain.h"
#include "fstream.h"

/*******************************************************
 
  DESCRIPTION
 
  This example code echoes a domain file. It converts special formats
  to the default one.
 
  It can be invoked by
 
  	echoDomain domainfile
 

  where

     domainfile: file containing a domain

  see the description in intersect.cpp for domainfile formats and examples.

*******************************************************/

int
main(int argc, char *argv[]) {

  
  	if(argc <2){
	  cout << "Usage: echoDomain domainfile" << endl;
	  return -1;
	}

	ifstream in(argv[1]);
	if(!in) {
	  cout << "cannot open file! " << endl;
	  return -1;
	}

	SpatialDomain domain;
	in >> domain;
	cout << domain;

	return 0;
}
