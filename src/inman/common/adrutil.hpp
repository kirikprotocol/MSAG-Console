#ident "$Id$"
/* ************************************************************************** *
 * Various Address packing utilities.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_ADDRESS_UTL_HPP__
#define __SMSC_CONVERSION_ADDRESS_UTL_HPP__

#include "sms/sms.h"
using smsc::sms::Address;


#define MAP_MAX_ISDN_AddressLength	8
#define CAP_MAX_SMS_AddressStringLength	10 //CAP-datatypes.maxSMS-AddressStringLength - 1
#define CAP_MAX_LocationNumber_Length	8
#define CAP_MAX_TimeAndTimezoneLength	8
#define CAP_MAX_IMSILength		8


namespace smsc {
namespace cvtutil {

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


/* Packs Address to OCTET_STRING */
extern unsigned packMAPAddress2OCTS(const Address& addr, TONNPI_ADDRESS_OCTS * oa);
extern unsigned packMAPAddress2LocationOCTS(const Address& addr,
						LOCATION_ADDRESS_OCTS * oa);
extern unsigned unpackOCTS2MAPAddress(Address& addr, TONNPI_ADDRESS_OCTS * oa,
							    unsigned valLen);

}//namespace cvtutil
}//namespace smsc

using smsc::cvtutil::TONNPI_OCT;
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::LOCATION_ADDRESS_OCTS;

#endif /* __SMSC_CONVERSION_ADDRESS_UTL_HPP__ */
