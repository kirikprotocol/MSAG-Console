#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/abprov/AbonentSubscription.hpp"
#include "util/vformat.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {

using smsc::util::format;

/* ************************************************************************** *
 * class AbonentSubscription implementation:
 * ************************************************************************** */
size_t AbonentSubscription::str_size(void) const
{
  return sizeof("contract %s, IMSI %s, %s, VLR %s")
          + csiSCF.str_size() + TonNpiAddress::_strSZ;
}

//Appends to use_str
void AbonentSubscription::toString(std::string & use_str) const
{
  //use_str.reserve(use_str.size() + str_size());
  format(use_str, "contract %s, IMSI %s, %s, VLR %s",
         type2Str(), imsiCStr(), csiSCF.toString().c_str(),
         vlrNum.empty() ? vlrNum.toString().c_str() : "<none>" );
}

std::string AbonentSubscription::toString(void) const
{
  std::string str(0, str_size());
  toString(str);
  return str;
}

//Returns true if at least one parameter was updated
bool AbonentSubscription::Merge(const AbonentSubscription & use_rcd)
{
  bool rval = false;

  if (!use_rcd.isUnknown()) {
    abType = use_rcd.abType;
    rval = true;
  }
  if (!use_rcd.abImsi.empty()) {
    abImsi = use_rcd.abImsi;
    rval = true;
  }
  if (!use_rcd.csiSCF.empty()) {
    csiSCF.Merge(use_rcd.csiSCF);
    rval = true;
  }
  if (!use_rcd.vlrNum.empty()) {
    vlrNum = use_rcd.vlrNum;
    rval = true;
  }
  if (rval && use_rcd.tmQueried)
    tmQueried = use_rcd.tmQueried;
  return rval;
}

} //iaprvd
} //inman
} //smsc


