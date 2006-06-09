#include "Encodings.h"

#include <memory>
#include <util/recoder/recode_dll.h>
#include <iconv.h>
#include <scag/exc/SCAGExceptions.h>

using namespace scag::exceptions;

namespace scag { namespace util { namespace encodings {

void Convertor::UCS2ToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str)
{
    int pos = 0;
    
    std::auto_ptr<char> buff(new char[ucs2len*3]);
    char * ptr = buff.get();

    for (int i = 0; i < ucs2len; i++) 
    {
        unsigned short ucs2ch = ucs2buff[i];
     
        if ((ucs2ch >=0x00000000)&&(ucs2ch <= 0x0000007F)) 
        {
            ptr[pos] = ucs2ch;
            pos++;
        }
        else
        if ((ucs2ch >=0x00000080)&&(ucs2ch <= 0x000007FF))
        {
            char ch1 = ((ucs2ch & 63) + 128);
            char ch2 = ((ucs2ch >> 6) + 192);

            ptr[pos] = ch2;
            ptr[pos+1] = ch1;
            pos+=2;
        }
        else
        if ((ucs2ch >=0x00000800)&&(ucs2ch <= 0x0000FFFF))
        {
            char ch;
            for (int j=0; j<2; j++) 
            {
                ch = ((ucs2ch & 63) + 128);
                ptr[pos+2-j] = ch;

                ucs2ch >> 6;
            }

            ch = ucs2ch + 224;

            ptr[pos] = ch;
            pos+=3;
        }      
    }

    utf8str.assign(ptr,pos);
  /*
    0x00000000 — 0x0000007F: 0xxxxxxx
    0x00000080 — 0x000007FF: 110xxxxx 10xxxxxx
    0x00000800 — 0x0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
    0x00010000 — 0x001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
}

void Convertor::UTF8ToUCS2(const char * utf8buff, unsigned int utf8len, std::string& ucs2str)
{
    iconv_t cd = iconv_open("utf8", "ucs2");

    if( cd == (iconv_t)(-1) ) throw SCAGException("Convertor: Cannot convert from 'utf8' to 'ucs2'");

    int nBuffSize = utf8len*2;
    unsigned int nBytesLeft = nBuffSize;

    std::auto_ptr<char> buff(new char[nBuffSize]);
    char * buffPtr = buff.get();

    size_t res = iconv(cd, &utf8buff, &utf8len, &buffPtr, &nBytesLeft);
    iconv_close(cd);

    ucs2str.assign(buff.get());
}


void Convertor::GSM7BitToUTF8(const char * gsm7BitBuff, unsigned int gsm7BitBuffLen, std::string& utf8str)
{
}

int Convertor::UTF8ToGSM7Bit(const char * utf8buff, unsigned int utf8len, char * gsm7BitBuff, unsigned int gsm7BitBuffLen)
{
    return 0;
}


void Convertor::MultibyteToUTF8(const char * latin1Buff, unsigned int latin1BuffLen, EncodeTypes::EncodingEnum encoding, std::string& utf8str)
{
}


int Convertor::UTF8ToMultibyte(const char * utf8buff, unsigned int utf8len, EncodeTypes::EncodingEnum encoding, char * latin1Buff, unsigned int latin1BuffLen)
{
    return 0;
}


}}}
