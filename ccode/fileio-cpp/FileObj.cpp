#include "FileObj.hpp"

//////////////////////////////////////////////////////////////////////////
//
// Initialization
//
//////////////////////////////////////////////////////////////////////////

// Constructor.  This is all generic with respect to reading and
// writing.  
FileObj::FileObj(int      argc, 
		 IDL_VPTR argv[], 
		 char     argk[], 
		 int      iAction) 
{

  if (ACTION_READ_ASCII == iAction || ACTION_READ_BINARY == iAction)
    InitRead(argc, argv, argk);
  else if (ACTION_WRITE_ASCII == iAction)
    InitWrite(argc, argv, argk);
  else
    {
      char mess[30];
      sprintf(mess, "Invalid action: %d", iAction);
      Message(mess);
      SetStatusKeyword(INPUT_ERROR);
      mStatus = FILEOBJ_INIT_ERROR;
    }
}

void
FileObj::InitRead(int      argc, 
		  IDL_VPTR argv[], 
		  char     argk[])
{

  // internal status
  mStatus = FILEOBJ_OK;

  /* Process the keywords */
  mNumParams = IDL_KWProcessByOffset(argc, argv, argk, kw_pars, 
				     (IDL_VPTR *) 0, 1, &kw);

  // Keyword status. No success until we read the file
  SetStatusKeyword(READ_FAILURE);

  // Should we just print a help statement?
  mHelp=0;
  if (kw.help_there)
    { 
      mHelp = kw.help;
      if (mHelp != 0) 
	{
	  SetStatusKeyword(HELP_REQUEST);
	  mStatus = FILEOBJ_PRINT_HELP;
	  return;
	}
    }

  if (mNumParams != 3)
    {
      SetStatusKeyword(INPUT_ERROR);
      mStatus = FILEOBJ_SYNTAX_ERROR;
      return;
    }

  // Get the arguments
  mFileVptr = argv[0];
  mStructdefVptr = argv[1];
  mNumRowsInFile = IDL_MEMINTScalar(argv[2]);


  // Verbosity level
  mVerbose = 0;
  if (kw.verbose_there) mVerbose = kw.verbose;

  // Should we skip lines?
  mSkipLines=0;
  if (kw.skiplines_there) mSkipLines = kw.skiplines;

  // Get the structdef info
  if (!GetStructdefInfo()) return;

  // Get the input rows if there are any
  if (!GetInputRows()) return;

  // See which columns should be read
  if (!GetInputColumns()) return;

  // Make sure they see what we wrote
  if (mVerbose) fflush(stdout);

}

void

FileObj::InitWrite(int      argc, 
		   IDL_VPTR argv[], 
		   char     argk[])
{

  // internal status
  mStatus = FILEOBJ_OK;

  /* Process the keywords */
  mNumParams = IDL_KWProcessByOffset(argc, argv, argk, kw_pars, 
				     (IDL_VPTR *) 0, 1, &kw);

  // Keyword status. No success until we read the file
  SetStatusKeyword(WRITE_FAILURE);

  // Should we just print a help statement?
  mHelp=0;
  if (kw.help_there)
    { 
      mHelp = kw.help;
      if (mHelp != 0) 
	{
	  SetStatusKeyword(HELP_REQUEST);
	  mStatus = FILEOBJ_PRINT_HELP;
	  return;
	}
    }

  if (mNumParams != 2)
    {
      SetStatusKeyword(INPUT_ERROR);
      mStatus = FILEOBJ_SYNTAX_ERROR;
      return;
    }


  // Get the arguments
  mStructdefVptr = argv[0];
  mFileVptr = argv[1];

  // Should we print entire help statement when printing syntax?
  mHelp=0;
  if (kw.help_there) mHelp = kw.help;

  // verbosity level
  mVerbose = 0;
  if (kw.verbose_there) mVerbose = kw.verbose;

  // Get the structdef info
  if (!GetStructdefInfo()) return;

  if (mVerbose) fflush(stdout);

}



// I was careful to use nice classes everywhere, and the one calloc is carefully
// freed, so no explicit cleanup is needed other than the IDL_KW_FREE call.
FileObj::~FileObj() 
{
  IDL_KW_FREE;
}



/////////////////////////////////////////////////////////////////////////////
//
// Get information about the input structure and store in such a way
// that is more easily accessible
//
/////////////////////////////////////////////////////////////////////////////

int FileObj::GetStructdefInfo()
{

  if (mStructdefVptr->type != IDL_TYP_STRUCT)
    {
      Message("The second argument must be a structure");
      SetStatusKeyword(INPUT_ERROR);
      mStatus = FILEOBJ_INIT_ERROR;
      return(0);
    }

  IDL_STRING *tptr;
  int debug=0;
  if (mVerbose == 2) debug=1;


  //---------------------------------------------------------------------------
  // Initialize the tag info structure
  // mStructdefVptr points to the input structdef
  //---------------------------------------------------------------------------


  mTagInfo.NumTags = IDL_StructNumTags(mStructdefVptr->value.s.sdef);
  mNumColsInFile = mTagInfo.NumTags;

  mTagInfo.TagNames.resize(mTagInfo.NumTags);
  mTagInfo.TagOffsets.resize(mTagInfo.NumTags);
  mTagInfo.TagDesc.resize(mTagInfo.NumTags);
  mTagInfo.TagBytes.resize(mTagInfo.NumTags);
  mTagInfo.TagNelts.resize(mTagInfo.NumTags);
  mTagInfo.buffer.resize(mTagInfo.NumTags);

  //---------------------------------------------------------------------------
  // Get the tag info
  //---------------------------------------------------------------------------


  mTagInfo.BytesPerRow = 0;
  for (IDL_MEMINT tag=0; tag<mTagInfo.NumTags; tag++)
    {
      // Get tag name
      mTagInfo.TagNames[tag] = IDL_StructTagNameByIndex(mStructdefVptr->value.s.sdef, tag, 
						       IDL_MSG_INFO, NULL);
      mTagInfo.TagMap[mTagInfo.TagNames[tag]] = tag;

      // Get tag offsets and the tag description variable (a copy?)
      mTagInfo.TagOffsets[tag] = IDL_StructTagInfoByIndex(mStructdefVptr->value.s.sdef, 
							 tag, 
							 IDL_MSG_LONGJMP,
							 &(mTagInfo.TagDesc[tag]) );
      if (debug) 
	{
	  printf("    Tag %d = \"%s\" ", tag, mTagInfo.TagNames[tag].c_str());
	  cout <<"    TagMap[\"" 
	    << mTagInfo.TagNames[tag] << "\"] = " <<
	    mTagInfo.TagMap[ mTagInfo.TagNames[tag] ] << endl;
	}

      // Deal with arrays
      if ( (mTagInfo.TagDesc[tag]->flags & IDL_V_ARR) != 0)
	{
	  // this is just for convenience
	  mTagInfo.TagNelts[tag] = mTagInfo.TagDesc[tag]->value.arr->n_elts;
	  if (debug) printf(" ARRAY[%d] ", mTagInfo.TagNelts[tag]);
	}
      else
	{
	  mTagInfo.TagNelts[tag] = 1;
	  if (debug) printf(" SCALAR ");
	}


      // Number of bytes for this variable type.
      if (mTagInfo.TagDesc[tag]->type == IDL_TYP_STRING) 
	{

	  // WARNING: This assumes that the elements in string arrays are all
	  // the same size
	  tptr = (IDL_STRING *) (mStructdefVptr->value.s.arr->data + mTagInfo.TagOffsets[tag]);
	  mTagInfo.TagBytes[tag] = tptr->slen;

	  /* Create the buffer */
	  mTagInfo.buffer[tag].resize(mTagInfo.TagBytes[tag]+1);
	  
	}
      else 
	mTagInfo.TagBytes[tag] = IDLTypeNbytes(mTagInfo.TagDesc[tag]->type);

      // Bytes in each row
      mTagInfo.BytesPerRow += mTagInfo.TagBytes[tag]*mTagInfo.TagNelts[tag];

      if (debug) 
	{
	  PrintIdlType(mTagInfo.TagDesc[tag]->type);
	  printf(" %d bytes",mTagInfo.TagBytes[tag]);
	}


      if (debug) printf("\n");


    }

  return(1);

}


