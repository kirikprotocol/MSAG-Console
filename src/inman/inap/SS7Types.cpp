#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <string.h>

#include "inman/inap/SS7Types.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace inman {
namespace inap {

//Packs ISDN International address to SCCP address string octets
//according to Q.713 clause 3.4.2 (with GT & SSN).
//Returns positive number of octets successfully packed, or
// -1 in case of invalid address string(illegal char or too long string).
short packSCCPAddress(SCCP_ADDRESS_OCTS & adr_oct, const char * adr_str, uint8_t ssn_val)
{
  unsigned len = (unsigned)strlen(adr_str);
  uint8_t oddNess = (len % 2) ? 0x01 : 0x02;
  len = (len + 1)/2 + 5;
  if (len >= SCCP_ADDRESS_OCTS::k_maxOctetsNum)
    return -1;

  adr_oct.addrLen = (uint8_t)len;   // length in octets
  adr_oct.addr[0] = 0x12;           // GlobTitle(0100) & SSN indicator
  adr_oct.addr[1] = ssn_val;        // SSN
  adr_oct.addr[2] = 0;              // Translation Type (not used)
  adr_oct.addr[3] = 0x10 | oddNess; // NumPlan(ISDN) & encScheme(BCD odd/even)
  adr_oct.addr[4] = 0x04;           // Nature of address (international)

  //NOTE: SCCP address uses filler '0000'B !!!
  smsc::cvtutil::packNumString2BCD(adr_oct.addr + 5, adr_str, len, false);
  return len ? (unsigned)(adr_oct.addrLen) : 0;
}

unsigned unpackSCCP2SSN_GT(const SCCP_ADDRESS_OCTS * dst, unsigned char & ssn, char *addr)
{
  int res = smsc::cvtutil::unpackBCD2NumString(dst->addr + 5, addr, dst->addrLen - 5, '?');
  unsigned adrlen = smsc::util::absOf<int>(res);
  //NOTE: SCCP address uses filler '0000'B !!!
  if ((dst->addr[3] & 0x3) % 2) { //odd length
    adrlen--;
    addr[adrlen-1] = 0;
  }
  ssn = dst->addr[1];
  return adrlen;
}

} //inap
} //inman
} //smsc

