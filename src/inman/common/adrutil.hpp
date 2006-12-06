#ident "$Id$"
/* ************************************************************************** *
 * Various Address packing utilities.
 * ************************************************************************** */
#ifndef __SMSC_CONVERSION_ADDRESS_UTL_HPP__
#define __SMSC_CONVERSION_ADDRESS_UTL_HPP__

#include <inttypes.h>   //OS dependent
#include <string.h>
#include <stdio.h>

#include <string>

#include "inman/common/types.hpp"
#include "inman/AbntContract.hpp"

#define MAP_MAX_IMSI_AddressLength      8
#define MAP_MAX_IMSI_AddressValueLength (MAP_MAX_IMSI_AddressLength*2)
#define MAP_MAX_ISDN_AddressLength      8

#define CAP_MAX_LocationNumber_Length   8
#define CAP_MAX_TimeAndTimezoneLength   8
#define CAP_MAX_IMSILength              8


namespace smsc {
namespace cvtutil {

typedef smsc::inman::GsmSCFinfo MAPSCFinfo;

typedef union TONPI_OCT_u {
    unsigned char tonpi;
    struct {
        unsigned char reserved_1:1;
        unsigned char ton       :3;
        unsigned char npi       :4;
    } st;
} TONNPI_OCT;

typedef struct TONNPI_ADDRESS_OCTS_s {
    TONNPI_OCT      b0;
    unsigned char   val[CAP_MAX_SMS_AddressStringLength];  //MAX_ADDRESS_VALUE_LENGTH/2
} TONNPI_ADDRESS_OCTS;


typedef struct LOCATION_ADDRESS_OCTS_s {
    union _b0_u {
        unsigned char ton;  /* type and length of number */
        struct {
            unsigned char oddAdrLen : 1; /* odd/even number of chars in address */
            unsigned char ton       : 7; /* type of number */
        } st;
    } b0;
    union _b1_u {
        unsigned char inds; /* octet of various indicators */
        struct {
            unsigned char INNid     : 1;
            unsigned char npi       : 3;
            unsigned char presRestr : 2;
            unsigned char screenId  : 2;
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

using smsc::cvtutil::TONNPI_OCT;
using smsc::cvtutil::TONNPI_ADDRESS_OCTS;
using smsc::cvtutil::LOCATION_ADDRESS_OCTS;
using smsc::cvtutil::MAPSCFinfo;

#endif /* __SMSC_CONVERSION_ADDRESS_UTL_HPP__ */
