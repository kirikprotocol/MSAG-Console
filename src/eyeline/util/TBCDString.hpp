/* ************************************************************************** *
 * Telephony and ordinary Binary Coded Decimal Strings.
 * ************************************************************************** */
#ifndef __EYELINE_UTIL_TBCD_STRING_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_UTIL_TBCD_STRING_HPP

#include <string>

namespace eyeline {
namespace util {

// Binary Coded Decimal String is used to represent several
// digits from 0 through 9, two digits per octet, each digit
// encoded 0000 to 1001 (0 to 9);
// by default, 1111 used as filler when there is an odd number
// of digits.

// bits 8765 of octet n keep encoding of digit 2n
// bits 4321 of octet n keep encoding of digit 2(n-1)+1
class BCDString : public std::string {
protected:
  //Converts low semioctet of given byte to BCD character.
  //Returns zero for non-alphabet character encoding.
  static char bcd2char(unsigned char bcd);

public:
  //check character for alphabet conformance
  //returns false in case of failure
  static bool validateChar(char use_char)
  {
    return ((use_char >= '0') && (use_char <= '9'));
  }
  //check characters for alphabet conformance
  //returns false if illegal character is found
  static bool validate(const char * str, unsigned slen);

  static unsigned countOctsLen(unsigned str_len)
  {
    return (str_len + 1)/2;
  }
  //NOTE: it's a caller responsibility to validate characters!
  //Returns number of bytes written into bcd buffer
  static unsigned pack2Octs(unsigned char* bcd_buf, const char * str,
                            unsigned slen, bool use_filler = true);
  //Unpacks and validates input bcd buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  static unsigned unpackOcts(const unsigned char * bcd_buf,
                             unsigned bcd_len, char * str);
  static unsigned unpackOcts(const unsigned char * bcd_buf,
                             unsigned bcd_len, std::string & str);

  //check characters for alphabet conformance
  //returns false if illegal character is found
  bool validate(void) const
  {
    return validate(c_str(), (unsigned)length());
  }
  //
  unsigned countOctsLen(void) const
  {
    return countOctsLen((unsigned)length());
  }
  //NOTE: it's a caller responsibility to validate characters!
  //Returns number of bytes written into bcd buffer
  unsigned pack2Octs(unsigned char* bcd_buf, bool use_filler = true) const
  {
    return pack2Octs(bcd_buf, c_str(), (unsigned)length(), use_filler);
  }
  //Unpacks and validates input bcd buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  unsigned unpackOcts(const unsigned char * bcd_buf, unsigned bcd_len)
  {
    return unpackOcts(bcd_buf, bcd_len, *this);
  }
};


// Telephony Binary Coded Decimal String is used to
// represent several digits from 0 through 9, *, #, a, b, c, two
// digits per octet, each digit encoded 0000 to 1001 (0 to 9),
// 1010 (*), 1011 (#), 1100 (a), 1101 (b) or 1110 (c);
// by default, 1111 used as filler when there is an odd number
// of digits.

// bits 8765 of octet n keep encoding of character 2n
// bits 4321 of octet n keep encoding of character 2(n-1)+1
class TBCDString : public std::string {
protected:
  //Converts low semioctet of given byte to TBCD character.
  //Returns zero for non-alphabet character encoding.
  static char tbcd2char(unsigned char bcd);

public:
  //check character for alphabet conformance
  //returns false in case of failure
  static bool validateChar(char use_char);
  //check characters for alphabet conformance
  //returns false if illegal character is found
  static bool validate(const char * str, unsigned slen);

  static unsigned countOctsLen(unsigned str_len)
  {
    return (str_len + 1)/2;
  }
  //NOTE: it's a caller responsibility to validate characters!
  static unsigned pack2Octs(unsigned char* bcd_buf, const char * str,
                            unsigned slen, bool filler = true);
  //Unpacks and validates input bcd buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  static unsigned unpackOcts(const unsigned char * bcd, unsigned bcd_len,
                             char * str);
  static unsigned unpackOcts(const unsigned char * bcd_buf,
                             unsigned bcd_len, std::string & str);


  unsigned countOctsLen(void) const
  {
    return countOctsLen((unsigned)length());
  }

  //check characters for alphabet conformance
  //returns false if illegal character is found
  bool validate(void) const
  {
    return validate(c_str(), (unsigned)length());
  }
  //NOTE: it's a caller responsibility to validate characters!
  //Returns number of bytes written into bcd buffer
  unsigned pack2Octs(unsigned char* bcd_buf, bool use_filler = true) const
  {
    return pack2Octs(bcd_buf, c_str(), (unsigned)length(), use_filler);
  }
  //Unpacks and validates input bcd buffer
  //Returns number of characters unpacked, 0 - in case of failure.
  unsigned unpackOcts(const unsigned char * bcd_buf, unsigned bcd_len)
  {
    return unpackOcts(bcd_buf, bcd_len, *this);
  }
};

} //util
} //eyeline

#endif /* __EYELINE_UTIL_Oct_STRING_HPP */

