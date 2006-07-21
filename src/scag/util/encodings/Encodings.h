#ifndef _SCAG_UTIL_ENCODINGS_H_
#define _SCAG_UTIL_ENCODINGS_H_

#include <string>
#include <iconv.h>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/synchronization/Mutex.hpp>

namespace scag { namespace util { namespace encodings {

using namespace smsc::core::buffers;
using namespace smsc::core::synchronization;

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


struct Convertor
{
    Convertor::~Convertor();
    
    /**
     *
     * @param usc2buff Буффер, содержащий строку в формате UCS2
     * @param ucs2len Длинна строки (каждый символ строки занимает 2 байта)
     * @param utf8str Строка, в которую возвращяется результат
     * 
     * @see UTF8ToUCS2
    */
    static void UCS2ToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str);

    /**
     *
     * @param utf8buff Буффер, содержащий строку в формате utf8
     * @param utf8len Размер буффера в байтах
     * @param ucs2str Строка, в которую возвращяется результат 
     * 
     * @see UCS2ToUTF8
    */
    static void UTF8ToUCS2(const char * utf8buff, unsigned int utf8len, std::string& ucs2str);


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


    static void UTF8ToKOI8R(const char * utf8buff, unsigned int utf8len, std::string& koi8rStr);
    static void KOI8RToUTF8(const char * latin1Buff, unsigned int latin1BuffLen, std::string& utf8str);
    
    static void convert(const char* inCharset, const char* outCharset,
                        const char * in, unsigned int inLen, TmpBuf<char, 2048>& buf);
                        
    static void convert(const char* inCharset, const char* outCharset,
                        const char * in, unsigned int inLen, std::string& outstr);
    
protected:
    static iconv_t getIconv(const char* inCharset, const char* outCharset);
    
    static Hash<iconv_t> iconvHash;
    static Mutex mtx;
}; 


}}}

#endif
