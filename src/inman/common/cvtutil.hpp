/* This module contains various conversion functions widely used in 3GPP */
#ifndef __SMSC_CONVERSION_UTIL_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CONVERSION_UTIL_HPP__

#include <time.h>
#include <string>
#include <stdexcept>
#include <inttypes.h>

namespace smsc {
namespace cvtutil {

/* macro checking for A(rray)B(ounds)R(ead)attemt while		*
 * extracting 7bit char from buffer containing packed 7bit text */
#define __notABR7(p, pEnd, ofs)  ((p + (ofs > 1 ? 1 : 0)) < pEnd)

/* ************************************************************************** *
 * Various text and alphabets conversion utility functions
 * ************************************************************************** */
extern unsigned char  const _cvt_7bit_2_8bit[128];
extern unsigned char  const _cvt_8bit_2_7bit[256];
/* GVR:
 * Packs unsigned integer [0.99] into octet in BCD form (network order).
 */
inline unsigned char packTinyNum2BCDOct(unsigned num)
{
    return ((unsigned char)(num % 10) << 4) | (unsigned char)(num / 10);
}

/* GVR:
 * Unpacks BCD octet (network order) to unsigned integer [0.99].
 */
inline unsigned char unpackBCDOct2Num(unsigned char &oct)
{
  return ((oct >> 4) + (oct & 0x0F)*10);
}

/* GVR:
 * Packs NumericString ('0'-'9') as Binary Coded Decimal String.
 * By default, in case of odd string length, the '1111'B filler is added
 * to high semioctet of last byte.
 *
 * NOTE: function doesn't perform check for output buffer ABW ('Array out
 * of Boundary Writing', its size is precisely determined, so
 * it's caller responsibility to provide necessary space.
 */
inline unsigned packNumString2BCD(unsigned char* bcd, const char* str,
                                  unsigned slen, bool filler = true)
{
    unsigned bcdLen = (slen + 1)/2;

    for (unsigned i = 0; i < slen; ++i) {
	if (i & 0x01) // i % 2
	    bcd[i/2] |= ((str[i]-'0') << 4);	// fill high semioctet
	else
	    bcd[i/2] = (str[i]-'0') & 0x0F;	// fill low semioctet
    }
    if ((slen % 2) && filler)
	bcd[bcdLen - 1] |= 0xF0;		// add filler to high semioctet

    return bcdLen;
}

/* GVR:
 * Unpacks BCD coded numeric string. Returns number of digits.
 */
inline unsigned unpackBCD2NumString(const unsigned char* bcd, char* str, unsigned bcdLen)
{
    unsigned i, k;
    for (i = k = 0; i < bcdLen; i++) {
        str[k++] = '0' + (char)(bcd[i] & 0x0F); // low semioctet
        // high semioctet, check for possible filler 
        if ((str[k] = (char)((bcd[i] >> 4) & 0x0F)) == 0x0F)
            break;
        str[k++] += '0';
    }
    str[k] = 0;
    return k;
}

/* GVR:
 * Unpacks TBCD (Telephony BCD) coded address to string. 
 * Returns signed number of digits (negative in case of illegal symbol was met).
 */
inline int unpackTBCD2String(const unsigned char* bcd, char* str, unsigned bcdLen)
{
    bool illegal = false;
    unsigned i, k;
    for (i = k = 0; i < bcdLen; i++) {
        str[k++] = '0' + (char)(bcd[i] & 0x0F); // low semioctet
        // high semioctet, check for possible filler 
        if ((str[k] = (char)((bcd[i] >> 4) & 0x0F)) > 0x09) {
            if (str[k] == 0x0F)
                break;
            if (str[k] == 0x0B)
                str[k++] = '*';
            else if (str[k] == 0x0C)
                str[k++] = '#';
            else {
                str[k++] = '?';
                illegal = true;
            }
        } else
            str[k++] += '0';
    }
    str[k++] = 0;
    return illegal ? -(int)k : (int)k;
}

/* GVR:
 * Adds symbol to buffer containing packed 7bit chars
 */
inline void packCharAs7Bit(unsigned char *& ptr, unsigned& shift,
						unsigned char val8bit)
{
    *ptr = (*ptr & (0xFF >> (8 - shift))) | (val8bit << shift);
    if (shift > 1)
	*(ptr + 1) = (*(ptr + 1) & (0xFF << shift)) | (val8bit >> (8 - shift));
    if ((shift += 7) >= 8) {
	shift &= 0x7;
	++ptr;
    }
}

/* GVR:
 * Adds symbol to buffer containing packed 7bit chars with ABW check.
 * In case of ABW attempt throws exception.
 */
inline void packCharAs7BitSafe(unsigned char *& ptr, unsigned& shift,
			 unsigned char val8bit, unsigned char* ptr_end)
                         throw(std::runtime_error)
{
    if (ptr >= ptr_end)
	throw std::runtime_error("packCharAs7Bit: ABW attempt detected!");

    packCharAs7Bit(ptr, shift, val8bit);
}

/* GVR:
 * Extracts 7Bit character 'as is' from buffer containing packed 7Bit text.
 */
inline unsigned char get7BitChar(unsigned char*& ptr, unsigned& shift)
{
    char val = (*ptr >> shift) & 0x7f;

    if (shift > 1)
	val |= (*(ptr + 1) << (8 - shift)) & 0x7f;
    if ((shift += 7) >= 8) {
	shift &= 0x7;
	++ptr;
    }
    return val;
}

/* GVR:
 * Determines number of packed 7bit chars in buffer keeping packed 7bit text, 
 * which are unpacked to single text symbol.
 * Returns 1 or 2, zero in case of end of array.
 */
inline unsigned estimate7BitChar(unsigned char*& b7ptr,
				const unsigned char*& b7End, unsigned& shift)
{
    unsigned num7ch = 0;

    if (__notABR7(b7ptr, b7End, shift)) {
	unsigned char ch = _cvt_7bit_2_8bit[get7BitChar(b7ptr, shift)];
	num7ch++;
	/* check for escaped character and don't forget about ABR check! */
	if ((ch == 0x1b) && __notABR7(b7ptr, b7End, shift))
	    num7ch++;
    }
    return num7ch;
}

extern unsigned char unpack7BitChar(unsigned char*& b7ptr, const unsigned char*& b7End,
					unsigned& shift, unsigned& num7ch);

extern unsigned estimateTextAs7Bit(const char* text, unsigned tlen, unsigned * _7bit_chars);

extern unsigned estimate7BitAsText(const unsigned char* b7buf, unsigned b7len,
							unsigned * _7bit_chars);
extern unsigned packTextAs7BitSafe(const char* text, unsigned tlen, unsigned char* octs,
						unsigned olen, unsigned * _7bit_chars);
extern unsigned packTextAs7Bit(const char* text, unsigned tlen, unsigned char* octs,
								unsigned * _7bit_chars);
extern unsigned packTextAs7BitPaddedSafe(const char* text, unsigned tlen,
				    unsigned char* octs, unsigned olen);
extern unsigned packTextAs7BitPadded(const char* text, unsigned tlen, unsigned char* octs);

extern unsigned unpack7Bit2Text(const unsigned char* b7buf, unsigned b7len,
					unsigned char* text, unsigned * _7bit_chars);
extern unsigned unpack7Bit2Text(const unsigned char* b7buf, unsigned b7len,
				std::string & str, unsigned * _7bit_chars);
extern unsigned unpack7Bit2TextSafe(const unsigned char* b7buf, unsigned b7len,
				    unsigned char* text, unsigned maxtlen,
				    unsigned * _7bit_chars) throw(std::runtime_error);
extern unsigned unpack7BitPadded2Text(const unsigned char* b7buf, unsigned b7len,
							unsigned char* text);
extern unsigned unpack7BitPadded2Text(const unsigned char* b7buf, unsigned b7len,
                                                        std::string & str);
extern unsigned unpack7BitPadded2TextSafe(const unsigned char* b7buf, unsigned b7len,
						unsigned char* text, unsigned maxtlen)
                                                throw(std::runtime_error);
/* ************************************************************************** *
 * Time conversion utility functions
 * ************************************************************************** */
#define _SMSC_CVT_BAD_TIME	(-1)
#define _SMSC_CVT_BAD_YEAR	(-2)
#define _SMSC_CVT_BAD_MONTH	(-3)
#define _SMSC_CVT_BAD_DAY	(-4)
#define _SMSC_CVT_BAD_HOUR	(-5)
#define _SMSC_CVT_BAD_MIN	(-6)
#define _SMSC_CVT_BAD_SEC	(-7)
#define _SMSC_CVT_BAD_QTZ	(-8)

extern int  unpack7BCD2TimeSTZ(unsigned char (*bcd)[7], struct tm &tms, int &qtz);
extern time_t cvtTimeSTZ2UTCTimeT(struct tm &tms, int &qtz);
extern time_t unpack7BCD2UTCTimeT(unsigned char (*bcd)[7]);
extern void packTimeSTZ2BCD7(unsigned char (*bcd)[7], struct tm &tms, int &qtz);
extern void packTimeSTZ2BCD8(unsigned char (*bcd)[8], struct tm &tms, int &qtz);
extern int  packTimeT2BCD7(unsigned char (*bcd)[7], time_t tmVal);
extern int  packTimeT2BCD8(unsigned char (*bcd)[8], time_t tmVal);

extern unsigned char packTP_VP_Relative(time_t vpVal);
extern time_t unpackTP_VP_Relative(unsigned char tpVp);

}//namespace cvtutil

/* ************************************************************************** *
 * CBS Data Coding related functions
 * ************************************************************************** */
namespace cbs {

typedef char   ISO_LANG[3]; //ISO 639, 2 letters

struct CBS_DCS {
   enum TextEncoding {dcGSM7Bit = 0, dcBINARY8 = 1, dcUCS2 = 2, dcReserved = 3};
   enum LangPrefix   { lngNone, lng4GSM7Bit, lng4UCS2 };

   enum TextEncoding    encoding;
   bool                 compressed;
   bool                 UDHind;
   enum LangPrefix      lngPrefix;
   bool                 msgClassDefined;
   unsigned char        msgClass : 2; //0 - 3
   ISO_LANG             language;  
};

CBS_DCS::TextEncoding  parseCBS_DCS(unsigned char dcs, CBS_DCS & res);
CBS_DCS::TextEncoding  parseCBS_DCS(uint8_t dcs);

} //cbs

}//namespace smsc
#endif /* __SMSC_CONVERSION_UTIL_HPP__ */

