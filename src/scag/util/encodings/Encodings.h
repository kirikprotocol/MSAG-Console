#ifndef _SCAG_UTIL_ENCODINGS_H_
#define _SCAG_UTIL_ENCODINGS_H_

#include <string>
#include <util/recoder/recode_dll.h>

namespace scag { namespace util { namespace encodings {

struct Convertor
{

    static void UCS2ToUTF8(unsigned short * ucs2buff, int ucs2len, std::string& utf8str);

    static void GSM7BitToUTF8(const char * gsm7BitBuff, int gsm7BitBuffLen, std::string& utf8str);
    static void MultibyteToUTF8(const char * latin1Buff, int latin1BuffLen, ConvEncodingEnum encoding, std::string& utf8str);

    static void UTF8ToUCS2(const char * utf8buff, int utf8len, std::string& ucs2str);
    static void GSM7BitToUCS2(const char * gsm7BitBuff, int gsm7BitBuffLen, std::string& ucs2str);
    static void MultibyteToUCS2(const char * latin1Buff, int latin1BuffLen, ConvEncodingEnum encoding, std::string& ucs2str);


}; 


}}}

#endif
