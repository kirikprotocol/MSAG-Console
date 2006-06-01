#include "Encodings.h"


namespace scag { namespace util { namespace encodings {

void Convertor::UCS2ToUTF8(unsigned char * usc2buff, int usc2len, std::string& utf8str)
{

}

void Convertor::GSM7BitToUTF8(unsigned char * gsm7BitBuff, int gsm7BitBuffLen, std::string& utf8str)
{
}

void Convertor::MultibyteToUTF8(unsigned char * latin1Buff, int latin1BuffLen, ConvEncodingEnum encoding, std::string& utf8str)
{
}

void Convertor::UTF8ToUSC2(unsigned char * utf8buff, int utf8len, std::string& ucs2str)
{
}

void Convertor::GSM7BitToUCS2(unsigned char * gsm7BitBuff, int gsm7BitBuffLen, std::string& ucs2str)
{
}

void Convertor::MultibyteToUCS2(unsigned char * latin1Buff, int latin1BuffLen, ConvEncodingEnum encoding, std::string& ucs2str)
{
}


}}}
