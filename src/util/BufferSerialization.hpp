#ifndef BUFFER_SERIALIZATION_DECLARATIONS
#define BUFFER_SERIALIZATION_DECLARATIONS

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include <exception>
#include <string>
#include "core/buffers/TmpBuf.hpp"
#include "util/Uint64Converter.h"

namespace smsc { 
namespace util {

using smsc::core::buffers::TmpBuf;
struct BufferSerializator
{
  static void putByte( uint8_t* &buffer, uint8_t val ) {
    *buffer = val;
    buffer+=sizeof(val);
  }
  
  static void putInt( uint8_t* &buffer, uint32_t val ) {
    uint32_t hval = htonl(val);
    memcpy((void*)(buffer), (const void*)&hval, sizeof(val));
    buffer += sizeof(val) ;
  }
  
  static void putInt64( uint8_t* &buffer, uint64_t val ) {
    uint64_t hval = Uint64Converter::toNetworkOrder(val);
    memcpy((void*)(buffer), (const void*)&hval, sizeof(val));
    buffer += sizeof(val) ;
  }
  
  static void putString(uint8_t* &buffer, const char* str, uint32_t len ) {
    putInt( buffer, len );
    memcpy((void*)(buffer), (const void*)str, len ); 
    buffer += len;
  }

  static void putString(uint8_t* &buffer, const std::string &str ) {
    putInt( buffer, str.length() );
    memcpy((void*)(buffer), (const void*)str.c_str(), str.length() ); 
    buffer += str.length();
  }

  static uint8_t getByte( uint8_t* &buffer) {
    uint8_t tmp = *buffer;
    buffer += sizeof(tmp);
    return tmp;
  }
  
  static uint32_t getInt( uint8_t* &buffer ) {
    uint32_t nval = 0;
    memcpy((void*)(&nval), (const void*)buffer, sizeof(nval));
    buffer += sizeof(nval) ;
    return ntohl(nval);
  }
  
  static uint64_t getInt64( uint8_t* &buffer ) {
    uint64_t nval;
    memcpy((void*)(&nval), (const void*)buffer, sizeof(nval));
    buffer += sizeof(nval) ;
    return Uint64Converter::toHostOrder(nval);
  }
  
  static int getString(uint8_t* &buffer, char* str, uint32_t maxlen ) {
    int len;
    len = getInt( buffer );
    if( len > maxlen ) throw std::runtime_error("buffer too short");
    memcpy((void*)(str), (const void*)buffer, len ); 
    buffer += len;
    return len;
  }

  static std::string getString(uint8_t* &buffer ) {
    int len;
    len = getInt( buffer );
    TmpBuf<char, 1024> buff(len+1);
    char* buffStr = buff.get(); 
    memcpy((void*)buffStr, (const void*)buffer, len);
    buffStr[len] = 0;
    buffer += len;
    return buffStr;
  }
};

}
}
#endif // BUFFER_SERIALIZATION_DECLARATIONS


