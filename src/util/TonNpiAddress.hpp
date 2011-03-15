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

//Mobile addresses alphabets defined in sscanf() format.
struct MobileAlphabet {
  static const unsigned _MAX_ALPHABET_LEN = 60;
  
  const char * _Numeric;  //Decimal digits only
  const char * _Tele;     //ISDN telephony
  const char * _Gsm7;     //NOTE: actually it's a subset of GSM-7bit alphabet
                          //      (3GPP TS 23.038) with Greece chars excluded

  MobileAlphabet();

  //Validates and ouputs no more then 'max_chars' characters according to specified alphaBet.
  //NOTE: 'out_buf' MUST have enough capacity to store 'max_chars' + 1 symbols.
  static bool validateChars(const char * alpha_bet, const unsigned max_chars, const char * in_buf, char * out_buf);
};

extern const MobileAlphabet _MAPAlphabet;

/* ************************************************************************** *
 * Various AddressStrings
 * ************************************************************************** */

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
                                 
//Represents variety of mobile addresses, which are based or derived from 
//MAP-CommonDataTypes.AddressString ASN.1 type, i.e.:
//
//  MAPAddress, ISDNAddress, FTNAddress, ADNAddress, CAPSmsAddress
//
struct TonNpiAddress {
  enum TypeOfNumber_e { // 3 bits values
    tonUnknown = 0x0, tonInternational = 0x1, tonNationalSign = 0x2
    , tonNetworkSpec = 0x3, tonSubscriber = 0x4
     //NOTE: tonAlphanum is defined only for CAPSmsAddress, for other ones it's reserved!
    , tonAlphanum = 0x5
    , tonAbbreviated = 0x6, tonReservedExt = 0x7
  };
  enum NumberingPlan_e { // 4 bits values
    npiUnknown = 0x00, npiISDNTele_e164 = 0x01, npiSpare2 = 0x02,
    npiData_x121 = 0x03, npiTelex_f69 = 0x04, npiSpare5 = 0x05,
    npiLandMobile_e212 = 0x06, npiSpare7 = 0x07,
    npiNational = 0x08, npiPrivate = 0x09,
    npiSpare10 = 0x0A,  npiSpare11 = 0x0B,
    npiSpare12 = 0x0C, npiSpare13 = 0x0D, npiSpare14 = 0x0E,
    npiReservedExt = 0x0F
  };

  static const unsigned  _maxSIGNALS = CAPConst::MAX_SMS_AddressValueLength;
  static const unsigned  _strSZ = TON_NPI_AddressStringLength;

  uint8_t length, typeOfNumber, numPlanInd;
  char    signals[_maxSIGNALS + 1]; //signals is always zero-terminated


  TonNpiAddress() : length(0), typeOfNumber(0), numPlanInd(0)
  {
    signals[0] = 0;
  }
  ~TonNpiAddress()
  { }

  //Returns true on success
  bool fromText(const char * text_str);

  void clear(void) { length = typeOfNumber = numPlanInd = signals[0] = 0; }

  const char * getSignals(void) const { return (const char*)&signals[0]; }

  bool empty(void) const { return (bool)!length; }

  //checks if address is ISDN International or Unknown and sets it to International
  //returns false if address is not an ISDN International
  bool fixISDN(void)
  {
    if ((numPlanInd != npiISDNTele_e164) || (typeOfNumber > tonInternational))
      return false;
    typeOfNumber = tonInternational; //correct isdn unknown
    return true;
  }

  //Returns true if address numbering is ISDN international
  bool interISDN(void) const
  {
    return ((tonInternational == typeOfNumber) && (numPlanInd == npiISDNTele_e164));
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

