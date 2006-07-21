#include "Encodings.h"

#include <memory>
#include <util/recoder/recode_dll.h>
#include <scag/exc/SCAGExceptions.h>
#include <scag/util/singleton/Singleton.h>

using namespace scag::exceptions;

namespace scag { namespace util { namespace encodings {

using namespace scag::util::singleton;

Hash<iconv_t> Convertor::iconvHash;

Mutex Convertor::mtx;

inline unsigned GetLongevity(Convertor*) { return 5; }
typedef SingletonHolder<Convertor> SingleC;

void Convertor::UCS2ToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str)
{
//    convert("UCS-2", "UTF-8", (const char*)ucs2buff, ucs2len, utf8str);
    
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
    convert("UTF-8", "UCS-2", utf8buff, utf8len, ucs2str);
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
    convert("KOI8-R", "UTF-8", latin1Buff, latin1BuffLen, utf8str);
}


void Convertor::UTF8ToKOI8R(const char * utf8buff, unsigned int utf8len, std::string& koi8rStr)
{
    convert("UTF-8", "KOI8-R", utf8buff, utf8len, koi8rStr);
}

iconv_t Convertor::getIconv(const char* inCharset, const char* outCharset)
{
    std::string s = inCharset;
    s += '#';
    s += outCharset;
    
    iconv_t *pcd = iconvHash.GetPtr(s.c_str());
    
    if(pcd) return *pcd;
        
    iconv_t cd = iconv_open(outCharset, inCharset);
    
    if( cd == (iconv_t)(-1) )
        throw SCAGException("Convertor: iconv_open. Cannot convert from '%s' to '%s'. errno=%d", inCharset, outCharset, errno);        
        
    iconvHash.Insert(s.c_str(), cd);
        
    return cd;        
}

void Convertor::convert(const char* inCharset, const char* outCharset,
                        const char * in, unsigned int inLen, TmpBuf<char, 2048>& buf)
{
    #define MAX_BYTES_IN_CHAR 2
    
    static Convertor& c = SingleC::Instance();
    
    MutexGuard mt(mtx);
    
    iconv_t cd = getIconv(inCharset, outCharset);
    
    char *outbufptr;
    size_t outbytesleft;
    
    iconv(cd, NULL, NULL, NULL, NULL);

    outbytesleft = MAX_BYTES_IN_CHAR * inLen;
    buf.setSize(buf.GetPos() + outbytesleft);
    outbufptr = buf.GetCurPtr();
    
    if(iconv(cd, &in, &inLen, &outbufptr, &outbytesleft) == (size_t)(-1) && errno != E2BIG)
        throw SCAGException("Convertor: iconv. Cannot convert from '%s' to '%s'. errno=%d. bytesleft=%d", inCharset, outCharset, errno, inLen);
        
    #undef MAX_BYTES_IN_CHAR        
}

void Convertor::convert(const char* inCharset, const char* outCharset,
                        const char * in, unsigned int inLen, std::string& outstr)
{
    TmpBuf<char, 2048> buf(2048);
    convert(inCharset, outCharset, in, inLen, buf);
    
    outstr.assign(buf.get(), buf.GetPos());
}

Convertor::~Convertor()
{
    char* k;
    iconv_t cd;
    
    MutexGuard mt(mtx);
    
    iconvHash.First();
    while(iconvHash.Next(k, cd))
    {
        if(cd != (iconv_t)-1)
            iconv_close(cd);
    }
}

}}}
