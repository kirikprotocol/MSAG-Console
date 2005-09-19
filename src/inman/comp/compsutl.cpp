static char const ident[] = "$Id$";

#include <vector>
#include <stdexcept>

#include "inman/codec/asn_internal.h"
#include "compsutl.hpp"

namespace smsc {
namespace inman {
namespace comp {
using std::vector;

extern "C" int print2vec(const void *buffer, size_t size, void *app_key)
{
    std::vector<unsigned char> *stream = (std::vector<unsigned char> *)app_key;
    unsigned char *buf = (unsigned char *)buffer;

    stream->insert(stream->end(),buf, buf + size);
    return 0;
}


unsigned packMAPAddress2OCTS(const Address& addr, TONNPI_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if ((addr.getTypeOfNumber() == 0x05) && !addr.getNumberingPlan())
	oa_length = smsc::cvtutil::packTextAs7BitPaddedSafe((const char*)addr.value,
			(unsigned)addr.length, oa->val, (unsigned)sizeof(oa->val));
    else
	oa_length = smsc::cvtutil::packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);

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
	addr.length = smsc::cvtutil::unpack7BitPadded2TextSafe(oa->val, valLen, (unsigned char*)addr.value,
							smsc::sms::MAX_ADDRESS_VALUE_LENGTH);
    else if (valLen*2 <= smsc::sms::MAX_ADDRESS_VALUE_LENGTH) //check for ABW
	addr.length = (uint8_t)smsc::cvtutil::unpackBCD2NumString(oa->val, (char*)addr.value, valLen);

    return (unsigned)addr.length;
}



unsigned packMAPAddress2LocationOCTS(const Address& addr, LOCATION_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if (addr.getTypeOfNumber() == 0x05)
	throw std::runtime_error(":INMAN: invalid typeOfNumber : 0x05");

    oa_length = smsc::cvtutil::packNumString2BCD(oa->val, (const char*)addr.value, (int)addr.length);
    oa->b0.st.oddAdrLen = (unsigned char)((int)addr.length % 2);
    oa->b0.st.ton = addr.getTypeOfNumber();

    /* just fill in predefined values for indicators */
    oa->b1.inds = 0;
    oa->b1.st.INNid    = 0x00;
    oa->b1.st.screenId = 0x01;

    return oa_length + 2;
}


Address	OCTET_STRING_2_Addres(OCTET_STRING_t * octs)
{
    Address	addr;
    if (octs && octs->size)
	unpackOCTS2MAPAddress(addr, (TONNPI_ADDRESS_OCTS *)(octs->buf), octs->size - 1);
    return addr;
}


}//namespace comp
}//namespace inman
}//namespace smsc

