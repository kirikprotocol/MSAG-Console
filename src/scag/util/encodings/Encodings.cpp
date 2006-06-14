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
    iconv_t cd = iconv_open("UCS-2", "UTF-8");

    if( cd == (iconv_t)(-1) ) throw SCAGException("Convertor: Cannot convert from 'utf8' to 'ucs2'");

    int nBuffSize = utf8len * 2;
    unsigned int nBytesLeft = nBuffSize;

    std::auto_ptr<char> buff(new char[nBuffSize]);
    char * buffPtr = buff.get();

    size_t res = iconv(cd, &utf8buff, &utf8len, &buffPtr, &nBytesLeft);
    iconv_close(cd);

    ucs2str.assign(buff.get(), nBuffSize - nBytesLeft);
}


void Convertor::GSM7BitToUTF8(const char * gsm7BitBuff, unsigned int gsm7BitBuffLen, std::string& utf8str)
{
    int nBuffSize = gsm7BitBuffLen;
    std::auto_ptr<char> buff(new char[nBuffSize]);

    ConvertSMSC7BitToLatin1(gsm7BitBuff, gsm7BitBuffLen, buff.get());
    utf8str.assign(buff.get(), gsm7BitBuffLen);
}

void Convertor::UTF8ToGSM7Bit(const char * utf8buff, unsigned int utf8len, std::string& gsm7BitStr)
{
    int nBuffSize = utf8len;
    std::auto_ptr<char> buff(new char[nBuffSize]);

    ConvertLatin1ToSMSC7Bit(utf8buff, utf8len, buff.get());

    gsm7BitStr.assign(buff.get(), utf8len);
}


void Convertor::KOI8RToUTF8(const char * latin1Buff, unsigned int latin1BuffLen, std::string& utf8str)
{
    iconv_t cd = iconv_open("UTF-8","KOI8-R");

    if( cd == (iconv_t)(-1) ) throw SCAGException("Convertor: Cannot convert from 'utf8' to 'ucs2'");

    int nBuffSize = latin1BuffLen * 2;
    unsigned int nBytesLeft = nBuffSize;

    std::auto_ptr<char> buff(new char[nBuffSize]);
    char * buffPtr = buff.get();

    size_t res = iconv(cd, &latin1Buff, &latin1BuffLen, &buffPtr, &nBytesLeft);
    iconv_close(cd);

    utf8str.assign(buff.get(), nBuffSize - nBytesLeft);
}


void Convertor::UTF8ToKOI8R(const char * utf8buff, unsigned int utf8len, std::string& koi8rStr)
{
    iconv_t cd = iconv_open("KOI8-R", "UTF-8");

    if( cd == (iconv_t)(-1) ) throw SCAGException("Convertor: Cannot convert from 'utf8' to 'koi8r'");

    unsigned int nBytesLeft = utf8len;
    int nBuffSize = utf8len;

    std::auto_ptr<char> buff(new char[nBuffSize]);
    char * buffPtr = buff.get();


    size_t res = iconv(cd, &utf8buff, &utf8len, &buffPtr, &nBytesLeft);
    iconv_close(cd);

    koi8rStr.assign(buff.get(), nBuffSize - nBytesLeft);

}


}}}