/////////////////////////////////////////////////////////////////////////////
//
// Create the output structure
//
/////////////////////////////////////////////////////////////////////////////

void FileObj::CreateOutputStruct()
{


  
  // Create output struct. Must initialize (IDL_TRUE) since we might have read errors
  // and this would cause error if try to free. 

  if (mNumColsToGet == mNumColsInFile)
    {
      if (mVerbose) 
	{printf("Extracting all columns\n");fflush(stdout);}
      mpResult = 
	IDL_MakeTempStructVector(mStructdefVptr->value.s.sdef, 
				 (IDL_MEMINT) mNumRowsToGet, 
				 &mResultVptr, 
				 IDL_TRUE);
    }
  else
    {
      if (mVerbose) 
	{
	  printf("Extracting %d/%d columns:  ",mNumColsToGet,mNumColsInFile);fflush(stdout);
	  for (IDL_MEMINT i=0;i<mNumColsInFile;i++)
	    if (mGetColumn[i]) printf("%d ", i);
	  printf("\n");
	}
      IDL_STRUCT_TAG_DEF *sub_tag_defs = GetSubStructdef();
      mpResult = 
	IDL_MakeTempStructVector((IDL_StructDefPtr) IDL_MakeStruct(NULL, sub_tag_defs),
				 (IDL_MEMINT) mNumRowsToGet, 
				 &mResultVptr, 
				 IDL_TRUE);

      // Reset the TagOffsets for this new struct
      // Get tag offsets and the tag description variable (a copy?)
      IDL_MEMINT itag=0;
      for (IDL_MEMINT tag=0; tag<mNumColsInFile; tag++)
	{
	  if (mGetColumn[tag])
	    {
	      IDL_VPTR TagDesc;
	      mTagInfo.TagOffsets[tag] = 
		IDL_StructTagInfoByIndex(mResultVptr->value.s.sdef, 
					 itag, 
					 IDL_MSG_LONGJMP,
					 &(mTagInfo.TagDesc[tag]) );
	      itag++;
	    }
	}

      FreeTagDefs(sub_tag_defs);

    }
}



// Dealing with sub-structures.  This is were we must be careful with memory
// because we can't use vectors

IDL_STRUCT_TAG_DEF *FileObj::GetSubStructdef()
{
  IDL_STRUCT_TAG_DEF *tagDefs;

  tagDefs = 
    (IDL_STRUCT_TAG_DEF *) calloc(mNumColsToGet, sizeof(IDL_STRUCT_TAG_DEF));
  

  IDL_MEMINT i = 0;
  for (IDL_MEMINT column=0; column < mNumColsInFile; column++)
    {
      if (mGetColumn[ column ])
	{

	  // No copy made here I don't think.
	  tagDefs[i].name = (char *) mTagInfo.TagNames[column].c_str();

	  // No extra memory used here
	  tagDefs[i].type = (void *) (( IDL_MEMINT ) mTagInfo.TagDesc[column]->type);

	  // No extra memory used here
	  tagDefs[i].flags = 0;

	  // Here we have to allocate a new array because the IDL people
	  // wanted to be clever.  We could have just re-used the pointer
	  // to dim and copied n_dim, or they could have just set dims to 
	  // the maximum dimensions+1.  We have to be careful to free this.
	  if ((mTagInfo.TagDesc[column]->flags & IDL_V_ARR) != 0)
	    {
	      IDL_MEMINT n_dim = mTagInfo.TagDesc[column]->value.arr->n_dim;
	      tagDefs[i].dims = (IDL_MEMINT *) calloc(n_dim+1, sizeof(IDL_MEMINT));
	      tagDefs[i].dims[0] = n_dim;
	      for (IDL_MEMINT j=1;j<n_dim+1;j++)
		tagDefs[i].dims[j] = mTagInfo.TagDesc[column]->value.arr->dim[j-1];
	      
	    }

	  i++;
	}
    }


  return(tagDefs);
}

// This seems to be working.  No memory leaks detected.
void FileObj::FreeTagDefs(IDL_STRUCT_TAG_DEF *tagDefs)
{
  if (tagDefs)
    {
      for (int i=0; i<mNumColsToGet; i++)
	{
	  if (tagDefs[i].dims) 
	    free(tagDefs[i].dims);
	}
      
      free(tagDefs);
    }
}

IDL_VPTR FileObj::OutputStruct()
{
  return(mResultVptr);
}









/////////////////////////////////////////////////////////////////////////////
//
// Open and close the file
//
/////////////////////////////////////////////////////////////////////////////

int FileObj::OpenFile(char* mode)
{

  if (mFileVptr->type == IDL_TYP_STRING)
    {
      // Get the file name 
      mpFileName = IDL_VarGetString(mFileVptr);
      if (mVerbose) printf("Opening file = %s with mode \"%s\"\n", mpFileName, mode);
      //    IDL_FileStat()  
      if (!(mFptr = fopen(mpFileName, mode)))
	{
	  Message("Error opening file");
	  return(0);
	}
    }
  else
    {
      return( GetFilePtr(mode) );
    }

}


