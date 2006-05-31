#ifndef _SCAG_UTIL_DECODER_H_
#define _SCAG_UTIL_DECODER_H_

namespace scag { namespace util { namespace encodings

struct Convertor
{

    static void UCS2ToUTF8(unsigned char * usc2buff, int usc2len, std::string& utf8str);
    static void 7BitToUTF8(unsigned char * 7BitBuff, int 7BitBuffLen, std::string& utf8str);
    static void Latin1ToUTF8(unsigned char * latin1Buff, int latin1BuffLen, std::string& utf8str);
    

} 


}}

#endif
