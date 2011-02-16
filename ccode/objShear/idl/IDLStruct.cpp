#include "IDLStruct.h"

/////////////////////////////////////////////////////////////////
//
// Methods for the IDLStruct class
//
/////////////////////////////////////////////////////////////////

IDLStruct::IDLStruct() {} // default constructor: do nothing
IDLStruct::IDLStruct(IDL_VPTR st) { // constructor
  // just call assign method
  assign(st);
  return;
}

int IDLStruct::assign(IDL_VPTR st) { // constructor
  
  int i;
  IDL_VPTR desc;
  char *struct_name;

  // point at the idl structure
  idlStruct = st;
  n_elements = idlStruct->value.arr->n_elts;
  
  num_tags = IDL_StructNumTags(idlStruct->value.s.sdef);
  
  // allocate the vectors
  tag_names.resize(num_tags); 
  tag_offsets.resize(num_tags);
  tag_desc.resize(num_tags);

  for(i=0;i<num_tags;i++)
    {

      tag_names[i] = IDL_StructTagNameByIndex(idlStruct->value.s.sdef, i, 
					     IDL_MSG_LONGJMP, &struct_name);

      tagIndexHash[tag_names[i]] = i;

      tagOffsetsHash[tag_names[i]] = 
	IDL_StructTagInfoByIndex(idlStruct->value.s.sdef, i, IDL_MSG_LONGJMP,
				 &desc);
      tagDescHash[tag_names[i]] = desc; 
     
      tag_offsets[i] = tagOffsetsHash[tag_names[i]];
      tag_desc[i] = desc;

    }

  idlStructName = struct_name;

  //aoffset = 
  //  getTagInfoFromIDLStruct(idlStruct, "A", IDL_TYP_FLOAT, 
  //			    adesc);

  return(1);

}

// number of elements in struct array
IDL_MEMINT IDLStruct::n_elts(){
  return n_elements;
}

IDL_MEMINT IDLStruct::n_tags(){
  return num_tags;
}

// return the name in string form
string IDLStruct::name() {
  return(idlStructName);
}

//////////////////////////////////////////////////////////////////////
// Getting tag info
//////////////////////////////////////////////////////////////////////

// Does this tag exist?
int IDLStruct::tagExists(const char *name)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(0);
  else
    return(1);
}
int IDLStruct::tagExists(int tag_index)
{
  if (tag_index < 0 || tag_index >= num_tags)
    return(0);
  else
    return(1);
}

// Return the type of this tag
short IDLStruct::tagType(const char *name)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(IDL_TYP_UNDEF);
  else
    return(tag_desc[indexIter->second]->type);
}
short IDLStruct::tagType(int tag_index)
{
  if (tag_index < 0 || tag_index >= num_tags)
    return(IDL_TYP_UNDEF);
  else
    return(tag_desc[tag_index]->type);
}

// Return the tag index given the name. If doesn't exist, return -1
int IDLStruct::tagIndex(const char *name)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(-1);
  else
    return(indexIter->second);

  return(indexIter->second);
}

// Return the name given the tag_index
string IDLStruct::tagName(int tag_index)
{
  string name;
  // Is it a valid tag index?
  if (tag_index < 0 || tag_index >= num_tags) 
    name = "__NO_SUCH_TAG__";
  else 
    name = tag_names[tag_index];

  return(name);

}

// Return a copy of the tag_names vector
vector<string> IDLStruct::tagNames()
{
  vector<string> ret(tag_names);
  return(ret);
}

// How many elements in this tag? 1 for scalars, otherwise
// use the description
IDL_MEMINT IDLStruct::tag_n_elts(const char *name)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(0);
  else
    {
      if (tag_desc[indexIter->second]->flags & IDL_V_ARR)
	return(tag_desc[indexIter->second]->value.arr->n_elts);
      else
	return(1);
    }
}
IDL_MEMINT IDLStruct::tag_n_elts(int tag_index)
{
  if (tag_index < 0 || tag_index >= num_tags) 
    return(0);
  else
    {
      if (tag_desc[tag_index]->flags & IDL_V_ARR)
	return(tag_desc[tag_index]->value.arr->n_elts);
      else
	return(1);
    }
}

