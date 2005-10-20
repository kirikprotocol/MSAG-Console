static char const ident[] = "$Id$";

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

unsigned packMAPAddress2OCTS(const Address& addr, TONNPI_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if ((addr.getTypeOfNumber() == 0x05) && !addr.getNumberingPlan())
	oa_length = packTextAs7BitPaddedSafe((const char*)addr.value,
			(unsigned)addr.length, oa->val, (unsigned)sizeof(oa->val));
    else
	oa_length = packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);

    oa->b0.st.ton = addr.getTypeOfNumber();
    oa->b0.st.npi = addr.getNumberingPlan();
    oa->b0.st.reserved_1 = 1;

    return oa_length + 1;
}

/*
 * Returns number of signals in address, zero in case of error.
 */
unsigned unpackOCTS2MAPAddress(Address& addr, TONNPI_ADDRESS_OCTS * oa, unsigned valLen)
{
    addr.plan = oa->b0.st.npi;
    addr.type = oa->b0.st.ton;
    addr.value[0] = addr.length = 0;

    if (addr.type == 0x05)	//packed 7bit text
	addr.length = unpack7BitPadded2TextSafe(oa->val, valLen, (unsigned char*)addr.value,
							smsc::sms::MAX_ADDRESS_VALUE_LENGTH);
    else if (valLen*2 <= smsc::sms::MAX_ADDRESS_VALUE_LENGTH) //check for ABW
	addr.length = (uint8_t)unpackBCD2NumString(oa->val, (char*)addr.value, valLen);

    return (unsigned)addr.length;
}



unsigned packMAPAddress2LocationOCTS(const Address& addr, LOCATION_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if (addr.getTypeOfNumber() == 0x05)
	throw std::runtime_error(":INMAN: invalid typeOfNumber : 0x05");

    oa_length = packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);
    oa->b0.st.oddAdrLen = (unsigned char)((int)addr.length % 2);
    oa->b0.st.ton = addr.getTypeOfNumber();

    /* just fill in predefined values for indicators */
    oa->b1.inds = 0;
    oa->b1.st.INNid    = 0x00;
    oa->b1.st.screenId = 0x01;

    return oa_length + 2;
}

}//namespace cvtutil
}//namespace smsc

