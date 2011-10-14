#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

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
      addr.length = (uint8_t)unpack7BitPadded2TextSafe(use_octs, octs_len,
                                                       (unsigned char*)addr.signals, addr._maxSIGNALS); //throws
    else if (octs_len*2 <= addr._maxSIGNALS) { //check for ABW
      int res = unpackBCD2NumString(use_octs, (char*)addr.signals, octs_len, '?'); //TODO: throw here if < 0 ?
      addr.length = (uint8_t)smsc::util::absOf<int>(res);
    }
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

}//namespace cvtutil
}//namespace smsc

