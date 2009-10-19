#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/abprov/IAProvider.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

/* ************************************************************************** *
 * class IAProviderITF implementation:
 * ************************************************************************** */
const char *  IAProviderITF::nmType(Type_e val_type)
{
  switch (val_type) {
  case iapDB:   return "iapDB";
  case iapHLR:  return "iapHLR";
  case iapATSI: return "iapATSI";
  case iapUnknown:
  default:;
  }
  return "iapUnknown";
}
const char *  IAProviderITF::nmAbility(Ability_e val_type)
{
  switch (val_type) {
  case abContractSCF: return "abContractSCF";
  case abSCF:         return "abSCF";
  case abContract:    return "abContract";
  case abNone:
  default:;
  }
  return "abNone";
}

} //iaprvd
} //inman
} //smsc


