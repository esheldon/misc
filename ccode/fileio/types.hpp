#if !defined (_types_hpp)
#define _types_hpp

#include <vector>
#include <map>
#include "export.h"

using namespace std;

typedef struct {

  IDL_MEMINT NumTags;

  vector<string> TagNames;
  map<string,IDL_MEMINT> TagMap;
  vector<IDL_MEMINT> TagOffsets;
  vector<IDL_VPTR> TagDesc;

  IDL_MEMINT BytesPerRow;

  vector<IDL_MEMINT> TagBytes;
  vector<IDL_MEMINT> TagNelts;
  vector<string> buffer;

} TagInfoStruct;


#endif //_types_hpp
