/*---------------------------------------------------------------------------

  NAME:
    ascii_write
  
  CALLING SEQUENCE:
    IDL> ascii_write, struct, filename/lun, /append, 
                    /csv, delimiter=, /bracket_arrays, status=, /help

  PURPOSE:

    Write an IDL structure to an ascii file.  This is about 12 times faster than
    using the built in printf statement for small structure definitions.  For
    really big ones, getting the offsets is the bottleneck. For a tsObj file
    its only about 5 times faster.

    This program is written in C++ and is linked to IDL via the DLM mechanism.

  INPUTS: 
     struct: The structure array to write. 
     file/lun: Filename or file unit. For string file names, the user must 
               expand all ~ or other environment variables.  If the file
	       unit is entered, the file must be opened with the appropriate 
	       keywords:
                 openr, lun, file, /get_lun, /stdio, bufsize=0

  OPTIONAL INPUTS:
     /cvs: Use ',' as the field delimiter.
     delimiter: The field delimiter; default is the tab character.
     /append: Append the file.
     /bracket_arrays: {} is placed around array data in each row, with values comma 
        delimited within.  This is the format, with a whitespace delimiter for 
	ordinary fields, is required for file input to postgresql databases.
     /help: Print this documentation.

  OPTIONAL OUTPUTS:
    status=: The status of the read. 0 for success, 1 for read failure, 
             2 for input errors such as bad file unit.


  REVISION HISTORY:
    Created December-2005: Erin Sheldon, NYU
    Converted to C++, 2006-July-17, E.S. NYU


  Copyright (C) 2005  Erin Sheldon, NYU.  erin dot sheldon at gmail dot com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA


  ---------------------------------------------------------------------------*/

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>

#include "export.h"
#include "FileObj.hpp"


using namespace std;


void ascii_write(int argc, IDL_VPTR argv[], char argk[])
{

  FileObj fileobj(argc, argv, argk, ACTION_WRITE_ASCII);

  // Syntax errors or help requests are special cases
  if ( FILEOBJ_SYNTAX_ERROR == fileobj.Status() || 
       FILEOBJ_PRINT_HELP   == fileobj.Status() )
    {
      fileobj.AsciiWriteSyntax();
      return;
    }

  // Other errors
  if (fileobj.Status() != FILEOBJ_OK)
    return;   

  fileobj.WriteAsAscii();
  return;

}





























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
    { (IDL_SYSRTN_GENERIC) ascii_write, 
      "ASCII_WRITE", 
      0, 
      IDL_MAXPARAMS, 
      IDL_SYSFUN_DEF_F_KEYWORDS, 0},
  };

  /* The false means it is not a function */
  return IDL_SysRtnAdd(pro_addr, IDL_FALSE, ARRLEN(pro_addr));

}

