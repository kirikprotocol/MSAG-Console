#ifndef MOD_IDENT_OFF
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_OFF */

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
  //initailize all required IAProviders and store their interfaces
  IAPrvdsRegistry::const_iterator it = _cfg->prvdReg->begin();
  for (; it != _cfg->prvdReg->end(); ++it) {
    ICServiceAC * pSrv = _icsHost->getICService(it.pValue()->_icsUId);
    if (!pSrv || (pSrv->icsState() < ICServiceAC::icsStInited))
      return ICServiceAC::icsRcError;
    IAProviderITF * iface = (IAProviderITF *)(pSrv->Interface());
    it.pValue()->_iface = iface;
    smsc_log_info(logger, "%s: AbonentProvider '%s': type %s, ident: %s, ability: %s",
                  _logId, it->first.c_str(), IAProviderITF::nmType(iface->type()),
                  iface->ident(), IAProviderITF::nmAbility(iface->ability()));
  }
  //bind IAProviders and associated policies
  return  _cfg->polReg.Init(*(_cfg->prvdReg.get())) ?
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

