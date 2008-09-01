#include <memory>

#include <iconv.h>
#include <util/recoder/recode_dll.h>

#include <core/buffers/Hash.hpp>
#include <core/synchronization/Mutex.hpp>

#include <scag/util/singleton/Singleton.h>

#include <logger/Logger.h>

#include "Encodings.h"
#include "scag/util/strlcpy.h"

namespace {
#if BYTE_ORDER == LITTLE_ENDIAN
// e.g. linux
const char* nativeucs2 = "UCS2-LE";
#else
const char* nativeucs2 = "UCS2-BE";
#endif
}

namespace {

void dump( const char* what, const char* s, size_t sz )
{
    fprintf( stderr, "%s has %u bytes:", what, sz );
    unsigned cnt = 0;
    const char* p = s;
    for ( ; sz-- > 0 ; ++p ) {
        fprintf( stderr, " %#2.2x", unsigned(*p) );
        if ( ++cnt >= 8 ) {
            fprintf( stderr, "\n" );
            cnt = 0;
        }
    }
    if ( cnt ) fprintf( stderr, "\n" );
}

}


namespace scag { namespace util { namespace encodings {

using namespace smsc::core::synchronization;

struct ConvertorImpl : public Convertor
{

    ConvertorImpl() : Convertor() {};
    ~ConvertorImpl();

    void convert(const char* inCharset, const char* outCharset,
                 const char * in, size_t inLen, TmpBuf<uint8_t, 2048>& buf);

private:

    iconv_t getIconv(const char* inCharset, const char* outCharset);

    Hash<iconv_t> iconvHash;
    Mutex mtx;
};

using namespace scag::util::singleton;

inline unsigned GetLongevity(Convertor*) { return 5; }
typedef SingletonHolder<ConvertorImpl> SingleC;


void Convertor::UCS2ToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str)
{
    convert( nativeucs2, "UTF-8", (const char*)ucs2buff, ucs2len*2, utf8str );
}

void Convertor::UCS2BEToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str)
{
    convert( "UCS2-BE", "UTF-8", (const char*)ucs2buff, ucs2len*2, utf8str );
}


