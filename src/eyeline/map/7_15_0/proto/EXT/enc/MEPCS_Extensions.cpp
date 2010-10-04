#include "MEPCS_Extensions.hpp"

namespace eyeline {
namespace map {
namespace ext {
namespace enc {

void
MEPCS_Extensions::setValue(const PCS_Extensions& value)
{
  clearFields(0);
  if (!value._unkExt._tsList.empty())
    _eUnkExt.init().setValue(value._unkExt, *this, 0);
}

}}}}