int FileObj::GetFilePtr(char* mode)
{

  IDL_FILE_STAT stat_blk;
  
  IDL_ENSURE_SCALAR(mFileVptr);
  IDL_LONG unit = IDL_LongScalar(mFileVptr);
  
  IDL_FileStat(unit, &stat_blk);
  
  if (*mode == 'w' || *mode=='a')
    {
      if ( !(stat_blk.access & IDL_OPEN_W) &&
	   !(stat_blk.access & IDL_OPEN_APND) &&
	   !(stat_blk.access & IDL_OPEN_NEW) )
	{
	  Message("File must be opened for writing");
	  return(0);
	}
    }
  else if (*mode == 'r')
    {
      if ( !(stat_blk.access & IDL_OPEN_R) )
	{
	  Message("File must be opened for reading");
	  return(0);
	}
    }
  else
    {
      char mess[25];
      sprintf(mess,"Unknown file mode %s", mode);
      Message(mess);
      return(0);
    }
  
  if ( ! (stat_blk.flags & IDL_F_STDIO) ) 
    Message("File not opened with /stdio flag.  Reading may fail");
  
  if ( !(mFptr = stat_blk.fptr) )
    {
      Message("FILEPTR is invalid");
      return(0);
    }
  
  return(1);
}


void FileObj::CloseFile()
{
  // Only close if it wasn't already opened on input
  if (mFileVptr->type == IDL_TYP_STRING)
    fclose(mFptr);
}


int FileObj::SkipLines(int num)
{
  if (num > 0)
    {
      IDL_MEMINT nlines = 0;
      char c;
      while (nlines < num) 
	{
	  c = getc(mFptr);
	  if (c == EOF) 
	    {
	      Message("SkipLines: Reached EOF prematurely");
	      return(0);
	    }
	  if (c == '\n') nlines++;
	}
    }
  return(1);
}





/////////////////////////////////////////////////////////////////////////////
//
// Read from the file as Binary
//
/////////////////////////////////////////////////////////////////////////////

int FileObj::ReadAsBinary()
{

  // Only this can result in no returned data at this point.
  if (!OpenFile("r")) 
    {
      SetStatusKeyword(READ_FAILURE);
      return(0);
    }

  // Skip any lines if requested
  if (!SkipLines(mSkipLines))
    {
      SetStatusKeyword(READ_FAILURE);
      return(0);
    }

  // The output data structure
  CreateOutputStruct();


  // Loop over the mRowsToGet
  IDL_MEMINT row2get, current_row=0;

  for (IDL_MEMINT irow2get=0; irow2get < mNumRowsToGet; irow2get++)
    {

      row2get = mRowsToGet[irow2get];

      // Are we skipping any rows? If not, then we just read
      if (current_row < row2get)
	{
	  if (fseek(mFptr, mTagInfo.BytesPerRow*(row2get-current_row), SEEK_CUR) != 0)
	    {
	      SetStatusKeyword(READ_FAILURE);
	      Message("Requested data beyond EOF. WARNING: Result will contain data read up to this error for debugging purposes.");
	      break;
	    }
	  current_row = row2get;
	}

      // Point to the structure row in memory.
      mpCurrentRow = 
	(char *) mResultVptr->value.s.arr->data + irow2get*( mResultVptr->value.arr->elt_len );

      // Read the row
      if (!ReadRowAsBinary())
	{
	  SetStatusKeyword(READ_FAILURE);
	  Message("Requested data beyond EOF. WARNING: Result will contain data read up to this error for debugging purposes.");
	  break;
	}

      current_row++;

    } // Loop over mRowsToGet

  CloseFile();
  return(1);

}

int FileObj::ReadRowAsBinary()
{

  for (IDL_MEMINT tag=0; tag < mNumColsInFile; tag++ )
    {

      if ( mGetColumn[tag] )
	{

	  // Offset into the struct we are getting; the TagOffsets have bee
	  // reset to point into the new struct if we are getting a substruct
	  char *tptr = mpCurrentRow + mTagInfo.TagOffsets[tag];

	  // Simple if not a string
	  if (mTagInfo.TagDesc[tag]->type != IDL_TYP_STRING)
	    {
	      int nRead = fread(tptr, 
				mTagInfo.TagBytes[tag], 
				mTagInfo.TagNelts[tag], 
				mFptr);
	      
	      // check for read errors 
	      if (nRead != mTagInfo.TagNelts[tag])
		return(0);
	    }
	  else 
	    {
	      // Strings: Need to loop over and store each separately in a
	      // buffer before copying to struct.  This will work for
	      // scalars and arrays of strings.  Note we assume all strings
	      // in array are same length as the example given in input
	      // struct! 
	      
	      IDL_STRING *tStringPtr = (IDL_STRING *) tptr;
	      for (IDL_MEMINT i=0; i<mTagInfo.TagNelts[tag]; i++)
		{
		  int nRead = fread( (char *) &(mTagInfo.buffer[tag])[0], 
				     mTagInfo.TagBytes[tag], 
				     1, 
				     mFptr);
		  
		  /* check for read errors */
		  if (nRead != 1)
		    return(0);
		  
		  IDL_StrStore(tStringPtr, (char *) mTagInfo.buffer[tag].c_str());
		  tStringPtr++;
		}
	      
	    }


	}
      else
	{
	  /* Skipping this column */
	  if (fseek(mFptr, mTagInfo.TagBytes[tag]*mTagInfo.TagNelts[tag], SEEK_CUR) != 0)
	    return(0);
	} // Not extracting column

    }

  return(1);

}


/////////////////////////////////////////////////////////////////////////////
//
// Read from the file as Ascii
//
/////////////////////////////////////////////////////////////////////////////


