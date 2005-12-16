#ifndef BUFFER_SERIALIZATION_DECLARATIONS
#define BUFFER_SERIALIZATION_DECLARATIONS

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif
#include "util/Uint64Converter.h"

namespace smsc { 
namespace util {
struct BufferSerializator
{
  static void putByte( uint8_t* &buffer, uint8_t val ) {
    *buffer = val;
    buffer+=sizeof(val);
  }
  
  static void putInt( uint8_t* &buffer, uint32_t val ) {
    uint32_t h_val = htonl(val);
    memcpy((void*)(buff), (const void*)&hval, sizeof(val));
    buffer += sizeof(val) ;
  }
  
  static void putInt64( uint8_t* &buffer, uint64_t val ) {
    uint32_t h_val = htonl(val);
    memcpy((void*)(buff), (const void*)&hval, sizeof(val));
    buffer += sizeof(val) ;
  }
  
  static void putString(uint8_t* &buffer, const char* str, uint32_t len ) {
    putInt( buffer, len );
    memcpy((void*)(buff), (const void*)str, len ); 
    buffer += len;
  }
};

}
}
#endif // BUFFER_SERIALIZATION_DECLARATIONS


