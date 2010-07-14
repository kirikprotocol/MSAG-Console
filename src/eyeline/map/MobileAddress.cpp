#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "eyeline/map/MobileAddress.hpp"
#include "eyeline/util/TBCDString.hpp"

namespace eyeline {
namespace map {

using eyeline::util::TBCDString;

// ------------------------------------------------
// -- Mobile Address string representations parsers
// ------------------------------------------------
//NOTE: buffer provided to store address signals MUST be at least
//      _maxAddressValueStringLength + 1 bytes long!
// 

typedef bool (*fpAddressStringParser)(unsigned max_digits, const char * adr_str,
                            unsigned & itype, unsigned & iplan, char * signals_buf);

static bool parseTeleSignals(unsigned max_digits, const char * in_str, char * signals_buf)
{
  //compose format string for sscanf
  char fmt[sizeof("%[0123456789*#abc]%4s") + sizeof(unsigned)*3];
  snprintf(fmt, sizeof(fmt), "%%%u%s", (unsigned)max_digits, "[0123456789*#abc]%4s");
  //parse input string
  char trash[6];
  int scanned = sscanf(in_str, fmt, signals_buf, trash);
  return (scanned == 1);
}

//Parses "TBCDSignals" representation (national or unknowm telephony)
static bool parseUnkownTele(unsigned max_digits, const char * adr_str,
                            unsigned & itype, unsigned & iplan, char * signals_buf)
{
  if (parseTeleSignals(max_digits, adr_str, signals_buf)) {
    itype = MobileAddress::tonUnknown;
    iplan = MobileAddress::npiISDNTele_e164;
    return true;
  }
  return false;
}

//Parses ".ToN.NPi.TBCDSignals" representation (generic)
static bool parseTonNpiTele(unsigned max_digits, const char * adr_str,
                            unsigned & itype, unsigned & iplan, char * signals_buf)
{
  if (adr_str[0] == '.') {
    int offset = 0;
    int scanned = sscanf(adr_str, ".%1u.%2u.%n", &itype, &iplan, &offset);
    return (scanned != 2) ? false : 
            parseTeleSignals(max_digits, adr_str + offset, signals_buf);
  }
  return false;
}

//Parses "+TBCDSignals" representation (international telephony)
static bool parseInterTele(unsigned max_digits, const char * adr_str,
                           unsigned & itype, unsigned & iplan, char * signals_buf)
{
  if ((adr_str[0] == '+') 
      && parseTeleSignals(max_digits, adr_str + 1, signals_buf)) {
    itype = MobileAddress::tonInternational;
    iplan = MobileAddress::npiISDNTele_e164;
    return true;
  }
  return false;
}

/* ************************************************************************* *
 * class MAPAddressStringAC implementation
 * ************************************************************************* */
//Encodes/Packs address to octet buffer
//NOTE: buffer must be able to store maxOctets() bytes!!!
unsigned MAPAddressStringAC::pack2Octs(uint8_t * use_buf) const
{
  use_buf[0] = _ind.pack2Oct();
  unsigned n = TBCDString::pack2Octs(use_buf + 1, _signals, length());
  return n ? n + 1 : 0;
}
//Decodes/Unpacks address from octet buffer
unsigned MAPAddressStringAC::unpackOcts(const uint8_t * use_buf, unsigned buf_len)
{
  if (buf_len < 2)
    return 0;

  _ind.unpackOct(*use_buf);
  unsigned bcdLen = ((_maxOcts < buf_len) ? _maxOcts : buf_len) - 1;
  unsigned n = TBCDString::unpackOcts(use_buf + 1, bcdLen, _signals);
  return n ? n + 1 : 0;
}

//Converts address from one of possible textual representation
//NOTE: supports at least ".ToN.NPi.Signals" form
bool MAPAddressStringAC::parseString(const char * adr_str)
{
  static fpAddressStringParser _parser[3] = {
    parseInterTele, parseTonNpiTele, parseUnkownTele 
  };

  if (!adr_str || !*adr_str)
    return false;

  unsigned  iplan = 0, itype = 0;
  //, _signals
  for (unsigned i = 0; i < 3; ++i) {
    _signals[0] = 0;
    if (_parser[i](maxValueStrLength(), adr_str, itype, iplan, _signals)) {
      setToN((uint8_t)itype);
      setNPi((uint8_t)iplan);
      return true;
    }
  }
  clear();
  return false;
}

//Prints address TonNpi prefix, by default as ".ToN.NPi."
//In case of International ISDN numbering, prefix is printed as "+"
//as recommended in E.164 clause 12
unsigned MAPAddressStringAC::printPrefix(char * buf, bool print_ton_npi/* = true*/) const
{
  unsigned n = 0;
  if (isInterISDN() && !print_ton_npi)
    buf[n++] = '+';
  else {
    n = snprintf(buf, maxValueStrLength() - 1, ".%1u.%2u.",
                 (unsigned)_ind._ToN, (unsigned)_ind._NPi);
  }
  return n;
}

//Converts address to one of possible string representation,
//by default to ".ToN.NPi.Signals".
//Returns number of characters printed.
//NOTE: buffer must be at least maxValueStrLength() bytes long
unsigned MAPAddressStringAC::toString(char * buf, unsigned buf_sz, 
                                      bool print_ton_npi/* = true*/) const
{
  unsigned n = 0;
  if (!empty()) {
    n = printPrefix(buf, print_ton_npi);
    strcpy(buf + n, getSignals()); //inserts ending zero
    n += length();
  } else
    buf[0] = 0;
  return n;
}  

} //map
} //eyeline

