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
  двлее везде предпологается и проверяется что строка читается ОДИН раз!
*/
struct OStr //: public MemoryManagerUnit
{
  // pointer on text
  char* text;
  uint16_t length;
  OStr() : text(0),length(0) {};
  //void dispose() {if ( text ) smartFree(text);}
  void dispose() {if ( text ) delete text; text = 0; length = 0;}
  ~OStr() {/*__trace__(__PRETTY_FUNCTION__); __watch__(text);*/ dispose();}
  operator const char*(){__require__ (length==0 || text!=NULL); return text;}
  const char* cstr(){__require__ (length==0 || text!=NULL); return text;}
  uint16_t size() { __require__ (length==0 || text!=NULL);return length; }
  void copy(const char* src, int src_length) 
  {
    __require__ ( src_length >= 0 );
    __require__ ( src != NULL );
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
  двлее везде предпологается и проверяется что строка читается ОДИН раз!
*/
struct COStr //: public MemoryManagerUnit
{
  // pointer on text
  char* text;
  COStr() : text(0) {};
  //void dispose() {if ( text ) smartFree(text); text = 0;}
  void dispose() {if ( text ) delete text; text = 0;}
  ~COStr(){ /*__trace__(__PRETTY_FUNCTION__); __watch__(text);*/ dispose(); }
  operator const char*(){/*__require__(text!=NULL);*/ return text;}
  const char* cstr(){/*__require__(text!=NULL);*/ return text;}
  uint16_t size(){ return text ? strlen( text ) : 0 ; }
  void copy(const char* src) 
  {
    __require__ ( src != NULL );
    //if ( text ) smartFree(text);
    //if ( text ) delete text;
    dispose();
    
    int length = strlen(src);
    if ( !length  ) return;
    //text = (char*)smartMalloc(length+1);
    text = new char[length+1];
    __require__ ( text != NULL );
    memcpy(text,src,length+1);
    text[length]=0;
  }
	COStr(const COStr& ostr) : text(0)
	{
		copy(ostr.text);
	}
	COStr& operator =(const COStr& ostr)
	{
		copy(ostr.text);
		return *this;
	}
};


};
};

#endif

