#ident "$Id$"
#ifndef __SMSC_INMAN_INAP_COMPS_UTL_HPP__
#define __SMSC_INMAN_INAP_COMPS_UTL_HPP__

#define INMAN_LOG_ON


#include "sms/sms.h"
#include "inman/common/cvtutil.hpp"
#include "inman/codec/constr_TYPE.h"

using smsc::sms::Address;
using smsc::logger::Logger;

#include "OCTET_STRING.h"
extern "C" int print2vec(const void *buffer, size_t size, void *app_key);


#ifndef INMAN_LOG_ON
#define INMAN_LOG_ENC(er, asnDef)
#define INMAN_LOG_DEC(drc, asnDef)
#else  /* INMAN_LOG_ON */
#include "logger/Logger.h"
#define INMAN_LOG_ENC(er, asnDef)	if (er.encoded == -1) { \
	smsc_log_error(compLogger, \
	"Encoding of %s failed at %s", asnDef.name, er.failed_type->name); \
	throw EncodeError(format(":InMan: Encoding of %s failed at %s", asnDef.name, er.failed_type->name)); }

#define INMAN_LOG_DEC(drc, asnDef)	if (drc.code != RC_OK) { \
	smsc_log_error(compLogger, \
	"Decoding of %s failed with code %s at byte: %d\n", asnDef.name, \
	drc.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE", drc.consumed); \
	throw DecodeError(format(":InMan: Decoding of %s failed with code %s at byte: %d\n", \
	    asnDef.name, drc.code == RC_FAIL ? "RC_FAIL" : "RC_WMORE", drc.consumed)); }
#endif /* INMAN_LOG_ON */


#define CAP_MAX_SMS_AddressStringLength	10 //CAP-datatypes.maxSMS-AddressStringLength - 1
#define CAP_MAX_LocationNumber_Length	8
#define CAP_MAX_TimeAndTimezoneLength	8
#define CAP_MAX_IMSILength		8


namespace smsc {
namespace inman {
namespace comp {

extern std::string printType2String(asn_TYPE_descriptor_t * def, void * tStruct);

inline void smsc_log_component(Logger *lgi, asn_TYPE_descriptor_t * asnDef, void * tStruct)
{
    if (lgi->isLogLevelEnabled(smsc::logger::Logger::LEVEL_DEBUG)) {
	std::string sdump = printType2String(asnDef, tStruct);
	lgi->log_(smsc::logger::Logger::LEVEL_DEBUG, sdump);
    }
}



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

extern Address	OCTET_STRING_2_Addres(OCTET_STRING_t * octs);


}//namespace comp
}//namespace inman
}//namespace smsc

#endif /* __SMSC_INMAN_INAP_COMPS_UTL_HPP__ */
