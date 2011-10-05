/*
  $Id$
*/


#if !defined  __Cxx_Header__smpp_strings_h__
#define  __Cxx_Header__smpp_strings_h__

#include "util/debug.h"
#include "smpp_memory.h"
#include <string.h>
//#include "smpp_stream.h"

namespace smsc{
namespace smpp{

/**
  Octet String
*/
struct OStr //: public MemoryManagerUnit
{
  // pointer on text
  char* text;
  uint16_t length;
  OStr() : text(0),length(0) {};
  //void dispose() {if ( text ) smartFree(text);}
  char* alloc(int sz)
  {
    text=new char[sz];
    length=sz;
    return text;
  }
  void dispose() {if ( text ) delete [] text; text = 0; length = 0;}
  ~OStr() {/*__trace__(__PRETTY_FUNCTION__); __watch__(text);*/ dispose();}
  operator const char*(){__require__ (length==0 || text!=NULL); return text;}
  const char* cstr(){__require__ (length==0 || text!=NULL); return text;}
  uint16_t size() { __require__ (length==0 || text!=NULL);return length; }
  void copy(const char* src, int src_length)
  {
    //__require__ ( src_length >= 0 );
    //__require__ ( src != NULL );
    //length = 0;
    //if ( text ) smartFree(text);
    //if ( text ) delete text;
    //text = 0;
    dispose();

    if ( !src_length  ) return;

    //text = (char*)smartMalloc(src_length);
    text = new char[src_length];
    __require__ ( text != NULL );
    memcpy(text,src,src_length);
    length = src_length;
  }

  void append(const char* src,int src_length)
  {
    if(src_length==0)return;
    char* newtext=new char[length+src_length+1];
    if(text)memcpy(newtext,text,length);
    memcpy(newtext+length,src,src_length);
    length+=src_length;
    newtext[length]=0;
    if(text)delete [] text;
    text=newtext;
  }

  OStr(const OStr& ostr) : text(0), length(0)
  {
    copy(ostr.text, ostr.length);
  }
  OStr& operator =(const OStr& ostr)
  {
    copy(ostr.text, ostr.length);
    return *this;
  }
};


/**
  C Octet String
*/
struct COStr //: public MemoryManagerUnit
{
  // pointer on text
  char* text;
  COStr() : text(0) {};
  //void dispose() {if ( text ) smartFree(text); text = 0;}
  void dispose()
  {
    if ( text ) delete [] text;
    text = 0;
  }
  ~COStr()
  { /*__trace__(__PRETTY_FUNCTION__); __watch__(text);*/
    dispose();
  }
  operator const char*() const
  {/*__require__(text!=NULL);*/
    return text;
  }
  const char* cstr() const
  {/*__require__(text!=NULL);*/
    return text;
  }
  uint16_t size() const
  {
    return text ? (uint16_t) strlen(text) : 0;
  }
  void copy(const char* src)
  {
//    __require__ ( src != NULL );
    //if ( text ) smartFree(text);
    //if ( text ) delete text;
    dispose();

    size_t length = src?strlen(src):0;
    if ( !length  ) return;
    //text = (char*)smartMalloc(length+1);
    text = new char[length+1];
    __require__ ( text != NULL );
    memcpy(text,src,length+1);
    text[length]=0;
  }
  COStr(const COStr& ostr) : text(0)
  {
    if(ostr.text)copy(ostr.text);
  }
  COStr& operator =(const COStr& ostr)
  {
    if(ostr.text)
      copy(ostr.text);
    else
      text=0;
    return *this;
  }
};


}
}

#endif
