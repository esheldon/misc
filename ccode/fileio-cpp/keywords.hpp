#if !defined (_keywords_hpp)
#define _keywords_hpp

#include "export.h"

using namespace std;

typedef struct {
  IDL_KW_RESULT_FIRST_FIELD; // Must be first entry in structure

  // These must be in alpabetical order

  // Ignored for reading
  IDL_LONG append;
  int append_there;

  // Ignored for reading
  IDL_LONG bracket_arrays;
  int bracket_arrays_there;


  IDL_VPTR columns;
  int columns_there;

  // Ignored for reading
  IDL_STRING delimiter;
  int delimiter_there;

  // ignored for binary
  IDL_LONG csv;
  int csv_there;

  IDL_LONG help;
  int help_there;

  IDL_VPTR rows;
  int rows_there;

  // Lines to skip
  IDL_MEMINT skiplines;
  int skiplines_there;

  // Status of the call 
  IDL_VPTR status;
  int status_there;

  IDL_LONG verbose;
  int verbose_there;
  
} KW_RESULT;

static IDL_KW_PAR kw_pars[] = {

  IDL_KW_FAST_SCAN, 

  /* These must be in alphabetical order */

  /* 
     IDL_KW_VIN: can be just an input, like var=3
     IDL_KW_OUT: can be output variable, var=var
     IDL_KW_ZERO: If not there, variable is set to 0

     If the VIN is set, then the _there variables will be
     0 when an undefined name variable is sent: no good for
     returning variables!
  */

  {"APPEND", 
   IDL_TYP_LONG, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(append_there), 
   static_cast<char *> IDL_KW_OFFSETOF(append) },

  {"BRACKET_ARRAYS", 
   IDL_TYP_LONG, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(bracket_arrays_there), 
   static_cast<char *> IDL_KW_OFFSETOF(bracket_arrays) },



  /* Column and row numbers to read */
  {"COLUMNS", 
   IDL_TYP_UNDEF, 
   1, 
   IDL_KW_VIN, 
   static_cast<int *> IDL_KW_OFFSETOF(columns_there), 
   static_cast<char *> IDL_KW_OFFSETOF(columns)},

  // Ignored for binary
  {"CSV", 
   IDL_TYP_LONG, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(csv_there), 
   static_cast<char *> IDL_KW_OFFSETOF(csv) },

  {"DELIMITER", 
   IDL_TYP_STRING, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(delimiter_there), 
   static_cast<char * >IDL_KW_OFFSETOF(delimiter) },


  {"HELP", 
   IDL_TYP_LONG, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(help_there), 
   static_cast<char *> IDL_KW_OFFSETOF(help) },

  {"ROWS", 
   IDL_TYP_UNDEF, 
   1, 
   IDL_KW_VIN, 
   static_cast<int *> IDL_KW_OFFSETOF(rows_there), 
   static_cast<char *> IDL_KW_OFFSETOF(rows)},


  {"SKIPLINES", 
   IDL_TYP_MEMINT, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(skiplines_there), 
   static_cast<char *> IDL_KW_OFFSETOF(skiplines) },


  {"STATUS", 
   IDL_TYP_UNDEF, 
   1, 
   IDL_KW_OUT | IDL_KW_ZERO, 
   static_cast<int *> IDL_KW_OFFSETOF(status_there), 
   static_cast<char *> IDL_KW_OFFSETOF(status) },

  // There was some kind of memory problem associated with this
  // when this was declared as IDL_TYP_INT but the keyword value
  // above was int
  {"VERBOSE", 
   IDL_TYP_LONG, 
   1, 
   0, 
   static_cast<int *> IDL_KW_OFFSETOF(verbose_there), 
   static_cast<char *> IDL_KW_OFFSETOF(verbose) },
  
  
  { NULL }
};


#endif // _keywords_hpp
