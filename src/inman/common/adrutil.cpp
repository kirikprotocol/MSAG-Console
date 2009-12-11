static char const ident[] = "$Id$";

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

//NOTE: provided buffer must be at least sizeof(TONNPI_ADDRESS_OCTS) bytes long
unsigned packMAPAddress2OCTS(const TonNpiAddress & addr, uint8_t * use_octs) //throws
{
  use_octs[0] = TONNPI_OCT::pack2Oct(addr.typeOfNumber, addr.numPlanInd); 

  unsigned	oa_length = 1;
  if ((addr.typeOfNumber == 0x05) && !addr.numPlanInd)
    oa_length += packTextAs7BitPaddedSafe(addr.getSignals(),
                (unsigned)addr.length, use_octs + 1, CAPConst::MAX_SMS_AddressStringLength);
  else
    oa_length += packNumString2BCD(use_octs + 1, addr.getSignals(), (int)addr.length);

  return oa_length;
}

/*
 * Returns number of signals in address, zero in case of error.
 */
unsigned unpackOCTS2MAPAddress(TonNpiAddress & addr, uint8_t * use_octs, unsigned octs_len) //throws
{
  addr.signals[0] = addr.length = 0;
  if (octs_len > 1) {
    TONNPI_OCT  tni(*use_octs);

    addr.numPlanInd = tni.npi;
    addr.typeOfNumber = tni.ton;
    ++use_octs; --octs_len;
  
    if (addr.typeOfNumber == 0x05)	//packed 7bit text
      addr.length = unpack7BitPadded2TextSafe(use_octs, octs_len,
                                      (unsigned char*)addr.signals, CAPConst::MAX_SMS_AddressValueLength);
    else if (octs_len*2 <= CAPConst::MAX_SMS_AddressValueLength) //check for ABW
      addr.length = (uint8_t)unpackBCD2NumString(use_octs, (char*)addr.signals, octs_len);
  }
  return (unsigned)addr.length;
}


//Packs TonNpiAddress to LocationAddress octets.
//Returns 0 in case of error, number of packed octets otherwise
//NOTE: alphanumeric numbers is forbidden! 
//NOTE: provided buffer must be at least sizeof(LOCATION_ADDRESS_OCTS) bytes long
unsigned packMAPAddress2LocationOCTS(const TonNpiAddress & addr, uint8_t * use_octs)
{
  if (addr.typeOfNumber == 0x05)
      return 0; //"invalid typeOfNumber : 0x05"

  LOCATION_ADDRESS_INDS ind;
  ind.b0.oddAdrLen = (uint8_t)(addr.length % 2);
  ind.b0.ton = addr.typeOfNumber;
  /* just fill in predefined values for indicators */
  ind.b1.INNid    = 0x00;
  ind.b1.screenId = 0x01;

  unsigned oa_length = ind.pack2Octs(use_octs);
  oa_length += packNumString2BCD(use_octs + oa_length, addr.getSignals(), (int)addr.length);
  return oa_length;
}

//according to Q.713 clause 3.4.2 (3.4.2.3.4) (with GT & SSN)
unsigned packSCCPAddress(SCCP_ADDRESS_T* dst, const char *saddr, unsigned char ssn)
{
    unsigned len = (unsigned)strlen(saddr);
    dst->addrLen = 5 + (len + 1)/2;             // length in octets
    dst->addr[0] = 0x12;                        // GlobTitle(0100) & SSN indicator
    dst->addr[1] = ssn;                         // SSN
    dst->addr[2] = 0;                           // Translation Type (not used)
    dst->addr[3] = 0x10 |                       // NumPlan(ISDN) & 
                    (!(len%2) ? 0x02 : 0x01);   //           encScheme(BCD odd/even)
    dst->addr[4] = 0x04;                        // Nature of address (international)

    //NOTE: SCCP address uses filler '0000'B !!!
    packNumString2BCD(dst->addr + 5, saddr, len, false);
    return len ? (unsigned)(dst->addrLen) : 0;
}

unsigned unpackSCCP2SSN_GT(const SCCP_ADDRESS_T* dst, unsigned char & ssn, char *addr)
{
    unsigned adrlen = unpackBCD2NumString(dst->addr + 5, addr, dst->addrLen - 5);
    //NOTE: SCCP address uses filler '0000'B !!!
    if ((dst->addr[3] & 0x3) % 2) { //odd length
        adrlen--;
        addr[adrlen-1] = 0;
    }
    ssn = dst->addr[1];
    return adrlen;
}


}//namespace cvtutil
}//namespace smsc

