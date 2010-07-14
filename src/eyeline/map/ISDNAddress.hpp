/* ************************************************************************** *
 * ISDN Address String.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_ISDN_ADDRESS_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_ISDN_ADDRESS_HPP

#include "eyeline/map/MobileAddress.hpp"

namespace eyeline {
namespace map {

//MAP-CommonDataTypes.ISDN-AddressString ::=
//                    AddressString (SIZE (1..maxISDN-AddressLength))
// This type is used to represent ISDN numbers.
// 
// maxISDN-AddressLength  INTEGER ::= 9
//
static const uint8_t _maxISDNAddressOcts = 9;
//
class ISDNAddress : public MAPAddressStringAC {
private:
  char  _store[MAX_ADDRESS_SIGNALS(_maxISDNAddressOcts) + 1];

public:
  explicit ISDNAddress()
    : MAPAddressStringAC(_maxISDNAddressOcts, _store)
  { }
  ISDNAddress(const ISDNAddress & use_obj)
    : MAPAddressStringAC(use_obj)
  {
    _signals = _store;
  }
  ~ISDNAddress()
  { }

  // -- MAPAddressStringAC interface method
  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  virtual bool fromString(const char * adr_str)
  {
    if (MAPAddressStringAC::parseString(adr_str)
        && (_ind._NPi == MobileAddress::npiISDNTele_e164))
      return true;
    clear();
    return false;
  }
};

} //map
} //eyeline

#endif /* __EYELINE_MAP_ISDN_ADDRESS_HPP */

