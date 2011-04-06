#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/iapmgr/ICSIAPManager.hpp"

namespace smsc {
namespace inman {
namespace iapmgr {
/* ************************************************************************** *
 * class ICSIAPManager implementation:
 * ************************************************************************** */

//Initializes service verifying that all dependent services are inited
ICServiceAC::RCode ICSIAPManager::_icsInit(void)
{
  //build address pools registry
  if (!_cfg->poolsReg.Count())
    _cfg->initPoolsRegistry();

  //initialize all required IAProviders and store their interfaces
  for (IAPrvdsRegistry::const_iterator
       it = _cfg->prvdReg->begin(); it != _cfg->prvdReg->end(); ++it) {
    ICServiceAC * pSrv = _icsHost->getICService(it.pValue()->_icsUId);
    if (!pSrv || (pSrv->icsState() < ICServiceAC::icsStInited))
      return ICServiceAC::icsRcError;
    IAProviderAC * iface = (IAProviderAC *)(pSrv->Interface());
    it.pValue()->_iface = iface;
    smsc_log_info(logger, "%s: AbonentProvider '%s': type %s",
                  _logId, it->first.c_str(), iface->getProperty().toString().c_str());
  }

  //bind IAProviders and associated policies
  return  _cfg->policiesReg.bindProviders(*(_cfg->prvdReg.get())) ?
            ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
}

} //iapmgr
} //inman
} //smsc

