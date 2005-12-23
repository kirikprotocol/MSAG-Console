static char const ident[] = "$Id$";

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

unsigned packMAPAddress2OCTS(const TonNpiAddress& addr, TONNPI_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if ((addr.typeOfNumber == 0x05) && !addr.numPlanInd)
	oa_length = packTextAs7BitPaddedSafe((const char*)addr.value,
			(unsigned)addr.length, oa->val, (unsigned)sizeof(oa->val));
    else
	oa_length = packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);

    oa->b0.st.ton = addr.typeOfNumber;
    oa->b0.st.npi = addr.numPlanInd;
    oa->b0.st.reserved_1 = 1;

    return oa_length + 1;
}

/*
 * Returns number of signals in address, zero in case of error.
 */
unsigned unpackOCTS2MAPAddress(TonNpiAddress& addr, TONNPI_ADDRESS_OCTS * oa, unsigned valLen)
{
    addr.numPlanInd = oa->b0.st.npi;
    addr.typeOfNumber = oa->b0.st.ton;
    addr.value[0] = addr.length = 0;

    if (addr.typeOfNumber == 0x05)	//packed 7bit text
	addr.length = unpack7BitPadded2TextSafe(oa->val, valLen, (unsigned char*)addr.value,
							CAP_MAX_SMS_AddressValueLength);
    else if (valLen*2 <= CAP_MAX_SMS_AddressValueLength) //check for ABW
	addr.length = (uint8_t)unpackBCD2NumString(oa->val, (char*)addr.value, valLen);

    return (unsigned)addr.length;
}


//Packs TonNpiAddress to LocationAddress octets.
//NOTE: alphanumeric numbers is forbidden!
//Returns 0 in case of error, number of packed octets otherwise
unsigned packMAPAddress2LocationOCTS(const TonNpiAddress& addr, LOCATION_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if (addr.typeOfNumber == 0x05)
        return 0; //"invalid typeOfNumber : 0x05"

    oa_length = packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);
    oa->b0.st.oddAdrLen = (unsigned char)((int)addr.length % 2);
    oa->b0.st.ton = addr.typeOfNumber;

    /* just fill in predefined values for indicators */
    oa->b1.inds = 0;
    oa->b1.st.INNid    = 0x00;
    oa->b1.st.screenId = 0x01;

    return oa_length + 2;
}

}//namespace cvtutil
}//namespace smsc

