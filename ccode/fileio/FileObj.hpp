#if !defined (_fileobj_hpp)
#define _fileobj_hpp


#include <iostream>
#include <cstdio>
#include <cctype> // for toupper
#include <algorithm> // for transform
#include <vector>
#include <string>

#include "export.h"
#include "types.hpp"
#include "keywords.hpp"

// Constants

// Status values for the return keyword
static const int READ_SUCCESS  = 0;
static const int WRITE_SUCCESS = 0;
static const int READ_FAILURE  = 1;
static const int WRITE_FAILURE = 1;
static const int INPUT_ERROR   = 2;
static const int HELP_REQUEST  = 3;

// Internal status
static const int FILEOBJ_OK           = 0;
static const int FILEOBJ_INIT_ERROR   = 1;
static const int FILEOBJ_SYNTAX_ERROR = 2;
static const int FILEOBJ_PRINT_HELP   = 3;

// Action types
static const int ACTION_READ_BINARY   = 1;
static const int ACTION_READ_ASCII    = 1;
static const int ACTION_WRITE_BINARY  = 2; // Not yet supported
static const int ACTION_WRITE_ASCII   = 2;


using namespace std;

class FileObj {
  
public:
  FileObj(int      argc, 
	  IDL_VPTR argv[], 
	  char     argk[], 
	  int      iAction);

  // I was careful to use nice classes everywhere, and the one calloc is carefully
  // freed, so no explicit cleanup is needed other than the IDL_KW_FREE call.
  ~FileObj();

  // Initialization.  The arg and keyword processing differs between 
  // read and write
  void InitRead(int      argc, 
		IDL_VPTR argv[], 
		char     argk[]);

  void InitWrite(int      argc, 
		 IDL_VPTR argv[], 
		 char     argk[]);

  // Set the output status value
  void SetStatusKeyword(int statusVal);

  // Return the internal status (not keyword status)
  int Status();

  // Process input keywords
  int GetStructdefInfo();
  int GetInputRows();
  int GetInputColumns();
  int CsvKeywordSet();

  // Creating the output structure when reading
  void CreateOutputStruct();
  IDL_STRUCT_TAG_DEF *GetSubStructdef();
  void FreeTagDefs(IDL_STRUCT_TAG_DEF *tagDefs);
  IDL_VPTR OutputStruct();


  // Reading from the file
  int OpenFile(char* mode);
  int GetFilePtr(char* mode);
  void CloseFile();
  int SkipLines(int num);

  int ReadAsBinary();
  int ReadRowAsBinary();

  int ReadAsAscii();
  int ReadRowAsAscii();
  int ScanVal(int   type, 
	      char* buffer);
  void GetScanFormats(IDL_LONG csv);

  // Writing to the file
  int WriteAsAscii();
  void AsciiPrint(int    idlType, 
		  UCHAR* tptr);

  // Utility functions
  int IDLTypeNbytes(int type);
  void PrintIdlType(int type);

  void Message(char* text);

  int NumParams();
  void BinaryReadSyntax();
  void AsciiReadSyntax();
  void AsciiWriteSyntax();

protected:

  // IDL demands this is called kw so we cannot follow naming conventions.
  KW_RESULT kw;

  // Number of ordinary position parameters
  int mNumParams;

  // Just print the help message and quit?
  IDL_LONG mHelp;

  // This is internal status, not the keyword
  int mStatus; 
  int mVerbose;

  // Input file Vptr: may be string or lun
  IDL_VPTR mFileVptr;
  char*    mpFileName;
  FILE*    mFptr;

  // Read specific
  IDL_MEMINT mSkipLines;
  IDL_VPTR   mStructdefVptr;
  IDL_MEMINT mNumRowsInFile;
  IDL_MEMINT mNumColsInFile;

  vector<IDL_MEMINT> mRowsToGet;
  IDL_MEMINT         mNumRowsToGet;

  vector<IDL_MEMINT> mGetColumn;
  IDL_MEMINT         mNumColsToGet;

  IDL_VPTR mResultVptr;  // The result of the read
  char*    mpResult;     // pointer to data section of result

  // For ascii reading
  vector<string> mScanFormats;
  int            mReadAsCsv;


  // For ascii writing
  string   mDelim;
  char*    mpDelim;     // points to c_str() of mDelim
  string   mArrayDelim;
  char*    mpArrayDelim; // points to c_str() of mArrayDelim

  int mWriteAsCsv;
  int mBracketArrays;

  // For use with both reading and writing
  TagInfoStruct mTagInfo;
  char*         mpCurrentRow;



};


#endif  // _fileobj_hpp