int FileObj::ReadAsAscii()
{

  // Only this can result in no returned data at this point.
  if (!OpenFile("r")) 
    {
      SetStatusKeyword(READ_FAILURE);
      Message("No results returned");
      return(0);
    }

  // Skip any lines if requested
  if (!SkipLines(mSkipLines))
    {
      SetStatusKeyword(READ_FAILURE);
      Message("No results returned");
      return(0);
    }

  // The output data structure
  CreateOutputStruct();
  
  mReadAsCsv = CsvKeywordSet();
  GetScanFormats(mReadAsCsv);


  // Loop over the mRowsToGet
  IDL_MEMINT row2get, current_row=0, rows2skip;

  for (IDL_MEMINT irow2get=0; irow2get < mNumRowsToGet; irow2get++)
    {

      row2get = mRowsToGet[irow2get];

      // Are we skipping any rows? If not, then we just read
      if (current_row < row2get)
	{

	  // If not the first row, we must also grab the \n from
	  // the last line since it will be gobbled by scanf
	  if (irow2get == 0)
	    rows2skip = row2get - current_row;
	  else
	    rows2skip = row2get - current_row + 1;
	  if (!SkipLines(rows2skip))
	    {
	      SetStatusKeyword(READ_FAILURE);
	      Message("WARNING: Result will contain data read up to this error for debugging purposes.");
	      return(1);
	    }
	  current_row = row2get;
	}

      // Point to the structure row in memory.
      mpCurrentRow = 
	(char *) mResultVptr->value.s.arr->data + irow2get*( mResultVptr->value.arr->elt_len );

      // Read the row
      if (!ReadRowAsAscii())
	{
	  SetStatusKeyword(READ_FAILURE);
	  Message("WARNING: Result will contain data read up to this error for debugging purposes.");
	  return(1);
	}

      current_row++;

    } // Loop over mRowsToGet

  CloseFile();
  return(1);

}

int FileObj::ReadRowAsAscii()
{
  /* Buffer for reading skipped columns and buffering string data*/
  char buffer[256];
  for (IDL_MEMINT tag=0; tag < mNumColsInFile; tag++ )
    {

      if ( mGetColumn[tag] )
	{

	  // Offset into the struct we are getting; the TagOffsets have bee
	  // reset to point into the new struct if we are getting a substruct
	  char *tptr = mpCurrentRow + mTagInfo.TagOffsets[tag];

	  // Simple if not a string
	  if (mTagInfo.TagDesc[tag]->type != IDL_TYP_STRING)
	    {

	      /* loop over elements */
	      for (IDL_MEMINT i=0; i<mTagInfo.TagNelts[tag]; i++)
		{
		  if (!ScanVal(mTagInfo.TagDesc[tag]->type, tptr))
		    {
		      SetStatusKeyword(READ_FAILURE);
		      return(0);
		    }
		  tptr += mTagInfo.TagBytes[tag];
		}
	    }
	  else 
	    {
	      /* Need to loop over and store each separately in a buffer
		 before copying to struct.  This will work for scalars and
		 arrays of strings. */
	      
	      int nbytes = mTagInfo.TagBytes[tag];
	      IDL_STRING *tStringPtr = (IDL_STRING *) tptr;
	      for (IDL_MEMINT i=0; i<mTagInfo.TagNelts[tag]; i++)
		{
		  // for not csv, we have not read the delimiter yet
		  char c;
		  if (!mReadAsCsv)
		    c = getc(mFptr);

		  // Read the number of expected bytes, and pad with null character
		  IDL_MEMINT j;
		  for (j=0; j<nbytes; j++)
		    buffer[j] = getc(mFptr);
		  buffer[j] = '\0';
		  
		  // Store the string in the IDL variable
		  IDL_StrStore(tStringPtr, buffer);
		  tStringPtr++;

		  // For CSV we will read until we hit the delimiter or EOL/EOF
		  if (mReadAsCsv) 
		    do {
		      c=getc(mFptr);
		    } while (c != ',' && c != '\n' && c != EOF);

		}	      
	    }


	}
      else
	{
	  /* Skipping this column */

	  if (mTagInfo.TagDesc[tag]->type != IDL_TYP_STRING)
	    {
	      
	      /* loop over elements */
	      for (IDL_MEMINT i=0; i<mTagInfo.TagNelts[tag]; i++)
		{
		  if (!ScanVal(mTagInfo.TagDesc[tag]->type, buffer))
		    {
		      SetStatusKeyword(READ_FAILURE);
		      return(0);
		    }
		}
	    }
	  else
	    {
	      int nbytes = mTagInfo.TagBytes[tag];
	      for (IDL_MEMINT i=0;i<mTagInfo.TagNelts[tag]; i++)
		{
		  // for not csv, we have not read the delimiter yet
		  char c;
		  if (!mReadAsCsv)
		    c = getc(mFptr);
		  
		  // Read the number of expected bytes
		  for (IDL_MEMINT j=0; j<nbytes; j++)
		    char c=getc(mFptr);
		  
		  // For CSV we will read until we hit the delimiter or EOL/EOF      
		  if (mReadAsCsv) 
		    do {
		      c=getc(mFptr);
		    } while (c != ',' && c != '\n' && c != EOF);
		  
		}
	    }
	} // Not extracting column

    }

  return(1);

}

int FileObj::ScanVal(int   type, 
		     char* buffer)
{

  int ret = fscanf(mFptr, mScanFormats[type].c_str(), buffer);
  if (ret != 1)
    {
      if (feof(mFptr))
	Message("ScanVal: EOF reached unexpectedly");
      else
	{
	  char mess[50];
	  sprintf(mess,"ScanVal: Read error: %d",ret);
	  Message(mess);
	}
      return(0);
    }

  return(1);
}


void FileObj::GetScanFormats(IDL_LONG csv)
{

  mScanFormats.resize(16);
  if (csv) 
    {

      /* Need spaces before comma to allow for them in file 
	 This works because scanf treats blanks as zero or more
	 white space */
      mScanFormats[0] = "NONE"; // Undef
      mScanFormats[1] = "%d ,"; // idl_byte
      mScanFormats[2] = "%d ,"; // idl_int
      mScanFormats[3] = "%d ,"; // idl_long
      mScanFormats[4] = "%f ,"; // idl_float
      mScanFormats[5] = "%lf ,"; // idl_double
      mScanFormats[6] = "NONE"; // idl_complex
      mScanFormats[7] = "%[^,],"; // idl_string
      mScanFormats[8] = "NONE"; // idl_struct
      mScanFormats[9] = "NONE"; // idl_double_complex
      mScanFormats[10] = "NONE"; // idl_ptr
      mScanFormats[11] = "NONE"; // idl_objref
      mScanFormats[12] = "%u ,"; // idl_uint
      mScanFormats[13] = "%u ,"; // idl_ulong
      mScanFormats[14] = "%Ld ,"; // idl_long64
      mScanFormats[15] = "%Lu ,"; // idl_ulong64

    }
  else
    {

      mScanFormats[0] = "NONE"; // Undef
      mScanFormats[1] = "%d"; // idl_byte
      mScanFormats[2] = "%d"; // idl_int
      mScanFormats[3] = "%d"; // idl_long
      mScanFormats[4] = "%f"; // idl_float
      mScanFormats[5] = "%lf"; // idl_double
      mScanFormats[6] = "NONE"; // idl_comples
      mScanFormats[7] = "%s"; // idl_string
      mScanFormats[8] = "NONE"; // idl_struct
      mScanFormats[9] = "NONE"; // idl_double_complex
      mScanFormats[10] = "NONE"; // idl_ptr
      mScanFormats[11] = "NONE"; // idl_objref
      mScanFormats[12] = "%u"; // idl_uint
      mScanFormats[13] = "%u"; // idl_ulong
      mScanFormats[14] = "%Ld"; // idl_long64
      mScanFormats[15] = "%Lu"; // idl_ulong64

    }


}




