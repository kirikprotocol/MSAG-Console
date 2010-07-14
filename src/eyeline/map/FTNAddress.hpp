/* ************************************************************************** *
 * Forwarded-to-number Address String.
 * ************************************************************************** */
#ifndef __EYELINE_MAP_FTN_ADDRESS_HPP
#ident "@(#)$Id$"
#define __EYELINE_MAP_FTN_ADDRESS_HPP

#include "eyeline/map/MobileAddress.hpp"

namespace eyeline {
namespace map {

// MAP-CommonDataTypes.FTN-AddressString ::=
//                    AddressString (SIZE (1..maxFTN-AddressLength))
// -- This type is used to represent forwarded-to numbers. 
// -- If NAI = international the first digits represent the country code (CC)
// -- and the network destination code (NDC) as for E.164.
//
// maxFTN-AddressLength  INTEGER ::= 15
//
static const uint8_t _maxFTNAddressOcts = 15;
//
class FTNAddress : public MAPAddressStringAC {
private:
  char  _store[MAX_ADDRESS_SIGNALS(_maxFTNAddressOcts) + 1];

public:
  explicit FTNAddress()
    : MAPAddressStringAC(_maxFTNAddressOcts, _store)
  { }
  FTNAddress(const FTNAddress & use_obj)
    : MAPAddressStringAC(use_obj)
  {
    _signals = _store;
  }
  ~FTNAddress()
  { }

  // -- MAPAddressStringAC interface method
  //Converts address from one of possible string representation
  //NOTE: supports at least ".ToN.NPi.Signals" form
  virtual bool fromString(const char * adr_str)
  {
    return MAPAddressStringAC::parseString(adr_str);
  }
};


} //map
} //eyeline

#endif /* __EYELINE_MAP_FTN_ADDRESS_HPP */