// Return the offset of a given tag into the opaque IDL data
// structure
IDL_MEMINT IDLStruct::tagOffset(const char *name)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(-1);
  else
    return(tag_offsets[indexIter->second]);
}
IDL_MEMINT IDLStruct::tagOffset(int tag_index)
{
  if (tag_index < 0 || tag_index >= num_tags) 
    return(-1);
  else
    return(tag_offsets[tag_index]);
}

// Return the description of this tag
int IDLStruct::tagDesc(const char *name, IDL_VPTR &desc)
{
  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) 
    return(0);
  else 
    desc = tag_desc[indexIter->second];

  return(1);
}
int IDLStruct::tagDesc(int tag_index, IDL_VPTR &desc)
{

  if (tag_index < 0 || tag_index >= num_tags) 
    return(0);
  else 
    desc = tag_desc[tag_index];

  return(1);
}

/////////////////////////////////////////////////////////////////////////////
// Return a pointer to the requested tag, or field
// This is overloaded for single structs (index=0) and for array
// subscripts
//
// Note using get instead of a hard-wired method for each tag is ~5 times 
// slower when the tag_index is sent and 44 times slower when the name is sent
////////////////////////////////////////////////////////////////////////////


// the zero index case by name (or single structure)
UCHAR *IDLStruct::get(const char *name, int type) {

  static int tag_index;

  // Does this tag exist?  This is the bottleneck, by far
  // Better for the user to find the index ahead of time
  // Slightly faster than doing the ful getRefFromIDLStrucf thing

  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) tagMessage(name);
  tag_index = indexIter->second;

  // Is the type correct? This is not as big a bottleneck as the name lookup
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) typeMessage(name);
#endif

  return( idlStruct->value.s.arr->data + tag_offsets[tag_index]);

}
// the zero index case by tag_index (or single structure)
UCHAR *IDLStruct::get(int tag_index, int type) {

  // Is it a valid tag index?
  if (tag_index < 0 || tag_index >= num_tags) tagMessage(tag_index);

  // Is the type correct? 
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) typeMessage(tag_index);
#endif

  return( idlStruct->value.s.arr->data + tag_offsets[tag_index]);

}

// the zero index case, by name, subscripted
UCHAR *IDLStruct::geta(const char *name, IDL_MEMINT arr_index, int type) {

  static int tag_index;
  static IDL_MEMINT aoffset;

  // Does this tag exist?  This is the bottleneck, by far
  // Better for the user to find the index ahead of time
  // Slightly faster than doing the ful getRefFromIDLStrucf thing

  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) tagMessage(name);
  tag_index = indexIter->second;


  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) typeMessage(name);
#endif

  // get the offset for this subscript. Do range checking
  //aoffset = getSubscriptOffset(tag_index, arr_index);
  if (tag_desc[tag_index]->flags & IDL_V_ARR) 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index < 0 || arr_index >= tag_desc[tag_index]->value.arr->n_elts) 
	arrayRangeMessage(name, arr_index);
      else 
	aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#else
      aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#endif
    } 
  else 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index != 0) 
	scalarRangeMessage(name, arr_index);
      else 
	aoffset = 0;
#else
      aoffset=0;
#endif
    }

  return( idlStruct->value.s.arr->data + tag_offsets[tag_index] + aoffset);

}
// the zero index case, by tag_index, subscripted
UCHAR *IDLStruct::geta(int tag_index, IDL_MEMINT arr_index, int type) {

  static IDL_MEMINT aoffset;

  // Is it a valid tag index?
  if (tag_index < 0 || tag_index >= num_tags) tagMessage(tag_index);

  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) typeMessage(tag_index);
