/* ************************************************************************** *
 * UMTS: Cell Global Identification.
 * ************************************************************************** */
#ifndef __INMAN_UMTS_CELL_GID_HPP
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define __INMAN_UMTS_CELL_GID_HPP

#include "inman/common/UMTSTypes.hpp"
#include "core/buffers/FixedLengthString.hpp"

namespace smsc {
namespace inman {

/* The internal structure is defined in 3GPP TS 23.003 as follows:
   octet 1 bits 4321    Mobile Country Code 1st digit
           bits 8765    Mobile Country Code 2nd digit
   octet 2 bits 4321    Mobile Country Code 3rd digit
           bits 8765    Mobile Network Code 3rd digit
                        or filler (1111) for 2 digit MNCs
   octet 3 bits 4321    Mobile Network Code 1st digit
           bits 8765    Mobile Network Code 2nd digit
   octets 4 and 5       Location Area Code according to 3GPP TS 24.008
   octets 6 and 7       Cell Identity (CI) value according to 3GPP TS 23.003
*/
struct CellGlobalId {
  static const size_t _StrSZ = sizeof("{MCC,MNC,LACH,CIHH}") + 2;
  typedef smsc::core::buffers::FixedLengthString<_StrSZ> StringForm_t;

  MobileCountryCode   _mcc;
  MobileNetworkCode   _mnc;
  LocationAreaCode    _lac;
  CellIdentity        _ci;

  bool empty(void) const
  {
    return _mcc.empty() || _mnc.empty() || _lac.empty() || _ci.empty();
  }
  bool validate(void) const
  {
    return _mcc.validate() && _mnc.validate() && _lac.validate() && _ci.validate();
  }

  void clear(void)
  {
    _mcc.clear(); _mnc.clear();
    _lac.clear(); _ci.clear();
  }

  //NOTE: value must be validated prior to this call!
  void pack2Octs(uint8_t (& oct_buf)[7]) const;

  //NOTE: it's recommneded to validate value after this call!
  void unpackOcts(const uint8_t (& oct_buf)[7]);

  StringForm_t  toString(void) const;
};
typedef CellGlobalId::StringForm_t CellGlobalIdString_t;

}//inman
}//smsc
#endif /* __INMAN_UMTS_CELL_GID_HPP */

