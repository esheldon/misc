#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"
#include "fstream.h"
#include <time.h>
#include "export.h"
#include "objShear.h"

int
main(int argc, void *argv[]) {

  // Input variables 
  LENS_INPUT_STRUCT 
    *lensInStruct;

  LRG_SCAT
    *scat;

  int32
    *revInd;

  SCINV_STRUCT 
    *scinvStruct;

  PAR_STRUCT
    *parStruct;

  //////////////////////////////
  // Assign Input variables
  //////////////////////////////

  lensInStruct  = (LENS_INPUT_STRUCT *) argv[0];
  scat          = (LRG_SCAT *) argv[1];
  revInd        = (int32 *) argv[2];
  scinvStruct   = (SCINV_STRUCT *) argv[3];
  parStruct     = (PAR_STRUCT *) argv[4];

  // Calling objShear
  int retval = objShear(lensInStruct, scat, revInd, scinvStruct, parStruct);
  
  return(retval);

}