#endif

  // get the offset for this subscript. Do range checking
  // This is a bit faster than calling the getSub...function
  //aoffset = getSubscriptOffset(tag_index, arr_index);

  if (tag_desc[tag_index]->flags & IDL_V_ARR) 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index < 0 || arr_index >= tag_desc[tag_index]->value.arr->n_elts) 
	arrayRangeMessage(tag_index, arr_index);
      else 
	aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#else
      aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#endif
    } 
  else 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index != 0) 
	scalarRangeMessage(tag_index, arr_index);
      else 
	aoffset = 0;
#else
      aoffset=0;
#endif
    }

  return( idlStruct->value.s.arr->data + tag_offsets[tag_index] + aoffset );

}


// subscript the struct as well, by name
UCHAR *IDLStruct::get(IDL_MEMINT index, const char *name, int type) {

  static int tag_index;

  // index in bounds?
#if IDLSTRUCT_RANGE_CHECK
  if (index >= n_elements || index < 0) structRangeMessage(index);
#endif

  // Does this tag exist?  This is the bottleneck, by far
  // Better for the user to find the index ahead of time
  // Slightly faster than doing the ful getRefFromIDLStrucf thing

  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) tagMessage(name);
  tag_index = indexIter->second;

  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) tagMessage(name);
#endif

  return( idlStruct->value.s.arr->data + 
	  index*idlStruct->value.arr->elt_len +
	  tag_offsets[tag_index]);

}

// subscript the struct as well, by tag_index
UCHAR *IDLStruct::get(IDL_MEMINT index, int tag_index, int type) {

  // Is it a valid tag index?
  if (tag_index < 0 || tag_index >= num_tags) tagMessage(tag_index);

  // index in bounds?
#if IDLSTRUCT_RANGE_CHECK
  if (index >= n_elements || index < 0) structRangeMessage(index);
#endif

  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) tagMessage(tag_index);
#endif

  return( idlStruct->value.s.arr->data + 
	  index*idlStruct->value.arr->elt_len +
	  tag_offsets[tag_index]);

}

// subscript the struct AND the field, by tag name
UCHAR *IDLStruct::geta(IDL_MEMINT index, const char *name, 
		      IDL_MEMINT arr_index, int type) {

  static IDL_MEMINT aoffset;
  int tag_index;

  // index in bounds?
#if IDLSTRUCT_RANGE_CHECK
  if (index >= n_elements || index < 0) structRangeMessage(index);
#endif

  // Does this tag exist?  This is the bottleneck, by far
  // Better for the user to find the index ahead of time
  // Slightly faster than doing the ful getRefFromIDLStrucf thing

  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) tagMessage(name);
  tag_index = indexIter->second;

  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) tagMessage(name);
#endif


  // get the offset for this subscript. Do range checking
  //aoffset = getSubscriptOffset(tagIndexHash[name], arr_index);

  if (tag_desc[tag_index]->flags & IDL_V_ARR) 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index < 0 || arr_index >= tag_desc[tag_index]->value.arr->n_elts) 
	arrayRangeMessage(name, arr_index);
      else 
	aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#else
      aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#endif
    } 
  else 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index != 0) 
	scalarRangeMessage(name, arr_index);
      else 
	aoffset = 0;
#else
      aoffset=0;
#endif
    }

  return( idlStruct->value.s.arr->data + 
	  index*idlStruct->value.arr->elt_len +
	  tag_offsets[tag_index] + aoffset);

}