/////////////////////////////////////////////////////////////////////////////
//
// Write the input structure as Ascii
//
/////////////////////////////////////////////////////////////////////////////


int FileObj::WriteAsAscii()
{

  // Only this can result in no returned data at this point.
  string mode="w";
  if (kw.append_there)
    if (kw.append != 0) mode="a";

  if (!OpenFile((char *) mode.c_str())) 
    {
      SetStatusKeyword(READ_FAILURE);
      Message("No results returned");
      return(0);
    }

  // Set the delimiter 
  mWriteAsCsv = CsvKeywordSet();
  if (mWriteAsCsv != 0)
    {
      mDelim=",";
    }
  else
    {
      if (kw.delimiter_there)
	mDelim = IDL_STRING_STR(&kw.delimiter);
      else
	mDelim = "\t";
    }
  // Get a pointer to the delim
  mpDelim = (char *) mDelim.c_str();


  // Should we place brackets around the arrays?
  if ( (kw.bracket_arrays_there) && (kw.bracket_arrays != 0) )
    {
      mBracketArrays = 1;
      mArrayDelim = ",";
    }
  else 
    {
      mBracketArrays = 0;
      mArrayDelim = mDelim;
    }
  mpArrayDelim = (char *) mArrayDelim.c_str();


  IDL_MEMINT nrows = (IDL_MEMINT) mStructdefVptr->value.s.arr->n_elts;
  for (IDL_MEMINT row=0; row<nrows; row++)
    {
      // Process the fields for this row
      for (IDL_MEMINT tag=0; tag<mTagInfo.NumTags;tag++)
	{

	  /* Arrays */
	  if ((mTagInfo.TagDesc[tag]->flags & IDL_V_ARR) != 0) 
	    {

	      if (mBracketArrays) 
		fprintf(mFptr,"{");

	      for (IDL_MEMINT el=0;el<mTagInfo.TagNelts[tag];el++)
		{
		  UCHAR* tptr = (mStructdefVptr->value.s.arr->data +
				row*mStructdefVptr->value.arr->elt_len + 
				mTagInfo.TagOffsets[tag] + 
				el*mTagInfo.TagDesc[tag]->value.arr->elt_len
				);
		  AsciiPrint(mTagInfo.TagDesc[tag]->type, tptr);
		  if (el < mTagInfo.TagNelts[tag]-1)
		    fprintf(mFptr, mpArrayDelim);

		}
	      if (mBracketArrays) 
		fprintf(mFptr,"}");

	    }
	  else 
	    {
	      UCHAR* tptr = (mStructdefVptr->value.s.arr->data +
			    row*mStructdefVptr->value.arr->elt_len + 
			    mTagInfo.TagOffsets[tag]
			    );
	      AsciiPrint(mTagInfo.TagDesc[tag]->type, tptr);
	    }

	  if (tag < mTagInfo.NumTags-1) 
	    fprintf(mFptr, mpDelim);
	  else
	    fprintf(mFptr, "\n");


	} // loop over tags
    } // loop over rows


  CloseFile();
  return(1);
}




void
FileObj::AsciiPrint(int    idlType, 
		    UCHAR* tptr)
{

  switch(idlType)
    {
    case IDL_TYP_FLOAT: 
      fprintf(mFptr, "%g", *(float *)tptr);
      break;
    case IDL_TYP_DOUBLE:
      fprintf(mFptr, "%15.8e", *(double *)tptr);
      break;
    case IDL_TYP_BYTE:
      fprintf(mFptr, "%d", *(short *)tptr);
      break;
    case IDL_TYP_INT:
      fprintf(mFptr, "%d", *(short *)tptr);
      break;
    case IDL_TYP_UINT:
      fprintf(mFptr, "%d", *(unsigned short *)tptr);
      break;
    case IDL_TYP_LONG:
      fprintf(mFptr, "%d", *(IDL_LONG *)tptr);
      break;
    case IDL_TYP_ULONG:
      fprintf(mFptr, "%d", *(IDL_ULONG *)tptr);
      break;
    case IDL_TYP_LONG64:
      fprintf(mFptr, "%lld", *(IDL_LONG64 *)tptr);
      break;
    case IDL_TYP_ULONG64:
      fprintf(mFptr, "%lld", *(IDL_ULONG64 *)tptr);
      break;
    case IDL_TYP_STRING:
      fprintf(mFptr, "%s", ( (IDL_STRING *)tptr )->s);
      break;
    default: 
      printf("Unsupported type %d found\n", idlType);
      fflush(stdout);
      break;
    }
}























/////////////////////////////////////////////////////////////////////////////
//
// Check if a row array has been input.  Select the unique elements within
// allowed range
//
/////////////////////////////////////////////////////////////////////////////

