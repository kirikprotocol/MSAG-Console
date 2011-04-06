#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
/* ************************************************************************** *
 * class IAPAbility implementation:
 * ************************************************************************** */
IAPAbilityStr_t IAPAbility::toString(void) const
{
  bool            first = true;
  IAPAbilityStr_t str;

  str += '{';
  if (value.st.contract) {
    str += "contract"; first = false;
  }
  if (value.st.gsmSCF) {
    if (!first) {
      str += ',';  first = false;
    }
    str += "gsmSCF";
  }
  if (value.st.imsi) {
    if (!first) {
      str += ',';  first = false;
    }
    str += "IMSI";
  }
  if (value.st.vlrNum) {
    if (!first) {
      str += ',';  first = false;
    }
    str += "VLR";
  }
  if (value.st.odbGD) {
    if (!first) {
      str += ',';  first = false;
    }
    str += "ODB";
  }
  str += '}';
  return str;
}

/* ************************************************************************** *
 * class IAPProperty implementation:
 * ************************************************************************** */
IAPTypeStr_t IAPProperty::toString(void) const
{
  IAPTypeStr_t  rval;
  snprintf(rval.str, IAPTypeStr_t::MAX_SZ-1, "%s%s",
           _iapIdent.c_str(), _iapAbility.toString().c_str());
  return rval;
}

} //iaprvd
} //inman
} //smsc


