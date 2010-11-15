#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/IAPMgrCfg.hpp"

namespace smsc  {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class PoliciesCfgReg implementation:
 * ************************************************************************** */
bool PoliciesCfgReg::bindProviders(const IAPrvdsRegistry & prvd_reg)
{
  bool failed = false;
  for (TRegistry::iterator sit = registry.begin(); sit != registry.end(); ++sit)
    failed |= !sit->second->bindProviders(prvd_reg);
  return !failed;
}

/* ************************************************************************** *
 * class IAPManagerCFG implementation:
 * ************************************************************************** */
//constructs address pools registry from policies registry
int IAPManagerCFG::initPoolsRegistry(void)
{
  for (PoliciesCfgReg::const_iterator
       cit = policiesReg.begin(); cit != policiesReg.end(); ++cit) {
    const AbonentPolicy * iaPol = cit->second;
    if (!iaPol->_poolsSet.empty()) {
      for (AddressPoolsSet::const_iterator
           pIt = iaPol->_poolsSet.begin(); pIt != iaPol->_poolsSet.end(); ++pIt) {
        poolsReg.Insert(pIt->_mask.c_str(), IAPRule(pIt->_name.c_str(), iaPol));
      }
    }
  }
  return poolsReg.Count();
}

} //iapmgr
} //inman
} //smsc


