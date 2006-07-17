#ident "$Id$"
/* ************************************************************************** *
 * Various Address packing utilities.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_ADDRESS_UTL_HPP__
#define __SMSC_CONVERSION_ADDRESS_UTL_HPP__

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include <string>

#include "inman/common/types.hpp"

#define MAP_MAX_IMSI_AddressLength	8
#define MAP_MAX_IMSI_AddressValueLength	(MAP_MAX_IMSI_AddressLength*2)
#define MAP_MAX_ISDN_AddressLength	8

#define CAP_MAX_SMS_AddressStringLength	10 //CAP-datatypes.maxSMS-AddressStringLength - 1
#define CAP_MAX_SMS_AddressValueLength  (CAP_MAX_SMS_AddressStringLength*2)
#define CAP_MAX_LocationNumber_Length	8
#define CAP_MAX_TimeAndTimezoneLength	8
#define CAP_MAX_IMSILength		8

#define NUMBERING_ISDN                  1
#define ToN_INTERNATIONAL               1
#define ToN_ALPHANUM                    5
#define ToN_UNKNOWN                     0

namespace smsc {
namespace cvtutil {

//This is the simplified analog of sms::sms::Address, the difference is that
//its methods return error code instead of throwing exception
struct TonNpiAddress {
    unsigned char    length, typeOfNumber, numPlanInd;
                     //value is always zero-terminated
    unsigned char    value[CAP_MAX_SMS_AddressValueLength + 1];

    inline void clear(void) { length = typeOfNumber  = numPlanInd = value[0] = 0; }
    inline const char * getSignals(void) const { return (const char*)&value[0]; }

    TonNpiAddress() { clear(); }

    bool fromText(const char* text)
    {
        if (!text || !*text)
            return false;

        char    buff[CAP_MAX_SMS_AddressValueLength + 1];
        char *  addr_value = buff;
        int     iplan = 0, itype = 0;
        int     max_scan = 1, scanned = 0;

        memset(buff, 0, sizeof(buff));
        switch (text[0]) {
        case '.': {  //ton.npi.adr
            scanned = sscanf(text, ".%d.%d.%20s", &itype, &iplan, addr_value);
            max_scan = 3;
        } break;
        case '+': {  //isdn international adr
            scanned = sscanf(text, "+%20[0123456789]s", addr_value);
            iplan = itype = 1;
        } break;
        default:    //isdn unknown or alpha-numeric adr
            length = strlen(text);
            scanned = sscanf(text, "%20[0123456789]s", addr_value);
            if (scanned == 1 && (length == strlen(addr_value))) {
                iplan = 1; /*itype = 0;*/   // isdn unknown
            } else {
                if (length <= CAP_MAX_SMS_AddressValueLength) {
                    /*iplan = 0;*/ itype = 5;   //alpha-numeric adr
                    addr_value = (char*)text;
                    scanned = 1;
                }
            }
        }
        if (scanned < max_scan)
            return false;
        
        numPlanInd = (unsigned char)iplan;
        typeOfNumber = (unsigned char)itype;
        memcpy(value, addr_value, length = strlen((const char*)addr_value));
        value[length] = 0; //sscanf makes length <= sizeof(value)
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

    inline bool operator== (const TonNpiAddress &adr2) const
    {
        if ((typeOfNumber == adr2.typeOfNumber)
            && (numPlanInd == adr2.numPlanInd)
            && (length == adr2.length)) {
            return !strcmp((char*)value, (char*)adr2.value) ? true : false;
        }
        return false;
    }
};


struct MAPSCFinfo { //gsmSCF paramaters
    uint32_t      serviceKey;
    TonNpiAddress scfAddress;

    MAPSCFinfo() : serviceKey(0) { }
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
//according to Q.713 clause 3.4.2 (with GT & SSN)
extern unsigned packSCCPAddress(SCCP_ADDRESS_T* dst, const char *saddr, unsigned char ssn);
extern unsigned unpackSCCP2SSN_GT(const SCCP_ADDRESS_T* dst, unsigned char & ssn, char *addr);
}//namespace cvtutil
}//namespace smsc

using smsc::cvtutil::TonNpiAddress;
using smsc::cvtutil::TONNPI_OCT;
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::LOCATION_ADDRESS_OCTS;
using smsc::cvtutil::MAPSCFinfo;

#endif /* __SMSC_CONVERSION_ADDRESS_UTL_HPP__ */
