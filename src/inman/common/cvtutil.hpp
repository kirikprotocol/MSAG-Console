/* ************************************************************************** *
 * Various text/alphabets conversion utility functions widely used in 3GPP.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_UTIL_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_CONVERSION_UTIL_HPP__

#include <string>
#include <stdexcept>

#include "util/IntTypes.hpp"

namespace smsc {
namespace cvtutil {

extern char const k_TBCD_alphabet[15];

//Converts low semioctet of given byte to BCD character ("0-9").
//Returns 'ill_char' for non-alphabet character encoding.
inline char bcd2char(const uint8_t bcd_oct, const char ill_char = 0)
{
  uint8_t nible = (bcd_oct & 0x0F);
  return (nible <= 0x09) ? '0' + nible : ill_char;
}

//Converts low semioctet of given byte to TBCD character ("0-9*#abc").
//Returns 'ill_char' for non-alphabet character encoding.
inline char tbcd2char(const uint8_t bcd_oct, const char ill_char = 0)
{
  uint8_t nibble = (bcd_oct & 0x0F);
  return (nibble < 0x0F) ? k_TBCD_alphabet[nibble] : ill_char;
}

/* Packs NumericString ('0'-'9') as Binary Coded Decimal String.
 * By default, in case of odd string length, the '1111'B filler is added
 * to high semioctet of last byte.
 * Returns number of resulting octets in output BCD buffer.
 * 
 * NOTE: fucntion doesn't validate chars of input string!
 * NOTE: function doesn't perform check for output buffer ABW ('Array out
 * of Boundary Writing', its size is precisely determined, so it's caller
 * responsibility to provide necessary space!
 */
extern unsigned short packNumString2BCD(unsigned char * bcd_buf, const char * in_str,
                                        unsigned short str_len, const bool filler = true);

// Unpacks BCD coded numeric string up to 64K length.
// Returns signed number of digits unpacked:
// positive if succeeded, negative if illegal symbol was met.
// NOTE: all illegal symbols is replaced by 'ill_char'
extern int unpackBCD2NumString(const unsigned char * bcd_buf, char * out_str,
                               unsigned short bcd_len, const char ill_char = 0);
// Unpacks TBCD (Telephony BCD) coded string up to 64K length.
// Returns signed number of digits unpacked:
// positive if succeeded, negative if illegal symbol was met.
// NOTE: all illegal symbols is replaced by 'ill_char'
extern int unpackTBCD2String(const unsigned char * bcd_buf, char * out_str,
                             unsigned short bcd_len, const char ill_char = 0);

// Adds symbol to buffer containing packed 7bit chars
extern void packCharAs7Bit(unsigned char * & ptr, unsigned & shift,
                           unsigned char val8bit);
// Adds symbol to buffer containing packed 7bit chars with ABW check.
// In case of ABW attempt throws exception.
inline void packCharAs7BitSafe(unsigned char* & ptr, unsigned & shift,
                               unsigned char val8bit, unsigned char* ptr_end)
  throw(std::runtime_error)
{
  if (ptr >= ptr_end)
    throw std::runtime_error("packCharAs7Bit: ABW attempt detected!");
  packCharAs7Bit(ptr, shift, val8bit);
}

// Extracts 7Bit character 'as is' from buffer containing packed 7Bit text.
extern unsigned char get7BitChar(unsigned char* & ptr, unsigned & shift);

// Determines number of packed 7bit chars in buffer keeping packed 7bit text, 
// which are unpacked to single text symbol.
// Returns 1 or 2, zero in case of end of array.
extern unsigned char estimate7BitChar(unsigned char* & b7ptr,
                                      const unsigned char* & b7End, unsigned& shift);

extern unsigned char unpack7BitChar(unsigned char*& b7ptr, const unsigned char*& b7End,
                                    unsigned& shift, unsigned& num7ch);

// Determines the length (in bytes) of packed 7bit text converted from input text.
// If 'p_7bit_cnt' is non NULL, number of 7bit chars in output buffer is returned.
extern unsigned estimateTextAs7Bit(const char* text, unsigned tlen, unsigned * p_7bit_cnt);

// Determines the length of resulted Latin1 text string unpacked from given
// buffer, containing packed 7bit text. 
// If 'p_7bit_cnt' is non NULL, number of 7bit chars in input buffer is returned.
//
// NOTE: returned length doesn't take into account terminating zero.
extern unsigned estimate7BitAsText(const unsigned char* b7_buf, unsigned b7_len,
                                   unsigned * p_7bit_cnt);

/* Converts Latin1 text to GSM 7 bit Default Alphabet and packs it as specified in
 * 3GPP TS 23.038 [Character Packing]
 *
 * NOTE: because of input text may contain symbols from GSM 7 bit default alphabet
 * extension table, which are escaped, it's impossible to precisely determine length
 * of output buffer, so caller should estimate necessary buffer size and pass it to
 * function in order to perform ABW check.
 *
 * NOTE: these functions doesn't suitable for iterative packing.
 */
extern unsigned packTextAs7Bit(const char* text, unsigned tlen, unsigned char* octs,
                               unsigned * p_7bit_cnt);
extern unsigned packTextAs7BitSafe(const char* text, unsigned tlen, unsigned char* octs,
                                   unsigned olen, unsigned * p_7bit_cnt);
extern unsigned packTextAs7BitPaddedSafe(const char* text, unsigned tlen,
                                         unsigned char* octs, unsigned olen);
extern unsigned packTextAs7BitPadded(const char* text, unsigned tlen, unsigned char* octs);


/* Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to
 * zero-terminated Latin1 text string.
 * Returns number of characters in output string (zero doesn't accounted).
 */
extern unsigned unpack7Bit2Text(const unsigned char* b7_buf, unsigned b7_len,
                                unsigned char* text, unsigned * p_7bit_cnt);
extern unsigned unpack7Bit2Text(const unsigned char* b7_buf, unsigned b7_len,
                                std::string & str, unsigned * p_7bit_cnt);

extern unsigned unpack7Bit2TextSafe(const unsigned char* b7_buf, unsigned b7_len,
                                    unsigned char* out_str, const unsigned max_tlen,
                                    unsigned * p_7bit_cnt)
  throw(std::runtime_error);

/* Unpacks and converts packed 7Bit text from GSM 7 bit Default Alphabet to
 * zero-terminated Latin1 text string. Performs check for 7bit text padding.
 * Returns number of characters in output string (zero doesn't accounted).
 */
extern unsigned unpack7BitPadded2Text(const unsigned char* b7_buf, unsigned b7_len,
                                      unsigned char* out_str);
extern unsigned unpack7BitPadded2Text(const unsigned char* b7_buf, unsigned b7_len,
                                      std::string & out_str);
extern unsigned unpack7BitPadded2TextSafe(const unsigned char* b7_buf, unsigned b7_len,
                                          unsigned char* out_str, const unsigned max_tlen)
  throw(std::runtime_error);

}//cvtutil
}//smsc
#endif /* __SMSC_CONVERSION_UTIL_HPP__ */

