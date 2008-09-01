#ifndef _SCAG_UTIL_ENCODINGS_H_
#define _SCAG_UTIL_ENCODINGS_H_

#include <string>

#include <core/buffers/TmpBuf.hpp>
#include <scag/exc/SCAGExceptions.h>

namespace scag { namespace util { namespace encodings {

using namespace scag::exceptions;

namespace EncodeTypes
{
    enum EncodingEnum
    {
  //Повторение костиковского енума
  CONV_ENCODING_ANSI = 0,
  CONV_ENCODING_CP1251 = 1,
  CONV_ENCODING_LATIN1 = 2,
  CONV_ENCODING_KOI8R = 3,
  CONV_ENCODING_ISO8895_5 = 4,

  //Дополнительные значения
  CONV_ENCODING_UCS2 = 5,
  CONV_ENCODING_UTF8 = 6,
  CONV_ENCODING_GMS7Bit = 7
    };
}

using namespace smsc::core::buffers;

struct Convertor
{
    /**
     *
     * @param usc2buff UCS2 string buffer (BIG ENDIAN!)
     * @param ucs2len the length of the string (in two-byte symbols)
     * @param utf8str resulting string
     *
     * NOTE: This method is provided for all network-related activities,
     * where endianness is strictly defined.
     * 
     * @see UTF8ToUCS2BE
    */
    static void UCS2BEToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str);

    /**
     *
     * @param utf8buff UTF8 buffer.
     * @param utf8len  buffer size in bytes.
     * @param ucs2str  resulting string in UCS2 (BIG ENDIAN!)
     *
     * NOTE: BOM is not placed into the resulting string.
     * 
     * @see UCS2BEToUTF8
    */
    static void UTF8ToUCS2BE(const char * utf8buff, unsigned int utf8len, std::string& ucs2str);


    /// NOTE: the following two methods are for "native" UCS2 conversion
    /// e.g. for parsing XML-document, etc.  The resulting UCS2 string
    /// should never leave the machine.
    /// The arguments are the same as for UCS2-BE methods.
    /// NOTE: BOM is not placed into the resulting string.
    static void UCS2ToUTF8( const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str);
    static void UTF8ToUCS2( const char* utf8buff, unsigned int utf8len, std::string& ucs2str );

    /**
     *
     * @param gsm7BitBuff Буффер, содержащий строку в формате GSM 7Bit
     * @param gsm7BitBuffLen Размер буффера в байтах
     * @param utf8str Строка, в которую возвращяется результат
     *
     * @see UTF8ToGSM7Bit
    */
    static void GSM7BitToUTF8(const char * gsm7BitBuff, unsigned int gsm7BitBuffLen, std::string& utf8str);


    /**
     *
     * @param utf8buff Буффер, содержащий строку в формате utf8
     * @param utf8len Размер буффера в байтах
     * @param gsm7BitBuff Буффер, в который будет возвращён результат в формате GSM 7Bit
     * @param gsm7BitBuffLen Размер буффера для результата
     *
     * @return Размер сегмента буффера gsm7BitBuff, занятого перекодированными в GSM7Bit байтами
     *
     * @see GSM7BitToUTF8
    */
    static void UTF8ToGSM7Bit(const char * utf8buff, unsigned int utf8len, std::string& gsm7BitStr);


    //TODO: Add DoxyGen comments !!!

    static void UTF8ToKOI8R(const char * utf8buff, unsigned int utf8len, std::string& koi8rStr);
    static void KOI8RToUTF8(const char * latin1Buff, unsigned int latin1BuffLen, std::string& utf8str);

    static void convert(const char* inCharset, const char* outCharset,
                        const char * in, size_t inLen, TmpBuf<uint8_t, 2048>& buf);

    static void convert(const char* inCharset, const char* outCharset,
                        const char * in, size_t inLen, std::string& outstr);

protected:

    Convertor() {};
    ~Convertor() {};
};


}}}

#endif
