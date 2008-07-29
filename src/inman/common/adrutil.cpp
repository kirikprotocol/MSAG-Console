static char const ident[] = "$Id$";

#include "inman/common/adrutil.hpp"
#include "inman/common/cvtutil.hpp"

namespace smsc {
namespace cvtutil {

unsigned packMAPAddress2OCTS(const TonNpiAddress& addr, TONNPI_ADDRESS_OCTS * oa)
{
    unsigned	oa_length;

    if ((addr.typeOfNumber == 0x05) && !addr.numPlanInd)
	oa_length = packTextAs7BitPaddedSafe((const char*)addr.signals,
			(unsigned)addr.length, oa->val, (unsigned)sizeof(oa->val));
    else
	oa_length = packNumString2BCD(oa->val, (const char*)addr.signals, (int)addr.length);

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
    addr.signals[0] = addr.length = 0;

    if (addr.typeOfNumber == 0x05)	//packed 7bit text
	addr.length = unpack7BitPadded2TextSafe(oa->val, valLen, (unsigned char*)addr.signals,
							CAP_MAX_SMS_AddressValueLength);
    else if (valLen*2 <= CAP_MAX_SMS_AddressValueLength) //check for ABW
	addr.length = (uint8_t)unpackBCD2NumString(oa->val, (char*)addr.signals, valLen);

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

    oa_length = packNumString2BCD(oa->val, (const char*)addr.signals, (int)addr.length);
    oa->b0.st.oddAdrLen = (unsigned char)((int)addr.length % 2);
    oa->b0.st.ton = addr.typeOfNumber;

    /* just fill in predefined values for indicators */
    oa->b1.inds = 0;
    oa->b1.st.INNid    = 0x00;
    oa->b1.st.screenId = 0x01;

    return oa_length + 2;
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

