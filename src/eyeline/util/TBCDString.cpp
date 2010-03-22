#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/util/TBCDString.hpp"

namespace eyeline {
namespace util {

/* ************************************************************************* *
 * class BCDString implementation
 * ************************************************************************* */
bool BCDString::validate(const char * str, unsigned slen)
{
  bool failed = slen ? false : true;
  for (unsigned i = 0; i < slen; ++i) {
    failed |= !validateChar(str[i]);
  }
  return !failed;
}
//
unsigned BCDString::pack2Octs(unsigned char* bcd_buf, const char * str,
                            unsigned slen, bool use_filler/* = true*/)
{
  unsigned bcdLen = countOctsLen(slen);

  for (unsigned i = 0; i < slen; ++i) {
      if (i & 0x01) // i % 2
          bcd_buf[i/2] |= ((str[i]-'0') << 4);	// fill high semioctet
      else
          bcd_buf[i/2] = (str[i]-'0') & 0x0F;	// fill low semioctet
  }
  if ((slen % 2) && use_filler)
      bcd_buf[bcdLen - 1] |= 0xF0;		// add filler to high semioctet

  return bcdLen;
}

//Converts low semioctet of given byte to TBCD character.
//Returns zero for non-alphabet character encoding.
char BCDString::bcd2char(unsigned char bcd)
{
  bcd &= 0x0F;
  return (bcd <= 0x09) ? '0' + bcd : 0;
}

#define BCD_UNPACK() \
  unsigned i = 0, k = 0; \
  for (; i < bcd_len; ++i) { \
    /* low semioctet */ \
    str[k] = bcd2char(bcd_buf[i]); \
    if (!str[k++]) /* illegal char encoding */ \
      return 0; \
    /* high semioctet */ \
    unsigned char sym = (bcd_buf[i] >> 4); \
    str[k] = bcd2char(bcd_buf[i]); \
    if (!str[k++]) { \
      /* either filler or illegal char encoding */ \
      return (sym == 0x0F) ? k : 0; \
    } \
  } \
  str[k] = 0; return k

//
unsigned BCDString::unpackOcts(const unsigned char * bcd_buf, 
                               unsigned bcd_len, char * str)
{
  BCD_UNPACK();
}
//
unsigned BCDString::unpackOcts(const unsigned char * bcd_buf,
                               unsigned bcd_len, std::string & str)
{
  str.clear();
  BCD_UNPACK();
}

/* ************************************************************************* *
 * class TBCDString implementation
 * ************************************************************************* */
static const char _TBCD_alphabet[] = {
//  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  '*', '#', 'a', 'b', 'c'
};

bool TBCDString::validateChar(char use_char)
{
  if (!BCDString::validateChar(use_char)) {
    for (unsigned i = 0; i < sizeof(_TBCD_alphabet); ++i) {
      if (use_char == _TBCD_alphabet[i])
        return true;
    }
  }
  return false;
}

bool TBCDString::validate(const char * str, unsigned slen)
{
  bool failed = slen ? false : true;
  for (unsigned i = 0; i < slen; ++i) {
    failed |= !validateChar(str[i]);
  }
  return !failed;
}

unsigned TBCDString::pack2Octs(unsigned char* bcd_buf, const char * str,
                            unsigned slen, bool use_filler/* = true*/)
{
  unsigned bcdLen = countOctsLen(slen);

  for (unsigned i = 0; i < slen; ++i) {
      if (i & 0x01) // i % 2
          bcd_buf[i/2] |= ((str[i]-'0') << 4);	// fill high semioctet
      else
          bcd_buf[i/2] = (str[i]-'0') & 0x0F;	// fill low semioctet
  }
  if ((slen % 2) && use_filler)
      bcd_buf[bcdLen - 1] |= 0xF0;		// add filler to high semioctet

  return bcdLen;
}

//Converts low semioctet of given byte to TBCD character.
//Returns zero for non-alphabet character encoding.
char TBCDString::tbcd2char(unsigned char bcd)
{
  bcd &= 0x0F;
  if (bcd <= 0x09)
    return '0' + bcd;
  if (bcd == 0x0A)
    return '*';
  else if (bcd == 0x0B)
    return '#';
  else if (bcd == 0x0C)
    return 'a';
  else if (bcd == 0x0D)
    return 'b';
  else if (bcd == 0x0E)
    return 'c';
  return 0;
}

#define TBCD_UNPACK() \
  unsigned i = 0, k = 0; \
  for (; i < bcd_len; ++i) { \
    /* low semioctet */ \
    str[k] = tbcd2char(bcd_buf[i]); \
    if (!str[k++]) /* illegal char encoding */ \
      return 0; \
    /* high semioctet */ \
    unsigned char sym = (bcd_buf[i] >> 4); \
    str[k] = tbcd2char(sym); \
    if (!str[k++]) { \
      /* either filler or illegal char encoding */ \
      return (sym == 0x0F) ? k : 0; \
    } \
  } \
  str[k] = 0; return k

//
unsigned TBCDString::unpackOcts(const unsigned char * bcd_buf,
                                unsigned bcd_len, char * str)
{
  TBCD_UNPACK();
}
//
unsigned TBCDString::unpackOcts(const unsigned char * bcd_buf,
                                unsigned bcd_len, std::string & str)
{
  str.clear();
  TBCD_UNPACK();
}


} //util
} //eyeline

