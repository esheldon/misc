//#     Filename:       SpatialMarkup.cpp
//#
//#     The SpatialMarkup class is defined here.
//#
//#     Author:         Peter Z. Kunszt based on A. Szalay's code
//#     
//#     Date:           October 23, 1998
//#
//#
//#
//# (c) Copyright The Johns Hopkins University 1998
//# All Rights Reserved
//#
//# The software and information contained herein are proprietary to The
//# Johns Hopkins University, Copyright 1998.  This software is furnished
//# pursuant to a written license agreement and may be used, copied,
//# transmitted, and stored only in accordance with the terms of such
//# license and with the inclusion of the above copyright notice.  This
//# software and information or any other copies thereof may not be
//# provided or otherwise made available to any other person.
//#
//#
//#     Modification History:
//#
#include "SpatialMarkup.h"

// ===========================================================================
//
// Static variables
//
// ===========================================================================

uint8 SpatialMarkup::vTrue  = 1;
uint8 SpatialMarkup::vFalse = 2;
uint8 SpatialMarkup::vUndef = 0; // we rely on this being zero at initializ.

// ===========================================================================
//
// Member functions for class SpatialMarkup
//
// ===========================================================================

/////////////CONSTRUCTOR//////////////////////////////////
//
//
SpatialMarkup::SpatialMarkup(const SpatialIndex & idx)
  : index(idx), 
    mark_(rEJECT, idx.nodes_.length(), 0),  // Set default fill value to reject
    vmark_(vUndef, idx.vertices_.length(), 0)// Set default to undef
{
  mark_.at(idx.nodes_.length()-1);
  vmark_.at(idx.vertices_.length()-1);
}

/////////////BRACKET OPERATOR[]///////////////////////////
// return marker for the node
//
SpatialMarkup::Markup &
SpatialMarkup::operator [](size_t nodeIndex) {
  return mark_.vector_[nodeIndex];
}

/////////////PARENTHESIS OPERATOR()///////////////////////
// return marker for the node
//
uint8 &
SpatialMarkup::operator ()(size_t vIndex) {
  return vmark_.vector_[vIndex];
}

/////////////CLEAR////////////////////////////////////////
// clear all markups to reject
//
void
SpatialMarkup::clear() {
  mark_.clear();
}

/////////////CLEARVERTEX//////////////////////////////////
// clear vertex markups to undef
//
void
SpatialMarkup::clearVertex() {
  vmark_.clear();
}

