/*
$Id$
*/

#if !defined HEADER_RECODE_DLL_H
#define HEADER_RECODE_DLL_H

#ifdef _DLL

#ifdef RECODE_DLL_EXPORTS
#define RECODE_DLL_API __declspec(dllexport)
#else
#define RECODE_DLL_API __declspec(dllimport)
#endif

#define RECODE_DECL __cdecl

#else
#define RECODE_DLL_API
#define RECODE_DECL
#endif

extern "C" {

enum ConvEncodingEnum{
  CONV_ENCODING_ANSI = 0,
  CONV_ENCODING_CP1251 = 1,
  CONV_ENCODING_LATIN1 = 2,
  CONV_ENCODING_KOI8R = 3,
  CONV_ENCODING_ISO8895_5 = 4
};

RECODE_DLL_API int RECODE_DECL ConvertUCS2ToMultibyte(const short* ucs2, size_t ucs2buff_size,
                                          char* text, size_t textbuff_size,
                                          ConvEncodingEnum encoding);
RECODE_DLL_API int RECODE_DECL ConvertMultibyteToUCS2(const char* text, size_t textbuff_size,
                                          short* ucs2, size_t ucs2buff_size,
                                          ConvEncodingEnum encoding);

RECODE_DLL_API int RECODE_DECL Convert7BitToText(const char* bit7buf, size_t bit7buf_size,
                                     char* text, size_t textbuff_size);

RECODE_DLL_API int RECODE_DECL ConvertTextTo7Bit(const char* text, size_t textbuff_size,
                                     char* bit7buf, size_t bit7buf_size,
                                     ConvEncodingEnum encoding);
RECODE_DLL_API int RECODE_DECL Convert7BitToUCS2(const char* bit7buf, size_t bit7buf_size,
                                     short* ucs2, size_t ucs2buff_size);

RECODE_DLL_API int RECODE_DECL ConvertUCS2To7Bit(const short* ucs2, size_t ucs2buff_size,
                                     char* bit7buf, size_t bit7buf_size);

RECODE_DLL_API int RECODE_DECL Transliterate(const char* buf,size_t len,
                    ConvEncodingEnum encoding,char *dest,size_t destlen);

RECODE_DLL_API unsigned RECODE_DECL ConvertLatin1ToSMSC7Bit(const char* in, size_t chars,char* out);
RECODE_DLL_API unsigned RECODE_DECL ConvertSMSC7BitToLatin1(const char* in, size_t chars,char* out);


}


#endif // HEADER_RECODE_DLL_H