int FileObj::GetInputRows() 
{
  if (kw.rows_there)
    {

      IDL_ENSURE_SIMPLE(kw.rows);

      int rowsConverted = 0;
      if (kw.rows->type != IDL_TYP_MEMINT)
	rowsConverted = 1;

      // Will return same variable if it is already right type
      // We have to explicitly release this mem; but at least IDL
      // will warn us if we forget
      IDL_VPTR rowsV = IDL_CvtMEMINT(1, &kw.rows);

      IDL_MEMINT *rowsPtr;
      IDL_MEMINT nrows; // n_elements in the input row array; limited to IDL_MEMINT

      // The true will ensure simple, which we have already done 
      // WARNING: Need to only *return* as many rows as are allowed by the IDL_MEMINT
      // size for the current machine. 
      IDL_VarGetData(rowsV, &nrows, (char **) &rowsPtr, IDL_TRUE);

      if (nrows > 0)
	{

	  for (IDL_MEMINT i=0; i< nrows; i++)
	    {
	      if (rowsPtr[i] > 0 && rowsPtr[i] < mNumRowsInFile)
		mRowsToGet.push_back(rowsPtr[i]);
	    }

	  if (rowsConverted) IDL_Deltmp(rowsV);


	  if (mRowsToGet.size() > 0)
	    {
	      // Sort input rows
	      sort(mRowsToGet.begin(), mRowsToGet.end());
	      // This pushes the dups to the back, so we will have to 
	      // count from the iterator
	      vector<IDL_MEMINT>::iterator 
		last_row = unique(mRowsToGet.begin(), mRowsToGet.end());

	      // count unique rows
	      mNumRowsToGet=0;
	      vector<IDL_MEMINT>::iterator row_iter;
	      for (row_iter=mRowsToGet.begin(); row_iter<last_row; row_iter++)
		mNumRowsToGet++;

	      // some bug with printf here, so used cout
	      if (mVerbose)
		cout << "Extracting " << mNumRowsToGet << "/" << mNumRowsInFile << " rows" << endl;
	      return(1);
	    }
	  else
	    {
	      // No good rows were found.  We will exit with an error.
	      Message("None of input rows is within allowed range");
	      SetStatusKeyword(INPUT_ERROR);
	      mStatus = FILEOBJ_INIT_ERROR;
	      return(0);
	    }

	}
      else
	{
	  // No actual rows were sent
	  if (rowsConverted) IDL_Deltmp(rowsV);
	  // We continue on here
	}

    }

  // We get here if rows keyword not present or n_elements() is zero
  if (mVerbose) printf("Extracting all rows\n");
  mNumRowsToGet = mNumRowsInFile;
  mRowsToGet.resize(mNumRowsInFile);
  for (IDL_MEMINT i=0;i<mNumRowsInFile;i++)
    mRowsToGet[i] = i;
  
  return(1);
}

/////////////////////////////////////////////////////////////////////////////
//
// Check if a column array has been input.  Select the unique elements within
// allowed range
//
/////////////////////////////////////////////////////////////////////////////


int FileObj::GetInputColumns() 
{

  // This will be 1 for use a column, 0 for not
  if (kw.columns_there)
    {

      if (kw.columns->type == IDL_TYP_STRING)
	{

	  //-----------------------------------------------------------
	  // The input columns keyword is in the form of strings
	  //-----------------------------------------------------------

	  if (mVerbose >= 2) 
	    cout << "Entered columns is an IDL_STRING" << endl;

	  IDL_MEMINT n_cols;
	  IDL_STRING *pcols;
	  IDL_VarGetData(kw.columns, &n_cols, (char **) &pcols, IDL_TRUE);

	  if (n_cols > 0)
	    {

	      // Start with getting no columns
	      mGetColumn.resize(mNumColsInFile, 0);
	      mNumColsToGet = 0;
	      // Loop over the input column names and see of they match the
	      // names in our structure definition
	      string name;
	      for (IDL_MEMINT i=0; i<n_cols; i++)
		{
		  name = pcols[i].s;
		  // Make upper case
		  std::transform(name.begin(), name.end(), 
				 name.begin(), (int(*)(int)) toupper);

		  // Does this name match our input structure?
		  if (mTagInfo.TagMap.find(name) != mTagInfo.TagMap.end())
		    {
		      IDL_MEMINT tag = mTagInfo.TagMap[name];
		      // Don't count dups.
		      if (mGetColumn[tag] != 1) 
			mNumColsToGet++;
		      mGetColumn[tag] = 1;
		    }
		}

	      if (mNumColsToGet == 0)
		{
		  // No good rows were found.  We will exit with an error.
		  Message("None of input column names matches input structure definition");
		  SetStatusKeyword(INPUT_ERROR);
		  mStatus = FILEOBJ_INIT_ERROR;
		  return(0);
		}

	      return(1);

	    } // If n_cols == 0 then we continue on and use all columns
	} // String input
      else 
	{

	  //-----------------------------------------------------------
	  // The input columns keyword is numerical
	  //-----------------------------------------------------------

	  IDL_ENSURE_SIMPLE(kw.columns);
	  
	  if (mVerbose >= 2) 
	    cout << "Entered columns is numerical" << endl;

	  int colsConverted = 0;
	  if (kw.columns->type != IDL_TYP_MEMINT)
	    colsConverted = 1;
	  
	  // Will return same variable if it is already type IDL_MEMINT
	  // We have to explicitly release this mem; but at least IDL
	  // will warn us if we forget
	  IDL_VPTR colsV = IDL_CvtMEMINT(1, &kw.columns);
	  
	  IDL_MEMINT *pcols, n_cols;
	  
	  // The true will ensure simple, which we have already done 
	  IDL_VarGetData(colsV, &n_cols, (char **) &pcols, IDL_TRUE);
	  
	  if (n_cols > 0)
	    {
	      
	      // Zero (dont' get column) unless found in input column vector
	      mGetColumn.resize(mNumColsInFile, 0);
	      mNumColsToGet = 0;
	      for (IDL_MEMINT i=0; i<n_cols;i++)
		{
		  if (pcols[i] >= 0 && pcols[i] < mNumColsInFile)
		    {
		      // Don't count dups
		      if (mGetColumn[ pcols[i] ] != 1) 
			mNumColsToGet++;
		      mGetColumn[ pcols[i] ] = 1;
		      
		    }
		}
	      
	      if (colsConverted)
		IDL_Deltmp(colsV);
	      
	      if (mNumColsToGet == 0)
		{
		  // No good rows were found.  We will exit with an error.
		  Message("None of input columns is within allowed range");
		  SetStatusKeyword(INPUT_ERROR);
		  mStatus = FILEOBJ_INIT_ERROR;
		  return(0);
		}
	      
	      return(1);
	    }
	  else
	    {
	      if (colsConverted)
		IDL_Deltmp(colsV);
	      // We continue on here
	    }

	} // Numerical input

    }

  // We get here if columns keyword not present or n_elements() is zero
  mNumColsToGet = mNumColsInFile;
  mGetColumn.resize(mNumColsInFile, 1);
  return(1);
}





/////////////////////////////////////////////////////////////////////////////
//  The size and names of IDL types from their code
/////////////////////////////////////////////////////////////////////////////

int FileObj::IDLTypeNbytes(int type)
{

  switch(type)
    {
    case IDL_TYP_UNDEF: return(0);
    case IDL_TYP_BYTE: return(1);
    case IDL_TYP_INT: return(2);
    case IDL_TYP_LONG: return(4);
    case IDL_TYP_FLOAT: return(4);
    case IDL_TYP_DOUBLE: return(8);
    case IDL_TYP_COMPLEX: return(4);
    case IDL_TYP_STRING: return(-1);
    case IDL_TYP_STRUCT: return(-1);
    case IDL_TYP_DCOMPLEX: return(8);
    case IDL_TYP_PTR: return(-1);
    case IDL_TYP_OBJREF: return(-1);
    case IDL_TYP_UINT: return(2);
    case IDL_TYP_ULONG: return(4);
    case IDL_TYP_LONG64: return(8);
    case IDL_TYP_ULONG64: return(8);
    default: printf("Unsupported type %d found\n",type); break;
    }
}


