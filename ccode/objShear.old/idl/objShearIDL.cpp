#include "SpatialInterface.h"
#include "SpatialDomain.h"
#include "VarStr.h"
#include "fstream.h"
#include <time.h>
#include "export.h"
#include "objShear.h"
#include "sigmaCritInv.h"
#include "gcircSurvey.h"
#include "IDLStruct.h"
#include <stdlib.h>
#include <vector>
#include <algorithm>
using namespace std;

//////////////////////////////////////////////////////////////////////////////
// This defines the IDL_Load function used for Dynamically Loadable Modules
// It includes a fix for the name mangling done by g++
//////////////////////////////////////////////////////////////////////////////

#define ARRLEN(arr) (sizeof(arr)/sizeof(arr[0]))

/*
 * Here's the code to fix the name mangling of g++
 */

//
// First is the name twist of the original function
//
int IDL_Load_(void);

//
// Next are the shells declared with "external C"
//
extern "C" {
  int IDL_Load(void);
}

//
// Last are the one-line functions to call the originals
//
int IDL_Load(void) {
  return(IDL_Load_());
}

int IDL_Load_(void)
{

  /* This must be static. It is a struct. */
  /* The name in strings is the name by which it will be called from IDL and
     MUST BE CAPITALIZED 
     5th parameter will say if it accepts keywords and some other flags 
     For more info see page 325 of external dev. guide */
  static IDL_SYSFUN_DEF2 pro_addr[] = {
    { (IDL_SYSRTN_GENERIC) objShear, "OBJSHEAR", 0, 5, 0, 0},
  };

  /* The false means it is not a function */
  return IDL_SysRtnAdd(pro_addr, IDL_FALSE, ARRLEN(pro_addr));

}