    /*
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

                ucs2ch >>= 6;
            }

            ch = ucs2ch + 224;

            ptr[pos] = ch;
            pos+=3;
        }
    }

    utf8str.assign(ptr,pos);
     */
  /*
    0x00000000 — 0x0000007F: 0xxxxxxx
    0x00000080 — 0x000007FF: 110xxxxx 10xxxxxx
    0x00000800 — 0x0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx
    0x00010000 — 0x001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    */

void Convertor::UTF8ToUCS2BE(const char * utf8buff, unsigned int utf8len, std::string& ucs2str)
{
    convert("UTF-8", "UCS-2BE", utf8buff, utf8len, ucs2str);
}

void Convertor::UTF8ToUCS2(const char * utf8buff, unsigned int utf8len, std::string& ucs2str)
{
    convert("UTF-8", nativeucs2, utf8buff, utf8len, ucs2str);
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


#define MAX_BYTES_IN_CHAR 4

void Convertor::convert(const char* inCharset, const char* outCharset,
                        const char * in, size_t inLen, TmpBuf<uint8_t, 2048>& buf)
{
    ConvertorImpl& c = SingleC::Instance();
    c.convert(inCharset, outCharset, in, inLen, buf);
}

void Convertor::convert(const char* inCharset, const char* outCharset,
                        const char * in, size_t inLen, std::string& outstr)
{
    TmpBuf<uint8_t, 2048> buf(inLen * MAX_BYTES_IN_CHAR);
    convert(inCharset, outCharset, in, inLen, buf);
//    smsc_log_debug(smsc::logger::Logger::getInstance("conv.conv"), "buf=%d getpos=%d", buf.get(), buf.GetPos());
    outstr.assign((const char*)buf.get(), buf.GetPos());
}

iconv_t ConvertorImpl::getIconv(const char* inCharset, const char* outCharset)
{
    uint32_t i;
    char s[128];

    i = strlcpy(s, inCharset, 128);
    s[i] = '#';
    strlcpy(s + i + 1, outCharset, 128 - i - 1);

    iconv_t *pcd = iconvHash.GetPtr(s);
    if(pcd) return *pcd;

    iconv_t cd = iconv_open(outCharset, inCharset);
    if( cd == (iconv_t)(-1) )
        throw SCAGException("Convertor: iconv_open. Cannot convert from '%s' to '%s'. errno=%d", inCharset, outCharset, errno);

    iconvHash.Insert(s, cd);

    return cd;
}

#define ICONV_BLOCK_SIZE 128

void ConvertorImpl::convert(const char* inCharset, const char* outCharset,
                            const char * in, size_t inLen, TmpBuf<uint8_t, 2048>& buf)
{

    size_t inlenleft = inLen;
    bool error=false;
    {
      MutexGuard mt(mtx);

      iconv_t cd = getIconv(inCharset, outCharset);

      char *outbufptr;
      size_t outbytesleft, i;

      iconv(cd, NULL, NULL, NULL, NULL);

      i = outbytesleft = MAX_BYTES_IN_CHAR * inLen;
      buf.setSize(buf.GetPos() + outbytesleft);
      outbufptr = (char*)buf.GetCurPtr();
      //unsigned long inLenLong=inLen;
//    smsc_log_debug(smsc::logger::Logger::getInstance("conv.conv"), "in=%s, out=%s, inbuf=%d, inbuflen=%d, outbutesleft=%d", inCharset, outCharset, in, inLen, outbytesleft);
      char* inptr = const_cast<char*>(in);

      if(iconv(cd,
// #ifdef __GNUC__
//                const_cast<char**>(&in), // the problem in iconv definition
// #else
//                &in,
// #endif
               &inptr,
               &inlenleft, &outbufptr, &outbytesleft) == (size_t)(-1) && errno != E2BIG)
          error=true;

      buf.SetPos(i - outbytesleft);
  }
  if(error)
  {
      MutexGuard mg(mtx);
      dump( "input", in, inLen );
      throw SCAGException("Convertor: iconv. Cannot convert from '%s' to '%s'. errno=%d. bytesleft=%d", inCharset, outCharset, errno, inlenleft);
  }

//    smsc_log_debug(smsc::logger::Logger::getInstance("conv.conv"), "in=%s, out=%s, inbuf=%d, inbuflen=%d, outbutesleft=%d", inCharset, outCharset, in, inLen, outbytesleft);

/*    while (inLen) {
         size_t i = inLen > ICONV_BLOCK_SIZE ? ICONV_BLOCK_SIZE : inLen;
         buf.setSize(buf.GetPos() + MAX_BYTES_IN_CHAR * ICONV_BLOCK_SIZE);
         outbufptr = buf.GetCurPtr();
         outbytesleft = MAX_BYTES_IN_CHAR * ICONV_BLOCK_SIZE;

         printf("<iconv_t=%d in=%p inLen=%d outbufptr=%p outbytesleft=%d incharset=%s outCharset=%s\n", cd, in, i, outbufptr, outbytesleft, inCharset, outCharset);
         if(iconv(cd, &in, &i, &outbufptr, &outbytesleft) == (size_t)(-1) && errno != E2BIG)
         {
             printf("iconv_err\n");
             throw SCAGException("Convertor: iconv. Cannot convert from '%s' to '%s'. errno=%d. bytesleft=%d", inCharset, outCharset, errno, inLen);
         }
         printf(">iconv_t=%d in=%p inLen=%d outbufptr=%p outbytesleft=%d incharset=%s outCharset=%s\n", cd, in, i, outbufptr, outbytesleft, inCharset, outCharset);
         buf.SetPos(buf.GetPos() + MAX_BYTES_IN_CHAR * ICONV_BLOCK_SIZE - outbytesleft);

         if(inLen > ICONV_BLOCK_SIZE)
             inLen -= ICONV_BLOCK_SIZE;
         else
             inLen = 0;
     }*/
}

#undef MAX_BYTES_IN_CHAR
#undef ICONV_BLOCK_SIZE

ConvertorImpl::~ConvertorImpl()
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