void FileObj::PrintIdlType(int type)
{

  switch(type)
    {
    case IDL_TYP_UNDEF: printf("UNDEF"); break;
    case IDL_TYP_BYTE: printf("BYTE"); break;
    case IDL_TYP_INT: printf("INT"); break;
    case IDL_TYP_LONG: printf("LONG"); break;
    case IDL_TYP_FLOAT: printf("FLOAT"); break;
    case IDL_TYP_DOUBLE: printf("DOUBLE"); break;
    case IDL_TYP_COMPLEX: printf("COMPLEX"); break;
    case IDL_TYP_STRING: printf("STRING"); break;
    case IDL_TYP_STRUCT: printf("STRUCT"); break;
    case IDL_TYP_DCOMPLEX: printf("DCOMPLEX"); break;
    case IDL_TYP_PTR: printf("PTR"); break;
    case IDL_TYP_OBJREF: printf("OBJREF"); break;
    case IDL_TYP_UINT: printf("UINT"); break;
    case IDL_TYP_ULONG: printf("ULONG"); break;
    case IDL_TYP_LONG64: printf("LONG64"); break;
    case IDL_TYP_ULONG64: printf("ULONG64"); break;
    default: printf("Unsupported type %d found\n",type); break;
    }
}


int
FileObj::CsvKeywordSet()
{
  int csv_set=0;
  if (kw.csv_there)
    if (kw.csv != 0) csv_set=1;

  return(csv_set);
}



/////////////////////////////////////////////////////////////////////////////
// Set the status keyword
/////////////////////////////////////////////////////////////////////////////

void
FileObj::SetStatusKeyword(int statusVal)
{
  if (kw.status_there) {
    /* This frees any existing memory and sets type to INT with value zero */
    IDL_StoreScalarZero(kw.status, IDL_TYP_INT);
    kw.status->value.i = statusVal;
  }
}

/////////////////////////////////////////////////////////////////////////////
// Return the INTERNAL status (not the keyword value).  This is simpley
// 1 or 0
/////////////////////////////////////////////////////////////////////////////

int FileObj::Status()
{
  return(mStatus);
}

/////////////////////////////////////////////////////////////////////////////
// Send messages through the IDL message stack.
/////////////////////////////////////////////////////////////////////////////

void FileObj::Message(char *text)
{
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_INFO, text);
}


////////////////////////////////////////////////////////////////////////////
//
// Syntax statements
//
//
////////////////////////////////////////////////////////////////////////////

int FileObj::NumParams()
{
  return(mNumParams);
}

void FileObj::BinaryReadSyntax()
{

  if (mHelp != 0)
    {
      string docstring = "/*---------------------------------------------------------------------------\n  NAME:\n    binary_read\n  \n  CALLING SEQUENCE:\n    IDL> struct = binary_read(file/lun, structdef, numrows, \n                              rows=, columns=, skiplines=, status=, verbose=, /help)\n\n  PURPOSE:\n\n    Read unformatted binary data from a file into a structure.  The user inputs\n    the structure definition that describes each \"row\" of the file.  The user\n    can extract certain columns and rows by number. For very large files, this\n    is a big advantage over the the IDL builtin procedure readu which can only\n    read contiguous chunks.  The return variable is a structure containing the\n    requested data.  Variable length columns are not currently supported.\n\n    The columns of the input file must be fixed length, and this includes\n    strings; this length must be represented in the input structure definition.\n    \n    The user can send the file name or the file unit opened from IDL in which\n    case the file must be opened with the /stdio keyword and bufsize=0. Lines\n    can be skipped using the skiplines= keyword.\n    \n\n    In general, due to the complex inputs and the fact that most files will\n    have a header describing the data, this program will be used as a utility\n    program and an IDL wrapper will parse the header and format the structure\n    definition.\n\n    This program is written in C and is linked to IDL via the DLM mechanism.\n\n  INPUTS: \n     file/lun: Filename or file unit. For string file names, the user must \n               expand all ~ or other environment variables.  If the file\n	       unit is entered, the file must be opened with the appropriate \n	       keywords:\n                 openr, lun, file, /get_lun, /stdio, bufsize=0\n     structdef: A structure that describes the layout of the data in each row.\n                Variable length fields are not supported.\n     numrows: Number of rows in the file.\n\n  OPTIONAL INPUTS:\n     rows=: An array or scalar of unique rows to read\n     columns=: An array or scalar of unique columns numbers to extract.\n     skiplines=: The number of lines to skip.  The newline character is searched\n         for, so be careful.  This is useful if there is a text header but not\n	 well defined otherwise.\n     verbose=: 0 for standard quiet. 1 for Basic info. > 1 for debug mode.\n     /help: Print this message, full documentation.\n\n  OPTIONAL OUTPUTS:\n    status=: The status of the read. 0 for success, 1 for read failure, \n             2 for input errors such as bad file unit.\n\n  TODO:\n\n    Might write support for variable length columns, such as for strings.  This\n    would need a binary_write.c to write them properly.  Would probably require\n    the user to specify which columns are variable and the first n bytes of the\n    field to describe the length. One byte supportes strings of length 255, two\n    bytes would support 65,535 length strings, four 4,294,967,295\n\n  REVISION HISTORY:\n    Created 20-April-2006: Erin Sheldon, NYU\n    Converted to C++, 2006-July-17, E.S. NYU\n\n\n  Copyright (C) 2005  Erin Sheldon, NYU.  erin dot sheldon at gmail dot com\n\n    This program is free software; you can redistribute it and/or modify\n    it under the terms of the GNU General Public License as published by\n    the Free Software Foundation; either version 2 of the License, or\n    (at your option) any later version.\n\n    This program is distributed in the hope that it will be useful,\n    but WITHOUT ANY WARRANTY; without even the implied warranty of\n    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n    GNU General Public License for more details.\n\n    You should have received a copy of the GNU General Public License\n    along with this program; if not, write to the Free Software\n    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA\n\n\n  ---------------------------------------------------------------------------*/";
      cout << docstring << endl;
    }
  else
    {
      printf("\n");
      printf("   struct = binary_read(file/lun, structdef, numrows, \n");
      printf("                        rows=, columns=, skiplines=, status=, verbose=, /help)\n");
      printf("\n");
      printf("   Send /help for full documentation.\n");
    }

  fflush(stdout);

}

