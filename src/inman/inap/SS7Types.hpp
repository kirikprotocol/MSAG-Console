/* ************************************************************************* *
 * Common Types used by SS7 releated stuff.
 * ************************************************************************* */
#ifndef __SMSC_SS7_COMMON_TYPES__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_SS7_COMMON_TYPES__

#include <inttypes.h>

namespace smsc {
namespace inman {
namespace inap {

//According to Q.713 clause 3.4.2 generic SCCPAddress contains following
//elements:
// 1) - Address elements Indicators, 1 byte, mandatory
// 2) - Signalling Point Code, 2 bytes, optional
// 3) - Subsystem number, 1 byte, optional
// 4) - packed GlobalTitle, 2-N bytes, mandatory
//
//NOTE: GlobalTitle of most used kinds of mobile addresses has maximum length
//     of 13 bytes: 3/*sizeof(GTI_Inter)*/ + 10/*packed address indicator*/
struct SCCP_ADDRESS_OCTS {
  static const uint8_t k_maxOctetsNum = 24; // > 4 + 13

  uint8_t addrLen;
  uint8_t addr[k_maxOctetsNum];

  SCCP_ADDRESS_OCTS() : addrLen(0)
  { }
};

//Packs ISDN International address to SCCP address string octets 
//according to Q.713 clause 3.4.2 (with GT & SSN).
//Returns positive number of octets successfully packed, or
// -1 in case of invalid address string(illegal char or too long string).
extern short packSCCPAddress(SCCP_ADDRESS_OCTS & dst, const char * adr_str, uint8_t ssn);

//Unpacks ISDN International address from SCCP address string octets 
//according to Q.713 clause 3.4.2 (with GT & SSN)
extern short unpackSCCP2SSN_GT(const SCCP_ADDRESS_OCTS & dst, uint8_t & ssn, char *addr);

} //inap
} //inman
} //smsc
#endif /* __SMSC_SS7_COMMON_TYPES__ */

