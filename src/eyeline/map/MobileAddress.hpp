/* ************************************************************************** *
 * Base classes for various mobile addresses.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_MOBILE_ADDRESS_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_MOBILE_ADDRESS_HPP

#include <inttypes.h>
#include <list>

#include "eyeline/util/TBCDString.hpp"

namespace eyeline {
namespace map {

using eyeline::util::TBCDString;

// ------------------------------------------------
// -- Mobile Address string representations parsers
// ------------------------------------------------
//NOTE: buffer provided to store address signals MUST be at least
//      MobileAddressAC::_maxStrValueLength bytes long!
// 
//Parses ".ToN.NPi.TBCDSignals" address representation (generic)
static bool parseTonNpiTBCD(const char * adr_str, unsigned & itype,
                              unsigned & iplan, char * signals);

//Parses "TBCDSignals" address representation (national or unknowm telephony)
static bool parseUnknownTele(const char * adr_str, unsigned & itype,
                              unsigned & iplan, char * signals);

//Parses "+TBCDSignals" address representation (international telephony)
static bool parseInterTele(const char * adr_str, unsigned & itype,
                              unsigned & iplan, char * signals);


class MobileAddressAC {
public:
  //Maximun number of octets in encoded/packed form
  //TonNpiIndicator octet + up to 19 octets of encoded/packed signals
  static const unsigned _maxAdrOctsLength = 20;
  //Maximum number of address signals
  static const unsigned _maxAdrValueLength = 38; // 2*(20-1)
  //Maximum number of characters in default string representation
  static const unsigned _maxStrValueLength = 44; // 38 + sizeof(".7.15.")

  struct TonNpiIndicator {
    uint8_t reserved : 1; //bit(s) : 7    (always = 1)
    uint8_t ToN      : 3; //bit(s) : 654
    uint8_t NPi      : 4; //bit(s) : 3210

    void reset(void) { reserved = 1; NPi = ToN = 0; }
  };

protected:
  //Function that parses one of possible string representation
  //of Mobile Address. 
  //Returns true on success
  typedef bool (*pfStrValParser)(const char * adr_str, unsigned & itype,
                               unsigned & iplan, char * signals);

  typedef std::list<pfStrValParser> ParsersList;
  typedef union _TON_NPI_Indicator_u {
    uint8_t         octet;
    TonNpiIndicator st;
  } TonNpiIndicator_u;

  TonNpiIndicator_u _ind;
  std::string       _signals;
  ParsersList       _parsers;

public:
  MobileAddressAC()
  {
    _ind.st.reset();
    _parsers.push_back(parseTonNpiTBCD);
  }
  virtual ~MobileAddressAC()
  { }

  const TonNpiIndicator & Indicator(void) const { return _ind.st; }
  uint8_t tonValue(void) const { return _ind.st.ToN; }
  uint8_t npiValue(void) const { return _ind.st.NPi; }
  //
  bool empty(void) const { return _signals.empty(); }
  //
  unsigned length(void) const { return (unsigned)_signals.length(); }
  //
  const char * getSignals(void) const { return _signals.c_str(); }
  //
  void clear(void) { _ind.st.reset(); _signals.clear(); }
  //
  void setToN(uint8_t use_ton) { _ind.st.ToN = (use_ton & 0x07); }
  //
  void setNPi(uint8_t use_npi) { _ind.st.NPi = (use_npi & 0x0F); }
  //
  void setSignals(const char * use_signals) { _signals = use_signals; }

  //Returns maximum allowed address value length (number of signals)
  unsigned maxAdrValueLen(void) const { return (maxAdrOctsLen() - 1)<<1; }
  //Returns maximum allowed number of characters in address string representation
  unsigned maxStrValueLen(void) const
  {
    return maxAdrValueLen() + (unsigned)sizeof(".7.15.");
  }

  //Prints address TonNpi prefix as ".ToN.NPi."
  unsigned printTonNpi(char * buf) const
  {
    return snprintf(buf, maxStrValueLen() - 1, ".%u.%u.",
            (unsigned)tonValue(), (unsigned)npiValue());
  }
  //Converts address to one of possible string representation,
  //by default to ".ToN.NPi.Signals".
  //Returns number of characters printed.
  //NOTE: buffer must be at least maxStrValueLen() bytes long
  unsigned toString(char * buf, bool print_ton_npi = true) const;
  //Converts address to one of possible string representation,
  //by default to ".ToN.NPi.Signals".
  std::string toString(bool print_ton_npi = true) const
  {
    char buf[_maxStrValueLength];
    toString(buf, print_ton_npi);
    return buf;
  }
  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  bool fromString(const char * adr_str);


  // --------------------------------------
  // -- MobileAddressAC interface methods
  // --------------------------------------
  //Returns maximum encoded octets length,
  //must be [1 .. MobileAddressAC::_maxOctsLength]
  virtual unsigned maxAdrOctsLen(void) const = 0;
  //Encodes/Packs address to octet buffer
  //NOTE: buffer must be able to store maxOctsLen() bytes!!!
  virtual unsigned pack2Octs(uint8_t * use_buf) const = 0;
  //Decodes/Unpacks address from octet buffer
  virtual unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len) = 0;

  //Prints address TonNpi prefix, by default as ".ToN.NPi."
  virtual unsigned printPrefix(char * buf, bool print_ton_npi = true) const
  {
    return print_ton_npi ? printTonNpi(buf) : 0;
  }
};


//MAP-CommonDataTypes.AddressString ::=
//                   OCTET STRING (SIZE (1..maxAddressLength))
// This type is used to represent a number for addressing purposes.
// It is composed of
// a) one octet for nature of address, and numbering plan indicator.
// b) digits of an address encoded as TBCD-String.
// 
// maxAddressLength  INTEGER ::= 20
class MobileAddress : public MobileAddressAC {
public:
  //Maximun number of octets in encoded/packed form
  //TonNpiIndicator octet + up to 19 octets of encoded/packed signals
  static const unsigned _mapAdrOctsLength = 20;

  enum TypeOfNumber_e { // 3 bits values
    tonUnknown = 0x0, tonInternational = 0x1, tonNationalSign = 0x2,
    tonNetworkSpec = 0x3, tonSubscriber = 0x4, tonReserved5 = 0x5,
    tonAbbreviated = 0x6, tonReservedExt = 0x7
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


  MobileAddress()
  {
    _parsers.push_back(parseInterTele);
    _parsers.push_back(parseUnknownTele);
  }

  TypeOfNumber_e ToN(void) const
  {
    return static_cast<TypeOfNumber_e>(tonValue());
  }

  NumberingPlan_e NPi(void) const
  {
    return static_cast<NumberingPlan_e>(npiValue());
  }

  //Returns true if address TonNpi is .International.ISDN
  bool interISDN(void) const
  {
    return ((ToN() == tonInternational) && (NPi() == npiISDNTele_e164));
  }
  // --------------------------------------
  // // MobileAddressAC interface methods
  // --------------------------------------
  //Returns maximum encoded octets length,
  //must be [1 .. MobileAddressAC::_maxOctsLength]
  unsigned maxAdrOctsLen(void) const { return _mapAdrOctsLength; }
  //Encodes/Packs address to octet buffer
  //NOTE: specified buffer must be able to store maxOctsLen() bytes!!!
  unsigned pack2Octs(uint8_t * use_buf) const
  {
    return TBCDString::pack2Octs(use_buf, _signals.c_str(), (unsigned)_signals.length());
  }
  //Decodes/Unpacks address from octet buffer
  unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len)
  {
    return TBCDString::unpackOcts(use_buf, buf_len, _signals);
  }

  //Prints address TonNpi prefix, by default as ".ToN.NPi."
  //In case of International ISDN numbering, prefix is printed as "+"
  //as recommended in E.164 clause 12
  unsigned printPrefix(char * buf, bool print_ton_npi = true) const
  {
    unsigned n = 0;
    if (interISDN() && !print_ton_npi)
      buf[n++] = '+';
    else
      n = MobileAddressAC::printTonNpi(buf);
    return n;
  }

};

//MAP-CommonDataTypes.ISDN-AddressString ::=
//                    AddressString (SIZE (1..maxISDN-AddressLength))
// This type is used to represent ISDN numbers.
// 
// maxISDN-AddressLength  INTEGER ::= 9
class ISDNAddress : public MobileAddress {
public:
  //Maximun number of octets in encoded/packed form
  //TonNpiIndicator octet + up to 8 octets of encoded/packed signals
  static const unsigned _isdnAdrOctsLength = 9;

  ISDNAddress() : MobileAddress()
  { }

  // --------------------------------------
  // // MobileAddressAC interface methods
  // --------------------------------------
  //Returns maximum encoded octets length,
  //must be <= MobileAddressAC::_maxOctsLength
  unsigned maxOctsLen(void) const { return _isdnAdrOctsLength; }
};


} //map
} //eyeline

#endif /* __EYELINE_MAP_MOBILE_ADDRESS_HPP */

