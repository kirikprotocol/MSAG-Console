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

  //initailize all required IAProviders and store their interfaces
  IAPrvdsRegistry::const_iterator it = _cfg->prvdReg->begin();
  for (; it != _cfg->prvdReg->end(); ++it) {
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

//Stops service
void  ICSIAPManager::_icsStop(bool do_wait/* = false*/)
{ //cancel quieries of all controlled IAProviders
  IAPrvdsRegistry::const_iterator it = _cfg->prvdReg->begin();
  for (; it != _cfg->prvdReg->end(); ++it) {
    IAProviderInfo * pInfo = it.pValue();
    if (pInfo->_iface) {
      pInfo->_iface->cancelAllQueries();
      pInfo->_iface = NULL;
    }
  }
  //IAProviders will be finally stopped later by _icsHost
}

} //iapmgr
} //inman
} //smsc

