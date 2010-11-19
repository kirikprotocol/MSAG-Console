#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/AbonentPolicy.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class ISDNAddressMask implementation:
 * ************************************************************************** */
const char * ISDNAddressMask::_numberingFmt = "%15[0-9\?]%2s";

bool ISDNAddressMask::fromText(const char * in_text, char * out_str)
{
  char  trash[4];
  int   scanned = sscanf(in_text, _numberingFmt, out_str, trash);
  return (scanned == 1) && (strlen(out_str) >= 5);
}

/* ************************************************************************** *
 * class AbonentPolicy implementation:
 * ************************************************************************** */
AbonentPolicy::AbonentPolicy(const char * use_id, const CStrList & nm_prvds)
  : _ident(use_id)
{
  if (!nm_prvds.empty()) {
    CStrList::const_iterator  cit = nm_prvds.begin();
    _prvdPrio.first._ident = cit->c_str();
    if ((++cit) != nm_prvds.end())
      _prvdPrio.second._ident = cit->c_str();
  }
}


//Returns true if at least one AbonentProvider is initialized
bool AbonentPolicy::bindProviders(const IAPrvdsRegistry & prvd_reg)
{
  bool  rval = false;
  const IAProviderInfo * pInfo = prvd_reg.find(_prvdPrio.first._ident);

  if (pInfo) {
    _prvdPrio.first._icsUId = pInfo->_icsUId;
    _prvdPrio.first._iface = pInfo->_iface;
    rval = true;
  }
  if (!_prvdPrio.first.empty()) {
    if ((pInfo = prvd_reg.find(_prvdPrio.second._ident))) {
      _prvdPrio.second._icsUId = pInfo->_icsUId;
      _prvdPrio.second._iface = pInfo->_iface;
      rval = true;
    }
  }
  return rval;
}


const IAProviderInfo * AbonentPolicy::getIAProvider(IAPPrio_e prvd_prio) const
{
  if (prvd_prio == AbonentPolicy::iapPrimary)
    return (_prvdPrio.first.empty() ? NULL : &_prvdPrio.first);
  if (prvd_prio == AbonentPolicy::iapSecondary)
    return (_prvdPrio.second.empty() ? NULL : &_prvdPrio.second);
  return NULL;
}

} //iapmgr
} //inman
} //smsc

