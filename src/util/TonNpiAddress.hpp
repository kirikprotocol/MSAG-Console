/* ************************************************************************** *
 * TonNpiAddress and GsmSCFinfo helper classes.
 * ************************************************************************** */
#ifndef __SMSC_UTIL_TONNPIADDR_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __SMSC_UTIL_TONNPIADDR_HPP__

#include <inttypes.h>

#include "core/buffers/FixedLengthString.hpp"

//TODO: proper define consts for MAP and CAP ToNs and NPis !!!
#define NUMBERING_ISDN                  1
#define ToN_INTERNATIONAL               1
#define ToN_ALPHANUM                    5
#define ToN_UNKNOWN                     0

namespace smsc {
namespace util {

//MAP constants defined in ASM.1 module MAP-CommonDataTypes 
struct MAPConst {
  //max number of octets representing IMSI ::= TBCD-STRING (SIZE (3..8))
  static const unsigned MAX_IMSI_AddressLength = 8;
  //max number of signals in IMSI
  static const unsigned MAX_IMSI_AddressValueLength = (MAX_IMSI_AddressLength*2);
  //max number of octets representing ISDN address (MAP-CommonDataTypes.maxISDN-AddressLength-1)
  static const unsigned MAX_ISDN_AddressLength = 8; 
  //max number of signals in ISDN address
  static const unsigned MAX_ISDN_AddressValueLength = (MAX_ISDN_AddressLength*2);
};

//CAP constants defined in ASM.1 module CAP-datatypes 
struct CAPConst {
  //max number of octets representing SMS address (CAP-datatypes.maxSMS-AddressStringLength - 1)
  static const unsigned MAX_SMS_AddressStringLength = 10;
  //max number of signals in SMS address
  static const unsigned MAX_SMS_AddressValueLength = (MAX_SMS_AddressStringLength*2);
  //max number of octets representing Location address
  static const unsigned MAX_LocationNumberLength = 8;
  //max number of octets representing time and timezone data
  static const unsigned MAX_TimeAndTimezoneLength = 8;
};

class IMSIString : public smsc::core::buffers::FixedLengthString<MAPConst::MAX_IMSI_AddressValueLength> {
public:
  static const char * _numberingFmt; // = "%15[0-9]%2s";

  static bool fromText(const char * in_text, char * out_str);

  IMSIString()
    : smsc::core::buffers::FixedLengthString<MAPConst::MAX_IMSI_AddressValueLength>()
  { }
  IMSIString(const char * use_val)
    : smsc::core::buffers::FixedLengthString<MAPConst::MAX_IMSI_AddressValueLength>(use_val)
  { }


  bool fromText(const char * in_text) { return fromText(in_text, str); }
};

//
typedef smsc::core::buffers::FixedLengthString<MAPConst::MAX_ISDN_AddressValueLength+1>
  MSCAddress;

//max number of chars in .ton.npi string representation of SMS address:
// '.' + 3 + '.' + 3 + '.' + 20
const unsigned TON_NPI_AddressStringLength = (9 + CAPConst::MAX_SMS_AddressValueLength + 1);

typedef smsc::core::buffers::FixedLengthString<TON_NPI_AddressStringLength>
  TonNpiAddressString;
                                 
//This is slightly more intelligent analog of sms::sms::Address,
//it also accepts alpha-numeric addresses.
//In opposite to sms::Address, all methods return error code instead
//of throwing exception.
struct TonNpiAddress {
    static const unsigned  _strSZ = TON_NPI_AddressStringLength;

    uint8_t length, typeOfNumber, numPlanInd;
            //signals is always zero-terminated
    char    signals[CAPConst::MAX_SMS_AddressValueLength + 1];

    void clear(void) { length = typeOfNumber = numPlanInd = signals[0] = 0; }
    const char * getSignals(void) const { return (const char*)&signals[0]; }
    bool empty(void) const { return (bool)!length; }

    TonNpiAddress() { clear(); }

    bool fromText(const char* text);

    //checks if address is ISDN International or Unknown and sets it to International
    //returns false if address is not an ISDN International
    bool fixISDN(void)
    {
        if ((numPlanInd != NUMBERING_ISDN) || (typeOfNumber > ToN_INTERNATIONAL))
            return false;
        typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown
        return true;
    }

    //Returns true if address numbering is ISDN international
    bool interISDN(void) const
    {
        return ((ToN_INTERNATIONAL == typeOfNumber) && (numPlanInd == NUMBERING_ISDN));
    }
                
    //use at least TonNpiAddress::_strSZ chars buffer
    int toString(char* buf, bool ton_npi = true, unsigned buflen = TonNpiAddress::_strSZ) const;

    TonNpiAddressString toString(bool ton_npi = true) const
    {
        TonNpiAddressString buf;
        toString(buf.str, ton_npi);
        return buf;
    }

    bool operator== (const TonNpiAddress &adr2) const
    {
        if ((typeOfNumber == adr2.typeOfNumber)
            && (numPlanInd == adr2.numPlanInd)
            && (length == adr2.length)) {
            return !strcmp(signals, adr2.signals) ? true : false;
        }
        return false;
    }
    bool operator!= (const TonNpiAddress &adr2) const
    {
      return !(*this == adr2);
    }

    bool operator < (const TonNpiAddress & cmp_obj) const
    {
      if (typeOfNumber == cmp_obj.typeOfNumber) {
        if (numPlanInd == cmp_obj.numPlanInd)
          return (strcmp(signals, cmp_obj.signals) < 0);
        return (numPlanInd < cmp_obj.numPlanInd);
      }
      return (typeOfNumber < cmp_obj.typeOfNumber);
    }
};


}//util
}//smsc
#endif /* __SMSC_UTIL_TONNPIADDR_HPP__ */

