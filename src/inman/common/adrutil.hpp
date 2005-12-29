#ident "$Id$"
/* ************************************************************************** *
 * Various Address packing utilities.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_ADDRESS_UTL_HPP__
#define __SMSC_CONVERSION_ADDRESS_UTL_HPP__

#include <string.h>
#include <stdio.h>

#include <string>

#define MAP_MAX_ISDN_AddressLength	8
#define CAP_MAX_SMS_AddressStringLength	10 //CAP-datatypes.maxSMS-AddressStringLength - 1
#define CAP_MAX_SMS_AddressValueLength  CAP_MAX_SMS_AddressStringLength*2
#define CAP_MAX_LocationNumber_Length	8
#define CAP_MAX_TimeAndTimezoneLength	8
#define CAP_MAX_IMSILength		8

namespace smsc {
namespace cvtutil {

//This is the simplified analog of sms::sms::Address, the difference is that
//its methods return error code instead of throwing exception
struct TonNpiAddress {
    unsigned char    length, typeOfNumber, numPlanInd;
                     //value is always zero-terminated
    unsigned char    value[CAP_MAX_SMS_AddressStringLength*2 + 1];

    inline void clear(void) { length = typeOfNumber  = numPlanInd = value[0] = 0; }

    TonNpiAddress() { clear(); }

    bool fromText(const char* text)
    {
        if (!text || !*text)
            return false;

        unsigned char    addr_value[20 + 1]; //CAP_MAX_SMS_AddressStringLength*2
        int iplan, itype;
        memset(addr_value, 0, sizeof(addr_value));

        int max_scan = 1, scanned;
        switch (text[0]) {
        case '.': {  //ton.npi.adr
            scanned = sscanf(text, ".%d.%d.%20s", &itype, &iplan, addr_value);
            max_scan = 3;
        } break;
        case '+': {  //isdn international adr
            scanned = sscanf(text, "+%20[0123456789?]s", addr_value);
            iplan = itype = 1;
        } break;
        default:    //isdn unknown adr
            scanned = sscanf(text, "%20[0123456789?]s", addr_value);
            iplan = 1; itype = 0;
        }
        if (scanned < max_scan)
            return false;
        
        numPlanInd = (unsigned char)iplan;
        typeOfNumber = (unsigned char)itype;
        memcpy(value, addr_value, length = strlen((const char*)addr_value));
        return true;
    }

    //use at least 30 chars buffer
    inline int toString(char* buf, int buflen) const
    {
        return snprintf(buf, buflen, ".%d.%d.%s", typeOfNumber, numPlanInd, value);
    }

    inline std::string toString() const
    {
        char buf[48]; // '.' + 3 + '.' + 3 + '.' + 20
        buf[0] = 0;   //helps in case of snprintf() failure
        snprintf(buf, sizeof(buf), ".%d.%d.%s", typeOfNumber, numPlanInd, value);
        return buf;
    }

    
};


typedef union TONPI_OCT_u {
    unsigned char tonpi;
    struct {
	unsigned char reserved_1:1;
	unsigned char ton:3;
	unsigned char npi:4;
    }	st;
} TONNPI_OCT;

typedef struct TONNPI_ADDRESS_OCTS_s {
    TONNPI_OCT	    b0;
    unsigned char   val[CAP_MAX_SMS_AddressStringLength];  //MAX_ADDRESS_VALUE_LENGTH/2
} TONNPI_ADDRESS_OCTS;


typedef struct LOCATION_ADDRESS_OCTS_s {
    union _b0_u {
	unsigned char ton;	/* type and length of number */
	struct {
	    unsigned char oddAdrLen	: 1; /* odd/even number of chars in address */
	    unsigned char ton		: 7; /* type of number */
	} st;
    } b0;
    union _b1_u {
	unsigned char inds;	/* octet of various indicators */
	struct {
	    unsigned char INNid		: 1;
	    unsigned char npi		: 3;
	    unsigned char presRestr	: 2;
	    unsigned char screenId	: 2;
	} st;
    } b1;
    unsigned char   val[CAP_MAX_LocationNumber_Length]; /* address */
} LOCATION_ADDRESS_OCTS;


//Packs TonNpiAddress to Address-String octets.
extern unsigned packMAPAddress2OCTS(const TonNpiAddress& addr, TONNPI_ADDRESS_OCTS * oa);

//Packs TonNpiAddress to LocationAddress octets.
//NOTE: alphanumeric numbers is forbidden!
//Returns 0 in case of error, number of packed octets otherwise
extern unsigned packMAPAddress2LocationOCTS(const TonNpiAddress& addr,
						LOCATION_ADDRESS_OCTS * oa);
extern unsigned unpackOCTS2MAPAddress(TonNpiAddress& addr, TONNPI_ADDRESS_OCTS * oa,
							    unsigned valLen);

}//namespace cvtutil
}//namespace smsc

using smsc::cvtutil::TonNpiAddress;
using smsc::cvtutil::TONNPI_OCT;
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::LOCATION_ADDRESS_OCTS;

#endif /* __SMSC_CONVERSION_ADDRESS_UTL_HPP__ */