// subscript the struct AND the field, by tag index
UCHAR *IDLStruct::geta(IDL_MEMINT index, int tag_index, 
		      IDL_MEMINT arr_index, int type) {

  static IDL_MEMINT aoffset;

  // Is it a valid tag index?
  if (tag_index < 0 || tag_index >= num_tags) tagMessage(tag_index);

  // index in bounds?
#if IDLSTRUCT_RANGE_CHECK
  if (index >= n_elements || index < 0) structRangeMessage(index);
#endif


  // Is the type correct
#if IDLSTRUCT_TYPECHECK
  if (tag_desc[tag_index]->type != type) tagMessage(tag_index);
#endif


  // get the offset for this subscript. Do range checking
  //aoffset = getSubscriptOffset(tagIndexHash[name], arr_index);

  if (tag_desc[tag_index]->flags & IDL_V_ARR) 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index < 0 || arr_index >= tag_desc[tag_index]->value.arr->n_elts) 
	arrayRangeMessage(tag_index, arr_index);
      else 
	aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#else
      aoffset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#endif
    } 
  else 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index != 0) 
	scalarRangeMessage(tag_index, arr_index);
      else 
	aoffset = 0;
#else
      aoffset=0;
#endif
    }

  return( idlStruct->value.s.arr->data + 
	  index*idlStruct->value.arr->elt_len +
	  tag_offsets[tag_index] + aoffset);

}


////////////////////////////////////////////////////////////////////////
//
// Get generic types by name. Could be slightly faster if we didn't use 
// The "get" functions.  Still much slower than hard-wired. 
//
////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////
// Get Bytes
///////////////////////////////////////////////