void FileObj::AsciiReadSyntax()
{

  if (mHelp != 0)
    {
      string docstring = "/*---------------------------------------------------------------------------\n  NAME:\n    ascii_read\n\n  CALLING SEQUENCE:\n    IDL> struct = ascii_read(file/lun, structdef, numrows, \n                             rows=, columns=, skiplines=, /csv, status=, verbose=, /help)\n  \n  PURPOSE: \n\n    Read ASCII data from file into a structure.  The file can be white space or\n    comma separated value (CSV). The structdef input provides the definition\n    describing each row of the file. Particular rows or columns may be\n    extracted by number. For very large files, this is a big advantage over the\n    IDL readf procedure which can only read contiguous chunks.  The return\n    variable is a structure containing the requested data.\n\n    Unlike binary_read, the columns may be variable length and the user can\n    input the string columns in structdef with any size because the memory will\n    be generated on the fly.  E.g. structdef = {a:0L, b:'', c:0LL}. String\n    columns are currently limited to 255 bytes.\n\n    Either the file name or an IDL file unit may be sent.  When a file unit is\n    sent, the must be opened with the /stdio keyword and bufsize=0. Lines can\n    be skipped using the skiplines= keyword.\n\n    In general, due to the complex inputs and the fact that most files will\n    have a header describing the data, this program will be used as a utility\n    program and an IDL wrapper will parse the header and format the structure\n    definition.\n\n    This program is written in C++ and is linked to IDL via the DLM mechanism.\n\n\n  INPUTS: \n     file/lun: Filename or file unit. For string file names, the user must \n               expand all ~ or other environment variables.  If the file\n	       unit is entered, the file must be opened with the appropriate \n	       keywords:\n                 openr, lun, file, /get_lun, /stdio, bufsize=0\n\n     structdef: A structure that describes the layout of the data in each row.\n                Variable length fields are not supported.\n     numrows: Number of rows in the file.\n\n  OPTIONAL INPUTS:\n     rows=: An array or scalar of unique rows to read\n     skiplines=: The number of lines to skip.  The newline character is searched\n         for, so be careful.  This is useful if there is a text header but not\n	 well defined otherwise.\n     columns=: An array or scalar of unique columns numbers to extract.\n     /csv: The file is formatted as comma separated value.  The fields cannot \n           contain commas in this case.\n     verbose=: 0 for standard quiet. 1 for basic info. > 1 for debug mode.\n     /help: print this message.\n\n  OPTIONAL OUTPUTS:\n    status=: The status of the read. 0 for success, 1 for read failure, \n             2 for input errors such as bad file unit.\n\n  REVISION HISTORY:\n    created 20-April-2006: Erin Sheldon, NYU\n    Converted to C++, 2006-July-17, E.S. NYU\n\n\n  Copyright (C) 2005  Erin Sheldon, NYU.  erin dot sheldon at gmail dot com\n\n    This program is free software; you can redistribute it and/or modify\n    it under the terms of the GNU General Public License as published by\n    the Free Software Foundation; either version 2 of the License, or\n    (at your option) any later version.\n\n    This program is distributed in the hope that it will be useful,\n    but WITHOUT ANY WARRANTY; without even the implied warranty of\n    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n    GNU General Public License for more details.\n\n    You should have received a copy of the GNU General Public License\n    along with this program; if not, write to the Free Software\n    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA\n\n\n\n  ---------------------------------------------------------------------------*/";
      cout << docstring << endl;
    }
  else
    {
      printf("\n");
      printf("   struct = ascii_read(file/lun, structdef, numrows, \n");
      printf("                       rows=, columns=, skiplines=, /csv, status=, verbose=, /help)\n");
      printf("\n");
      printf("   Send /help for full documentation.\n");
    }

  fflush(stdout);

}

void FileObj::AsciiWriteSyntax()
{

  if (mHelp != 0)
    {
      string docstring = "/*---------------------------------------------------------------------------\n\n  NAME:\n    ascii_write\n  \n  CALLING SEQUENCE:\n    IDL> ascii_write, struct, filename/lun, /append, \n                    /csv, delimiter=, /bracket_arrays, status=, /help\n\n  PURPOSE:\n\n    Write an IDL structure to an ascii file.  This is about 12 times faster than\n    using the built in printf statement for small structure definitions.  For\n    really big ones, getting the offsets is the bottleneck. For a tsObj file\n    its only about 5 times faster.\n\n    This program is written in C++ and is linked to IDL via the DLM mechanism.\n\n  INPUTS: \n     struct: The structure array to write. \n     file/lun: Filename or file unit. For string file names, the user must \n               expand all ~ or other environment variables.  If the file\n	       unit is entered, the file must be opened with the appropriate \n	       keywords:\n                 openr, lun, file, /get_lun, /stdio, bufsize=0\n\n  OPTIONAL INPUTS:\n     /cvs: Use ',' as the field delimiter.\n     delimiter: The field delimiter; default is the tab character.\n     /append: Append the file.\n     /bracket_arrays: {} is placed around array data in each row, with values comma \n        delimited within.  This is the format, with a whitespace delimiter for \n	ordinary fields, is required for file input to postgresql databases.\n     /help: Print this documentation.\n\n  OPTIONAL OUTPUTS:\n    status=: The status of the read. 0 for success, 1 for read failure, \n             2 for input errors such as bad file unit.\n\n\n  REVISION HISTORY:\n    Created December-2005: Erin Sheldon, NYU\n    Converted to C++, 2006-July-17, E.S. NYU\n\n\n  Copyright (C) 2005  Erin Sheldon, NYU.  erin dot sheldon at gmail dot com\n\n    This program is free software; you can redistribute it and/or modify\n    it under the terms of the GNU General Public License as published by\n    the Free Software Foundation; either version 2 of the License, or\n    (at your option) any later version.\n\n    This program is distributed in the hope that it will be useful,\n    but WITHOUT ANY WARRANTY; without even the implied warranty of\n    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n    GNU General Public License for more details.\n\n    You should have received a copy of the GNU General Public License\n    along with this program; if not, write to the Free Software\n    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA\n\n\n  ---------------------------------------------------------------------------*/";
      cout << docstring << endl;
    }
  else
    {
      printf("\n");
      printf("   ascii_write, struct, file/lun, \n");
      printf("              /csv, delimiter=, /bracket_arrays, status=, /help\n");
      printf("\n");
      printf("   Send /help for full documentation.\n");

    }

  fflush(stdout);

}
