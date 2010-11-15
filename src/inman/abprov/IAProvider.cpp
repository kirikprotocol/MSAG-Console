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
    str += "contract";
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
  str += '}';
  return str;
}

/* ************************************************************************** *
 * class IAPType implementation:
 * ************************************************************************** */
IAPProperty  _knownIAPType[] = {
    IAPProperty(IAPProperty::iapUnknown, "iapUnknown", 0)
  , IAPProperty(IAPProperty::iapDB, "iapDB", IAPAbility::abContract)
  , IAPProperty(IAPProperty::iapCHSRI, "iapCHSRI", IAPAbility::abContract | IAPAbility::abSCF
                       | IAPAbility::abIMSI | IAPAbility::abVLR)
  , IAPProperty(IAPProperty::iapATSI, "iapATSI", IAPAbility::abContract | IAPAbility::abSCF)
};


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


