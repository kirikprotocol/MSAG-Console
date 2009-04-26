#ifndef MOD_IDENT_OFF
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include <string.h>

#include "eyeline/map/MobileAddress.hpp"

namespace eyeline {
namespace map {

//Parses ".ToN.NPi.TBCDSignals" representation (generic)
bool parseTonNpiTBCD(const char * adr_str, unsigned & itype,
                                unsigned & iplan, char * signals)
{
  if (adr_str[0] == '.') {
    char trash[10];
    //                            _maxAdrValueLength = 36
    int scanned = sscanf(adr_str, ".%u.%u.%44[0123456789*#abc]%10s", &itype, &iplan, signals, trash);
    return (scanned != 3) ? false : true;
  }
  return false;
}

//Parses "TBCDSignals" representation (national or unknowm telephony)
bool parseUnkownTele(const char * adr_str, unsigned & itype,
                                unsigned & iplan, char * signals)
{
  if (adr_str[0] == '.') {
    char trash[10];
    //                            _maxAdrValueLength = 36
    int scanned = sscanf(adr_str, "%44[0123456789*#abc]%10s", &itype, &iplan, signals, trash);
    if (scanned == 3) {
      itype = MobileAddress::tonUnknown;
      iplan = MobileAddress::npiISDNTele_e164;
      return true;
    }
  }
  return false;
}

//Parses "+TBCDSignals" representation (international telephony)
bool parseInterTele(const char * adr_str, unsigned & itype,
                                unsigned & iplan, char * signals)
{
  if (adr_str[0] == '+') {
    if (parseUnkownTele(adr_str + 1, itype, iplan, signals)) {
      itype = MobileAddress::tonInternational;
      return true;
    }
  }
  return false;
}

/* ************************************************************************* *
 * class MobileAddressAC implementation
 * ************************************************************************* */
bool MobileAddressAC::fromString(const char * adr_str)
{
  if (!adr_str || !*adr_str)
      return false;

  unsigned  iplan = 0, itype = 0;
  char      buff[MobileAddressAC::_maxStrValueLength + 1];

  for (ParsersList::const_iterator cit = _parsers.begin();
                                  cit != _parsers.end(); ++cit) {
    buff[0] = 0;
    if ((*cit)(adr_str, itype, iplan, buff)
        && (itype <= 0x7) && (iplan <= 0x0F)
        && (strlen(buff) <= maxAdrValueLen())) {
      setToN((uint8_t)itype);
      setNPi((uint8_t)iplan);
      setSignals(buff);
      return true;
    }
  }
  return false;  
}

unsigned MobileAddressAC::toString(char * buf, bool print_ton_npi/* = true*/) const
{
  unsigned n = 0;
  if (length()) {
    n = printPrefix(buf, print_ton_npi);
    strcpy(buf + n, getSignals()); //inserts ending zero
    n += length();
  } else
    buf[0] = 0;
  return n;
}  

} //map
} //eyeline

