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
  //���������� ������������� �����
  CONV_ENCODING_ANSI = 0,
  CONV_ENCODING_CP1251 = 1,
  CONV_ENCODING_LATIN1 = 2,
  CONV_ENCODING_KOI8R = 3,
  CONV_ENCODING_ISO8895_5 = 4,

  //�������������� ��������
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
     * @param usc2buff ������, ���������� ������ � ������� UCS2
     * @param ucs2len ������ ������ (������ ������ ������ �������� 2 �����)
     * @param utf8str ������, � ������� ������������ ���������
     *
     * @see UTF8ToUCS2
    */
    static void UCS2ToUTF8(const unsigned short * ucs2buff, unsigned int ucs2len, std::string& utf8str);

    /**
     *
     * @param utf8buff ������, ���������� ������ � ������� utf8
     * @param utf8len ������ ������� � ������
     * @param ucs2str ������, � ������� ������������ ���������
     *
     * @see UCS2ToUTF8
    */
    static void UTF8ToUCS2(const char * utf8buff, unsigned int utf8len, std::string& ucs2str);


    /**
     *
     * @param gsm7BitBuff ������, ���������� ������ � ������� GSM 7Bit
     * @param gsm7BitBuffLen ������ ������� � ������
     * @param utf8str ������, � ������� ������������ ���������
     *
     * @see UTF8ToGSM7Bit
    */
    static void GSM7BitToUTF8(const char * gsm7BitBuff, unsigned int gsm7BitBuffLen, std::string& utf8str);


    /**
     *
     * @param utf8buff ������, ���������� ������ � ������� utf8
     * @param utf8len ������ ������� � ������
     * @param gsm7BitBuff ������, � ������� ����� ��������� ��������� � ������� GSM 7Bit
     * @param gsm7BitBuffLen ������ ������� ��� ����������
     *
     * @return ������ �������� ������� gsm7BitBuff, �������� ����������������� � GSM7Bit �������
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