// Scalars
UCHAR *IDLStruct::bget(const char *name) {
  return (UCHAR *) get(name, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bget(int tag_index) {
  return (UCHAR *) get(tag_index, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bget(IDL_MEMINT index, const char *name) {
  return (UCHAR *) get(index, name, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bget(IDL_MEMINT index, int tag_index) {
  return (UCHAR *) get(index, tag_index, IDL_TYP_BYTE);
}

// Arrays
UCHAR *IDLStruct::bgeta(const char *name, IDL_MEMINT arr_index) {
  return (UCHAR *) geta(name, arr_index, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bgeta(int tag_index, IDL_MEMINT arr_index) {
  return (UCHAR *) geta(tag_index, arr_index, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bgeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (UCHAR *) geta(index, name, arr_index, IDL_TYP_BYTE);
}
UCHAR *IDLStruct::bgeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (UCHAR *) geta(index, tag_index, arr_index, IDL_TYP_BYTE);
}

///////////////////////////////////////////////
// Get Ints (short)
///////////////////////////////////////////////

// Scalars
short *IDLStruct::iget(const char *name) {
  return (short *) get(name, IDL_TYP_INT);
}
short *IDLStruct::iget(int tag_index) {
  return (short *) get(tag_index, IDL_TYP_INT);
}
short *IDLStruct::iget(IDL_MEMINT index, const char *name) {
  return (short *) get(index, name, IDL_TYP_INT);
}
short *IDLStruct::iget(IDL_MEMINT index, int tag_index) {
  return (short *) get(index, tag_index, IDL_TYP_INT);
}

// Arrays
short *IDLStruct::igeta(const char *name, IDL_MEMINT arr_index) {
  return (short *) geta(name, arr_index, IDL_TYP_INT);
}
short *IDLStruct::igeta(int tag_index, IDL_MEMINT arr_index) {
  return (short *) geta(tag_index, arr_index, IDL_TYP_INT);
}
short *IDLStruct::igeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (short *) geta(index, name, arr_index, IDL_TYP_INT);
}
short *IDLStruct::igeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (short *) geta(index, tag_index, arr_index, IDL_TYP_INT);
}

///////////////////////////////////////////////
// Get UINT (unsigned short)
///////////////////////////////////////////////

// Scalars
IDL_UINT *IDLStruct::uiget(const char *name) {
  return (IDL_UINT *) get(name, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uiget(int tag_index) {
  return (IDL_UINT *) get(tag_index, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uiget(IDL_MEMINT index, const char *name) {
  return (IDL_UINT *) get(index, name, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uiget(IDL_MEMINT index, int tag_index) {
  return (IDL_UINT *) get(index, tag_index, IDL_TYP_UINT);
}

// Arrays
IDL_UINT *IDLStruct::uigeta(const char *name, IDL_MEMINT arr_index) {
  return (IDL_UINT *) geta(name, arr_index, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uigeta(int tag_index, IDL_MEMINT arr_index) {
  return (IDL_UINT *) geta(tag_index, arr_index, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uigeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (IDL_UINT *) geta(index, name, arr_index, IDL_TYP_UINT);
}
IDL_UINT *IDLStruct::uigeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (IDL_UINT *) geta(index, tag_index, arr_index, IDL_TYP_UINT);
}

///////////////////////////////////////////////
// Get IDL_LONG (int)
///////////////////////////////////////////////

// Scalars
IDL_LONG *IDLStruct::lget(const char *name) {
  return (IDL_LONG *) get(name, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lget(int tag_index) {
  return (IDL_LONG *) get(tag_index, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lget(IDL_MEMINT index, const char *name) {
  return (IDL_LONG *) get(index, name, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lget(IDL_MEMINT index, int tag_index) {
  return (IDL_LONG *) get(index, tag_index, IDL_TYP_LONG);
}

// Arrays
IDL_LONG *IDLStruct::lgeta(const char *name, IDL_MEMINT arr_index) {
  return (IDL_LONG *) geta(name, arr_index, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lgeta(int tag_index, IDL_MEMINT arr_index) {
  return (IDL_LONG *) geta(tag_index, arr_index, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lgeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (IDL_LONG *) geta(index, name, arr_index, IDL_TYP_LONG);
}
IDL_LONG *IDLStruct::lgeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (IDL_LONG *) geta(index, tag_index, arr_index, IDL_TYP_LONG);
}

///////////////////////////////////////////////
// Get IDL_ULONG (unsigned int)
///////////////////////////////////////////////

// Scalars
IDL_ULONG *IDLStruct::ulget(const char *name) {
  return (IDL_ULONG *) get(name, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulget(int tag_index) {
  return (IDL_ULONG *) get(tag_index, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulget(IDL_MEMINT index, const char *name) {
  return (IDL_ULONG *) get(index, name, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulget(IDL_MEMINT index, int tag_index) {
  return (IDL_ULONG *) get(index, tag_index, IDL_TYP_ULONG);
}

// Arrays
IDL_ULONG *IDLStruct::ulgeta(const char *name, IDL_MEMINT arr_index) {
  return (IDL_ULONG *) geta(name, arr_index, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulgeta(int tag_index, IDL_MEMINT arr_index) {
  return (IDL_ULONG *) geta(tag_index, arr_index, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulgeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (IDL_ULONG *) geta(index, name, arr_index, IDL_TYP_ULONG);
}
IDL_ULONG *IDLStruct::ulgeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (IDL_ULONG *) geta(index, tag_index, arr_index, IDL_TYP_ULONG);
}

///////////////////////////////////////////////
// Get IDL_LONG64 
///////////////////////////////////////////////

// Scalars
IDL_LONG64 *IDLStruct::l64get(const char *name) {
  return (IDL_LONG64 *) get(name, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64get(int tag_index) {
  return (IDL_LONG64 *) get(tag_index, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64get(IDL_MEMINT index, const char *name) {
  return (IDL_LONG64 *) get(index, name, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64get(IDL_MEMINT index, int tag_index) {
  return (IDL_LONG64 *) get(index, tag_index, IDL_TYP_LONG64);
}

// Arrays
IDL_LONG64 *IDLStruct::l64geta(const char *name, IDL_MEMINT arr_index) {
  return (IDL_LONG64 *) geta(name, arr_index, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64geta(int tag_index, IDL_MEMINT arr_index) {
  return (IDL_LONG64 *) geta(tag_index, arr_index, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64geta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (IDL_LONG64 *) geta(index, name, arr_index, IDL_TYP_LONG64);
}
IDL_LONG64 *IDLStruct::l64geta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (IDL_LONG64 *) geta(index, tag_index, arr_index, IDL_TYP_LONG64);
}

///////////////////////////////////////////////
// Get IDL_ULONG64
///////////////////////////////////////////////

// Scalars
IDL_ULONG64 *IDLStruct::ul64get(const char *name) {
  return (IDL_ULONG64 *) get(name, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64get(int tag_index) {
  return (IDL_ULONG64 *) get(tag_index, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64get(IDL_MEMINT index, const char *name) {
  return (IDL_ULONG64 *) get(index, name, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64get(IDL_MEMINT index, int tag_index) {
  return (IDL_ULONG64 *) get(index, tag_index, IDL_TYP_ULONG64);
}

// Arrays
IDL_ULONG64 *IDLStruct::ul64geta(const char *name, IDL_MEMINT arr_index) {
  return (IDL_ULONG64 *) geta(name, arr_index, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64geta(int tag_index, IDL_MEMINT arr_index) {
  return (IDL_ULONG64 *) geta(tag_index, arr_index, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64geta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (IDL_ULONG64 *) geta(index, name, arr_index, IDL_TYP_ULONG64);
}
IDL_ULONG64 *IDLStruct::ul64geta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (IDL_ULONG64 *) geta(index, tag_index, arr_index, IDL_TYP_ULONG64);
}


///////////////////////////////////////////////
// Get Floats
///////////////////////////////////////////////

// Scalars
float *IDLStruct::fget(const char *name) {
  return (float *) get(name, IDL_TYP_FLOAT);
}
float *IDLStruct::fget(int tag_index) {
  return (float *) get(tag_index, IDL_TYP_FLOAT);
}
float *IDLStruct::fget(IDL_MEMINT index, const char *name) {
  return (float *) get(index, name, IDL_TYP_FLOAT);
}
float *IDLStruct::fget(IDL_MEMINT index, int tag_index) {
  return (float *) get(index, tag_index, IDL_TYP_FLOAT);
}

// Arrays
float *IDLStruct::fgeta(const char *name, IDL_MEMINT arr_index) {
  return (float *) geta(name, arr_index, IDL_TYP_FLOAT);
}
float *IDLStruct::fgeta(int tag_index, IDL_MEMINT arr_index) {
  return (float *) geta(tag_index, arr_index, IDL_TYP_FLOAT);
}
float *IDLStruct::fgeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (float *) geta(index, name, arr_index, IDL_TYP_FLOAT);
}
float *IDLStruct::fgeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (float *) geta(index, tag_index, arr_index, IDL_TYP_FLOAT);
}

///////////////////////////////////////////////
// Get Doubles
///////////////////////////////////////////////

// Scalars
double *IDLStruct::dget(const char *name) {
  return (double *) get(name, IDL_TYP_DOUBLE);
}
double *IDLStruct::dget(int tag_index) {
  return (double *) get(tag_index, IDL_TYP_DOUBLE);
}
double *IDLStruct::dget(IDL_MEMINT index, const char *name) {
  return (double *) get(index, name, IDL_TYP_DOUBLE);
}
double *IDLStruct::dget(IDL_MEMINT index, int tag_index) {
  return (double *) get(index, tag_index, IDL_TYP_DOUBLE);
}

// Arrays
double *IDLStruct::dgeta(const char *name, IDL_MEMINT arr_index) {
  return (double *) geta(name, arr_index, IDL_TYP_DOUBLE);
}
double *IDLStruct::dgeta(int tag_index, IDL_MEMINT arr_index) {
  return (double *) geta(tag_index, arr_index, IDL_TYP_DOUBLE);
}
double *IDLStruct::dgeta(IDL_MEMINT index, const char *name, 
		       IDL_MEMINT arr_index) {
  return (double *) geta(index, name, arr_index, IDL_TYP_DOUBLE);
}
double *IDLStruct::dgeta(IDL_MEMINT index, int tag_index, 
		       IDL_MEMINT arr_index) {
  return (double *) geta(index, tag_index, arr_index, IDL_TYP_DOUBLE);
}

// For testing hard-wired get

float *IDLStruct::a()
{
  return (float *) ( idlStruct->value.s.arr->data + aoffset );
}

float *IDLStruct::a(IDL_MEMINT arr_index)
{
  return (float *) ( idlStruct->value.s.arr->data + 
		     aoffset + 
		     arr_index*adesc->value.arr->elt_len);
}



///////////////////////////////////////////////////////////
// Return an IDL_VPTR 
///////////////////////////////////////////////////////////

IDL_VPTR IDLStruct::vget(const char *name)
{

  IDL_VPTR tmp;
  UCHAR *p;
  char message[80];
  static int tag_index;

  // Does this tag exist?  This is the bottleneck, by far
  // Better for the user to find the index ahead of time
  // Slightly faster than doing the ful getRefFromIDLStrucf thing

  indexIter = tagIndexHash.find(name);
  if (indexIter == tagIndexHash.end()) tagMessage(name);
  tag_index = indexIter->second;

  // point at the data
  p = ( idlStruct->value.s.arr->data + tag_offsets[tag_index] );


  // for arrays. Problem is we still have to case the result
  if (tag_desc[tag_index]->flags & IDL_V_ARR)
    {
      // should copy the desc pointer efficiency
      tmp = IDL_ImportArray(tag_desc[tag_index]->value.arr->n_dim,
			    tag_desc[tag_index]->value.arr->dim,
			    tag_desc[tag_index]->type,
			    p,
			    NULL,NULL);
    }
  else // scalars: what if we want to modify the value? Should we just import 
    //as array anyway?  But how if the .arr is not set?
    {

      // This could be an issue
      tmp = IDL_Gettmp();

      tmp->type = tagDescHash[name]->type;
      switch (tagDescHash[name]->type) {
      case IDL_TYP_BYTE:    tmp->value.c =    *(UCHAR *)p;       break;
      case IDL_TYP_INT:     tmp->value.i =    *(short *)p;       break;
      case IDL_TYP_UINT:    tmp->value.ui =   *(IDL_UINT *)p;    break;
      case IDL_TYP_LONG:    tmp->value.l =    *(IDL_LONG *)p;    break;
      case IDL_TYP_ULONG:   tmp->value.ul =   *(IDL_ULONG *)p;   break;
      case IDL_TYP_LONG64:  tmp->value.l64 =  *(IDL_LONG64 *)p;  break;
      case IDL_TYP_ULONG64: tmp->value.ul64 = *(IDL_ULONG64 *)p; break;
      case IDL_TYP_FLOAT:   tmp->value.f =    *(float *)p;       break;
	//case IDL_TYP_FLOAT: IDL_StoreScalar(&tmp, IDL_TYP_FLOAT
      case IDL_TYP_DOUBLE:  tmp->value.d =    *(double *)p;      break;
      case IDL_TYP_UNDEF: IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
				      "TYPE is undefined");
      default: IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
				      "Cannot work with type");
      }

    }

  return(tmp);

}

////////////////////////////////////////////////////////////////
// Subscript checking for arrays
////////////////////////////////////////////////////////////////

IDL_MEMINT 
IDLStruct::getSubscriptOffset(int tag_index, IDL_MEMINT arr_index)
  
{
  static IDL_MEMINT offset;

  // Is the subscript in bounds?
  // If its a scalar, but arr_index is 0 this is also OK

  if (tag_desc[tag_index]->flags & IDL_V_ARR) 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index < 0 || arr_index >= tag_desc[tag_index]->value.arr->n_elts) 
        arrayRangeMessage(tag_index, arr_index);
      else 
	offset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#else
      offset = arr_index*tag_desc[tag_index]->value.arr->elt_len;
#endif
    } 
  else 
    {
#if IDLSTRUCT_RANGE_CHECK
      if (arr_index != 0) 
	scalarRangeMessage(tag_index, arr_index);
      else 
	offset = 0;
#else
      offset=0;
#endif
    }

  return(offset);
}

////////////////////////////////////////////////////////////////
// IDLStruct Messages
////////////////////////////////////////////////////////////////

void IDLStruct::tagMessage(const char *name)
{
  stringstream message;
  message << "Tag \"" << name << "\" does not exist";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());  
}
void IDLStruct::tagMessage(int tag_index)
{
  stringstream message;
  message << "tag_index " << tag_index << " is out of range";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}

void IDLStruct::typeMessage(const char *name)
{
  stringstream message;
  message << "Type mismatch found for tag \"" << name << "\"";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}
void IDLStruct::typeMessage(int tag_index)
{
  stringstream message;
  message << "Type mismatch found for tag index " << tag_index;
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}


void IDLStruct::structRangeMessage(IDL_MEMINT index)
{
  stringstream message;
  message << "Attempt to subscript struct " << idlStructName << 
    " with index " << index << 
    " is out of bounds [0, " << idlStruct->value.arr->n_elts-1 << "]";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}

void IDLStruct::scalarRangeMessage(const char *name, IDL_MEMINT arr_index)
{
  stringstream message;
  message << "Attempt to subscript scalar \"" << name << 
    "\" with index " << arr_index << " != 0";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}
void IDLStruct::scalarRangeMessage(int tag_index, IDL_MEMINT arr_index)
{
  stringstream message;
  message << "Attempt to subscript scalar tag_index " << tag_index <<
    " with index " << arr_index << " != 0";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}

void IDLStruct::arrayRangeMessage(const char *name, IDL_MEMINT arr_index)
{
  stringstream message;
  message << "Attempt to subscript array \"" << name << 
    "\" with index " << arr_index << 
    " is out of bounds [0, " << tagDescHash[name]->value.arr->n_elts-1 << "]";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}
void IDLStruct::arrayRangeMessage(int tag_index, IDL_MEMINT arr_index)
{
  stringstream message;
  message << "Attempt to subscript tag_index " << tag_index << 
    " with index " << arr_index << 
    " is out of bounds [0, " << 
    tag_desc[tag_index]->value.arr->n_elts-1 << "]";
  IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, message.str().c_str());
}


//////////////////////////////////////////////////////////////////////////
// A wrapper for IDL_StructTagInfoByName that does type checking
//////////////////////////////////////////////////////////////////////////

IDL_MEMINT getTagInfoFromIDLStruct(IDL_VPTR src, char *name, int type, IDL_VPTR &desc)
{

  IDL_MEMINT offset;
  static char message[80];
    
  offset = IDL_StructTagInfoByName(src->value.s.sdef, name, IDL_MSG_LONGJMP,
				   &desc);
  
  /* make sure we got what we asked for */
  if ( desc->type != type)
    {
      sprintf(message,"Type mismatch found for tag %s", name);
      IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, 
		  message);
    }
  
  return(offset);

}

////////////////////////////////////////////////////////////////////////////////
// This routine access strucure elements by struct index and tag name
// The return value is UCHAR * so it must be cast into the appropriate type
////////////////////////////////////////////////////////////////////////////////

UCHAR *getRefFromIDLStruct(IDL_VPTR src, IDL_MEMINT isrc, 
			   char *name, int type)
{
  
  IDL_VPTR desc;
  static IDL_MEMINT offset;
  static char message[80];
  
  if (isrc >= src->value.arr->n_elts || isrc < 0)
    {
      IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP,
		  "Attempt to subscript structure out of bounds");
    }
  
  offset = IDL_StructTagInfoByName(src->value.s.sdef, name, IDL_MSG_LONGJMP,
				   &desc);
  
  /* make sure we got what we asked for */
  if (desc->type != type)
    {
      sprintf(message,"Type mismatch found for tag %s", name);
      IDL_Message(IDL_M_NAMED_GENERIC, IDL_MSG_LONGJMP, 
		  message);
    }
  
  return( src->value.s.arr->data + 
	  isrc*src->value.arr->elt_len +
	  offset  );
  
}
