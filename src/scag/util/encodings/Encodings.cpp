#include "Encodings.h"

#include <memory>

namespace scag { namespace util { namespace encodings {

void Convertor::UCS2ToUTF8(unsigned short * ucs2buff, int ucs2len, std::string& utf8str)
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
            char ch = ((ucs2ch & 63) + 128);
            ptr[pos] = ch;
            pos++;

            ch = ((ucs2ch >> 6) + 192);
            ptr[pos] = ch;
            pos++;
        }
        else
        if ((ucs2ch >=0x00000800)&&(ucs2ch <= 0x0000FFFF))
        {
            char ch;
            for (int j=0; j<2; j++) 
            {
                ch = ((ucs2ch & 63) + 128);
                ptr[pos] = ch;
                pos++;

                ucs2ch >> 6;
            }

            ch = ucs2ch + 224;

            ptr[pos] = ch;
            pos++;
        }
    }

    utf8str.assign(ptr,pos);
  /*
    0x00000000 � 0x0000007F: 0xxxxxxx
    0x00000080 � 0x000007FF: 110xxxxx 10xxxxxx
    0x00000800 � 0x0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
    0x00010000 � 0x001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */
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
