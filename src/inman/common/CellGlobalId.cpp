#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/common/CellGlobalId.hpp"

namespace smsc   {
namespace inman  {
/* ************************************************************************** *
 * CellGlobalId implementation
 * ************************************************************************** */
/* Cell Global Identification
   The internal structure is defined as follows:
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

//NOTE: value must be validated prior to this call!
void CellGlobalId::pack2Octs(uint8_t (& oct_buf)[7]) const
{
  //MCC & MNC
  oct_buf[0] = _mcc.char2uintByIdx(0) | (_mcc.char2uintByIdx(1) << 4);
  oct_buf[1] = _mcc.char2uintByIdx(2);
  oct_buf[1] |= (_mnc.length() == 3) ? (_mnc.char2uintByIdx(2) << 4) : 0xF0;
  oct_buf[2] = _mnc.char2uintByIdx(0) | (_mnc.char2uintByIdx(1) << 4);
  //LAC (MSB-2-LSB order)
  oct_buf[3] = (_lac.char2uintByIdx(0) << 4) | _lac.char2uintByIdx(1);
  oct_buf[4] = (_lac.char2uintByIdx(2) << 4) | _lac.char2uintByIdx(3);
  //CI (MSB-2-LSB order)
  oct_buf[5] = (_ci.char2uintByIdx(0) << 4) | _ci.char2uintByIdx(1);
  oct_buf[6] = (_ci.char2uintByIdx(2) << 4) | _ci.char2uintByIdx(3);
}

void CellGlobalId::unpackOcts(const uint8_t (& oct_buf)[7])
{
  //MCC & MNC
  _mcc.uint2charByIdx(0, oct_buf[0] & 0x0F);
  _mcc.uint2charByIdx(1, oct_buf[0] >> 4);
  _mcc.uint2charByIdx(2, oct_buf[1] & 0x0F);
  //NOTE: in case of 2-bytes MNC the filler 0x0F is converted to NumericString_T::EOS
  //because of it's illegal binary value for NumericString_T char.
  _mnc.uint2charByIdx(2, oct_buf[1] >> 4);
  _mnc.uint2charByIdx(0, oct_buf[2] & 0x0F);
  _mnc.uint2charByIdx(1, oct_buf[2] >> 4);
  //LAC (MSB-2-LSB order)
  _lac.uint2charByIdx(0, oct_buf[3] >> 4);
  _lac.uint2charByIdx(1, oct_buf[3] & 0x0F);
  _lac.uint2charByIdx(2, oct_buf[4] >> 4);
  _lac.uint2charByIdx(3, oct_buf[4] & 0x0F);
  //CI (MSB-2-LSB order)
  _ci.uint2charByIdx(0, oct_buf[5] >> 4);
  _ci.uint2charByIdx(1, oct_buf[5] & 0x0F);
  _ci.uint2charByIdx(2, oct_buf[6] >> 4);
  _ci.uint2charByIdx(3, oct_buf[6] & 0x0F);
}

CellGlobalId::StringForm_t CellGlobalId::toString(void) const
{
  StringForm_t rval("{");
  strncat(rval.str, _mcc._value, _mcc._MAX_SZ);
  rval += ','; strncat(rval.str, _mnc._value, _mnc._MAX_SZ);
  rval += ','; strncat(rval.str, _lac._value, _lac._MAX_SZ);
  rval += ','; strncat(rval.str, _ci._value, _ci._MAX_SZ);
  rval += '}';
  return rval;
}

} //inman
} //smsc

