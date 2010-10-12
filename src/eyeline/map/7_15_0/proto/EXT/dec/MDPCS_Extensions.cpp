#include "MDPCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace dec {

void
MDPCS_Extensions::construct()
{
  setUnkExtension(0);
}

/*
 PCS-Extensions ::= SEQUENCE { ... }
 */
asn1::ber::TypeDecoderAC*
MDPCS_Extensions::prepareAlternative(uint16_t unique_idx)
{
  if (!_value)
    throw smsc::util::Exception("MDPCS_Extensions::prepareAlternative: value isn't set!");
  if (unique_idx > 0)
    throw smsc::util::Exception("MDPCS_Extensions::prepareAlternative: undefined UId");

  _uext.init(getTSRule()).setValue(_value->_unkExt);
  return _uext.get();

}

}}}}
