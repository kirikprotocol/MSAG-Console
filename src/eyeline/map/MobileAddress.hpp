/* ************************************************************************** *
 * Base class for various mobile addresses.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_MOBILE_ADDRESS_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __EYELINE_MAP_MOBILE_ADDRESS_HPP

#include <inttypes.h>

#include "core/buffers/FixedLengthString.hpp"

namespace eyeline {
namespace map {

struct TonNpiIndicator {
  uint8_t _reserved : 1; //bit(s) : 7    (always = 1)
  uint8_t _ToN      : 3; //bit(s) : 654
  uint8_t _NPi      : 4; //bit(s) : 3210

  TonNpiIndicator() : _reserved(1), _ToN(0), _NPi(0)
  { }
  ~TonNpiIndicator()
  { }

  void clear(void) { _reserved = 1; _NPi = _ToN = 0; }

  uint8_t pack2Oct(void) const
  {
    return _NPi | (_ToN << 4) | (uint8_t)(0x80);
  }

  void unpackOct(uint8_t use_oct)
  {
    _reserved = use_oct >> 7;
    _NPi = use_oct & 0x0F;
    _ToN = (use_oct >> 4) & 0x07;
  }
};

//Maximum number of address signals
#define MAX_ADDRESS_SIGNALS(max_octs) ((max_octs - 1)*2)
//Maximum number of characters in textual representation of AddressString
#define MAX_ADDRESS_TXT_LEN(max_octs) ((unsigned)(sizeof(".7.15.") + (max_octs - 1)*2))

//MAP-CommonDataTypes.AddressString ::=
//                   OCTET STRING (SIZE (1..maxAddressLength))
// This type is used to represent a number for addressing purposes
// (MobileAddress, ISDNAddress, FTNAddress).
// 
// It is composed of
// a) one octet for nature of address, and numbering plan indicator.
// b) digits of an address encoded as TBCD-String (number of digits
//    depends on address kind).
//
static const uint8_t _maxAddressStringOcts = 20;
//
static const unsigned _maxAddressValueStringLength = MAX_ADDRESS_TXT_LEN(_maxAddressStringOcts);
//Maximum possible number of characters in textual representation
typedef smsc::core::buffers::FixedLengthString<_maxAddressValueStringLength + 1>
  AddressValueString_t;

class MAPAddressStringAC {
protected:
  const uint8_t   _maxOcts;
  TonNpiIndicator _ind;
  char *          _signals;

  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  bool parseString(const char * adr_str);
  //Prints address TonNpi prefix, by default as ".ToN.NPi."
  //In case of International ISDN numbering, prefix is printed as "+"
  //as recommended in E.164 clause 12
  unsigned printPrefix(char * buf, bool print_ton_npi = true) const;

  //
  MAPAddressStringAC(uint8_t max_octs, char * signals_store)
    : _maxOcts(max_octs), _signals(signals_store)
  { }
  //NOTE: copying constructor of successor must properly set _signals
  MAPAddressStringAC(const MAPAddressStringAC & use_obj)
    : _maxOcts(use_obj._maxOcts), _ind(use_obj._ind), _signals(0)
  { }

public:
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

  virtual ~MAPAddressStringAC()
  { }

  //Maximun number of octets in encoded/packed form
  unsigned maxOctets(void) const { return _maxOcts; }
  //Maximum number of address signals
  unsigned maxSignals(void) const { return (_maxOcts - 1)*2; }
  //Maximum number of characters in textual representation
  unsigned maxValueStrLength(void) const { return MAX_ADDRESS_TXT_LEN(_maxOcts); }

  //Encodes/Packs address to octet buffer
  //NOTE: buffer must be able to store maxOctets() bytes!!!
  unsigned pack2Octs(uint8_t * use_buf) const;
  //Decodes/Unpacks address from octet buffer
  unsigned unpackOcts(const uint8_t * use_buf, unsigned buf_len);


  bool isInterISDN(void) const
  {
    return (_ind._ToN == tonInternational) && (_ind._NPi == npiISDNTele_e164);
  }
  //
  uint8_t getToNValue(void) const { return _ind._ToN; }
  //
  uint8_t getNPiValue(void) const { return _ind._NPi; }
  //
  bool empty(void) const { return _signals[0] == 0; }
  //
  unsigned length(void) const { return (unsigned)strlen(_signals); }
  //
  const char * getSignals(void) const { return _signals; }

  //
  void clear(void) { _ind.clear(); _signals[0] = 0; }
  //
  void setToN(uint8_t use_ton) { _ind._ToN = (use_ton & 0x07); }
  //
  void setNPi(uint8_t use_npi) { _ind._NPi = (use_npi & 0x0F); }
  //
  void setSignals(const char * use_signals)
  {
    strncpy(_signals, use_signals, maxSignals());
  }

  //Converts address to one of possible string representation,
  //by default to ".ToN.NPi.Signals".
  //Returns number of characters printed.
  //NOTE: buffer must be at least maxValueStrLength() bytes long
  unsigned toString(char * buf, unsigned buf_sz, bool print_ton_npi = true) const;

  //Converts address to one of possible string representation,
  //by default to ".ToN.NPi.Signals".
  AddressValueString_t toString(bool print_ton_npi = true) const
  {
    AddressValueString_t  str;
    toString(str.str, _maxAddressValueStringLength + 1, print_ton_npi);
    return str;
  }

  // -------------------------------------
  // -- MAPAddressStringAC interface method
  // -------------------------------------
  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  virtual bool fromString(const char * adr_str) = 0;
};

class MobileAddress : public MAPAddressStringAC {
private:
  char  _store[MAX_ADDRESS_SIGNALS(_maxAddressStringOcts) + 1];

public:
  explicit MobileAddress()
    : MAPAddressStringAC(_maxAddressStringOcts, _store)
  { }
  MobileAddress(const MobileAddress & use_obj)
    : MAPAddressStringAC(use_obj)
  {
    _signals = _store;
  }
  ~MobileAddress()
  { }

  // -------------------------------------
  // -- MAPAddressStringAC interface method
  // -------------------------------------
  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  virtual bool fromString(const char * adr_str)
  {
    return MAPAddressStringAC::parseString(adr_str);
  }
};

} //map
} //eyeline

#endif /* __EYELINE_MAP_MOBILE_ADDRESS_HPP */

