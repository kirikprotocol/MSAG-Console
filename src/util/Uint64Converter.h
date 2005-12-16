#ifndef UINT64_CONVERTER_DECLARATIONS
#define UINT64_CONVERTER_DECLARATIONS

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

namespace smsc { namespace util
{
    struct Uint64Converter
    {
        static uint64_t toNetworkOrder(uint64_t value)
        {
            uint64_t result = 0;
            unsigned char *ptr=(unsigned char *)&result;
            ptr[0]=(unsigned char)(value>>56)&0xFF; ptr[1]=(unsigned char)(value>>48)&0xFF;
            ptr[2]=(unsigned char)(value>>40)&0xFF; ptr[3]=(unsigned char)(value>>32)&0xFF;
            ptr[4]=(unsigned char)(value>>24)&0xFF; ptr[5]=(unsigned char)(value>>16)&0xFF;
            ptr[6]=(unsigned char)(value>>8 )&0xFF; ptr[7]=(unsigned char)(value    )&0xFF;
            return result;
        };
        static uint64_t toHostOrder(uint64_t value)
        {
            unsigned char *ptr=(unsigned char *)&value;
            return (((((uint64_t)ptr[0])<<56)&0xFF00000000000000)|
                    ((((uint64_t)ptr[1])<<48)&0x00FF000000000000)|
                    ((((uint64_t)ptr[2])<<40)&0x0000FF0000000000)|
                    ((((uint64_t)ptr[3])<<32)&0x000000FF00000000)|
                    ((((uint64_t)ptr[4])<<24)&0x00000000FF000000)|
                    ((((uint64_t)ptr[5])<<16)&0x0000000000FF0000)|
                    ((((uint64_t)ptr[6])<< 8)&0x000000000000FF00)|
                    ((((uint64_t)ptr[7])    )&0x00000000000000FF));
        };
    };

}}

#endif // UINT64_CONVERTER_DECLARATIONS
