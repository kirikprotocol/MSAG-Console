#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/abdtcr/ICSAbntDtcr.hpp"
using smsc::core::synchronization::MutexGuard;
using smsc::inman::tcpsrv::ICSTcpServerIface;

namespace smsc {
namespace inman {
namespace abdtcr {
/* ************************************************************************** *
 * class ICSAbntDetector implementation.
 * ************************************************************************** */
const INPAbntContract  ICSAbntDetector::_iProtoDef; //provided protocol definition

ICSAbntDetector::~ICSAbntDetector()
{
  ICSStop(true);
  {
    MutexGuard grd(_sync);
    ICSTcpServerIface * tcpSrv = (ICSTcpServerIface *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
    while (tcpSrv && !tcpSrv->unregisterProtocol(_iProtoDef._protoId))
      _sync.wait();
  }
}

// ---------------------------------------------
// -- ICServiceAC interface methods
// --------------------------------------------- 
//NOTE: all methods are called with _sync locked!
ICServiceAC::RCode ICSAbntDetector::_icsInit(void)
{
  _wCfg.iapMgr = (const IAPManagerITF *)
                _icsHost->getInterface(ICSIdent::icsIdIAPManager);
  //check that default policy is configured
  const AbonentPolicy * dfltPol = _wCfg.iapMgr->getPolicy(_wCfg.policyNm);
  if (!dfltPol) {
    smsc_log_fatal(logger, "%s: IAPolicy %s is not configured!", _logId,
                   _wCfg.policyNm.c_str());
    return ICServiceAC::icsRcError;
  }
  if (_wCfg.useCache)
    _wCfg.abCache = (AbonentCacheITF*)_icsHost->getInterface(ICSIdent::icsIdAbntCache);
  //there is only one timeout value AbonentDetectors use.
  smsc::core::timers::TimeWatchersRegistryITF *
    icsTW = (smsc::core::timers::TimeWatchersRegistryITF *)_icsHost->getInterface(ICSIdent::icsIdTimeWatcher);
  _wCfg.abtTimeout.Init(icsTW, _wCfg.maxRequests);

  ICSTcpServerIface * tcpSrv = (ICSTcpServerIface *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
  tcpSrv->registerProtocol(*this);

  return ICServiceAC::icsRcOk;
}

void ICSAbntDetector::_icsStop(bool do_wait/* = true*/)
{
  bool waitMode = false;
  do {
    SessionsRegistry::const_iterator it = _sessReg.begin();
    for (; it != _sessReg.end(); ++it)
      it->second->stop(waitMode);
  } while (waitMode++ < do_wait);
}

// -------------------------------------
// ICSConnServiceIface interface methods:
// -------------------------------------
//Creates a connect listener serving given connect.
//Returns true on success, false -  otherwise.
//Note: upon entry the referenced Connect is configured 
//  to process in consequitive mode, so it's recommended
//  to reconfigure Connect within this call.
bool ICSAbntDetector::setConnListener(const ConnectGuard & use_conn) /*throw()*/
{
  MutexGuard  grd(_sync);
  if (_icsState != ICServiceAC::icsStStarted) {
    _sync.notify();
    return false;
  }

  AbntDetectorManager * pMgr = _sessReg.find(use_conn->getId());
  if (pMgr) {
    smsc_log_warn(logger, "%s: session[%s] is already opened on Connect[%u]",
                  _logId,  pMgr->mgrId(), (unsigned)use_conn->getId());
    _sync.notify();
    return false;
  }
  //create new connect manager
  pMgr = new AbntDetectorManager(_wCfg, _iProtoDef, ++_lastSessId, logger);
  _sessReg.insert(use_conn->getId(), pMgr);
  pMgr->bind(&use_conn);
  pMgr->start(); //switch Connect to asynchronous mode
  _sync.notify();
  return true;
}

//Notifies that given connection is to be closed, no more socket events will be reported.
void ICSAbntDetector::onDisconnect(const ConnectGuard & use_conn) /*throw()*/
{
  std::auto_ptr<AbntDetectorManager> pMgr;
  {
    MutexGuard  grd(_sync);
    pMgr.reset(_sessReg.extract(use_conn->getId()));
    if (!pMgr.get()) {
      _sync.notify();
      return;
    }
  }
  pMgr->bind(NULL);
  pMgr->stop(false);
  smsc_log_debug(logger, "%s: closing session[%s] on Connect[%u]",
                _logId,  pMgr->mgrId(), (unsigned)use_conn->getId());
  _sync.notify();
  //pMgr.reset(); //AbntDetectorManager is destroyed at this point
}

} //abdtcr
} //inman
} //smsc

