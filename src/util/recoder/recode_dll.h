/*
$Id$
*/

#if !defined HEADER_RECODE_DLL_H
#define HEADER_RECODE_DLL_H

#ifdef _WIN32 

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

RECODE_DLL_API int RECODE_DECL ConvertUCS2ToMultibyte(const short* ucs2, int ucs2buff_size,
                                          char* text, int textbuff_size,
                                          ConvEncodingEnum encoding);
RECODE_DLL_API int RECODE_DECL ConvertMultibyteToUCS2(const char* text, int textbuff_size,
                                          short* ucs2, int ucs2buff_size,
                                          ConvEncodingEnum encoding);

RECODE_DLL_API int RECODE_DECL Convert7BitToText(const char* bit7buf, int bit7buf_size,
                                     char* text, int textbuff_size);

RECODE_DLL_API int RECODE_DECL ConvertTextTo7Bit(const char* text, int textbuff_size,
                                     char* bit7buf, int bit7buf_size,
                                     ConvEncodingEnum encoding);
RECODE_DLL_API int RECODE_DECL Convert7BitToUCS2(const char* bit7buf, int bit7buf_size,
                                     short* ucs2, int ucs2buff_size);

RECODE_DLL_API int RECODE_DECL ConvertUCS2To7Bit(const short* ucs2, int ucs2buff_size,
                                     char* bit7buf, int bit7buf_size);

}


#endif // HEADER_RECODE_DLL_H
