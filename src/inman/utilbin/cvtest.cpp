static char const ident[] = "$Id$";

#include <string.h>
#include <stdio.h>
#include "inman/common/cvtutil.hpp"

static char _tstr[] = "Your Subscriber Id is not valid";
//"*100*1#";
//"*100#";

/*
static unsigned char _res[] = {
    0xD6, 0xF0, 0x1C, 0x0D, 0xA2, 0xBF, 0xC7, 0x68, 0x77, 0x3A, 0x0F, 0x12,
    0x87, 0xD9, 0x61, 0xF7, 0x1C, 0x34, 0x7F, 0xCF, 0xE9, 0x61, 0x3B, 0x3B,
    0x1F, 0x2E, 0xD3, 0x41, 0x36, 0x17, 0xCC, 0x06, 0xAA, 0xCF, 0xC9, 0x2E
};*/
static unsigned char _res[] = {

0x53, 0x79, 0x73, 0x74, 0x65, 0x6D, 0x20, 0x65, 0x72, 0x72, 0x6F, 0x72,
0x2E, 0x20, 0x50, 0x6C, 0x65, 0x61, 0x73, 0x65, 0x20, 0x61, 0x73, 0x6B,
0x20, 0x66, 0x6F, 0x72, 0x20, 0x79, 0x6F, 0x75,
0x72, 0x20, 0x62, 0x61, 0x6C, 0x61, 0x6E, 0x63, 0x65, 0x20, 0x6C, 0x61,
0x74, 0x65, 0x72, 0x2E, 0x00

//    0xD3, 0xFC, 0x9C, 0x5E, 0x6E, 0x83, 0xCA, 0x72, 0xF9, 0x5B, 0xEE, 0x02
//    0xF5, 0xF9, 0x9C, 0xAC, 0xA3, 0xD1, 0x70
//  0xaa, 0x98, 0x6c, 0x36, 0x02
};

inline void printHex(const unsigned char * buf, unsigned len)
{
    for (unsigned i = 0; i < len; i++)
        printf(" %02x", (unsigned)buf[i]);
    return;    
}

inline void printAscii(const char * buf, unsigned len)
{
    for (unsigned i = 0; i < len; i++) {
      switch (buf[i]) {
      case '\n': printf("\\n"); break;
      case '\r': printf("\\r"); break;
      case '\t': printf("\\t"); break;
      default:
        printf("%c", buf[i]);
      }
    }
    return;    
}


int main (void)
{
    unsigned len;
    unsigned char _buffer[200];

    memset(_buffer, 0, sizeof(_buffer));

    printf("Initial ASCII Text(%u bytes): <", (unsigned)strlen(_tstr));
    printAscii(_tstr, (unsigned)strlen(_tstr));
    printf(">\n");

    len = smsc::cvtutil::packTextAs7BitPadded(_tstr, (unsigned)sizeof(_tstr)-1, _buffer);
    printf("Packed 7-Bit GSM text (%d bytes): ", len);
    printHex(_buffer, len);
    printf("\n");

    std::string  txt;
    len = smsc::cvtutil::unpack7BitPadded2Text(&_buffer[0], len, txt);
    printf("\nUnpacked ASCII Text: <");
    printAscii(txt.c_str(), (unsigned)txt.length());
    printf(">\n");

/*
    memset(_buffer, 0, sizeof(_buffer));
    printf("Packed 7-Bit GSM text (%d bytes): ", len = (unsigned)sizeof(_res));
    printHex(&_res[0], len);

    len = smsc::cvtutil::unpack7BitPadded2Text(&_res[0], (unsigned)sizeof(_res), &_buffer[0]);
    _buffer[len] = 0;
    printf("\nInitial ASCII Text: <%s>\n", _buffer);

    std::string  txt;
    len = smsc::cvtutil::unpack7BitPadded2Text(&_res[0], (unsigned)sizeof(_res), txt);
    printf("\nInitial ASCII Text: <%s>\n", txt.c_str());
*/
    return 0;
}