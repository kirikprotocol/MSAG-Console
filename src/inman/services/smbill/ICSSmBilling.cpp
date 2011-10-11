#ifdef MOD_IDENT_ON
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/services/smbill/ICSSmBilling.hpp"
using smsc::inman::tcpsrv::ICSTcpServerIface;
using smsc::inman::iapmgr::IAPManagerITF;

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::_ac_cap3_sms;
using smsc::inman::comp::initCAP3SMSComponents;

namespace smsc {
namespace inman {
namespace smbill {
/* ************************************************************************** *
 * class ICSSmBilling implementation.
 * ************************************************************************** */
const INPBilling  ICSSmBilling::_iProtoDef; //provided protocol definition

ICSSmBilling::~ICSSmBilling()
{
  ICSStop(true);
  {
    MutexGuard grd(_sync);
    ICSTcpServerIface * tcpSrv = (ICSTcpServerIface *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
    while (tcpSrv && !tcpSrv->unregisterProtocol(_iProtoDef._protoId))
      _sync.wait();
  }
}

// -------------------------------------
// ICServiceAC interface methods:
// -------------------------------------
//Initializes service verifying that all dependent services are inited
ICServiceAC::RCode ICSSmBilling::_icsInit(void)
{
  if (_wCfg.prm->useCache) {
    _wCfg.abCache = (AbonentCacheITF*)_icsHost->getInterface(ICSIdent::icsIdAbntCache);
    if (!_wCfg.abCache) {
      smsc_log_warn(logger, "Abonents Cache service required but not loaded!");
      _wCfg.prm->useCache = false;
    }
  }

  //Initialize CAP3Sms components factory
  if (_wCfg.prm->capSms.get()) {
    _wCfg.tcDisp = (TCAPDispatcherITF *)_icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
    if (!_wCfg.tcDisp->acRegistry()->getFactory(_ac_cap3_sms)
        && !_wCfg.tcDisp->acRegistry()->regFactory(initCAP3SMSComponents)) {
      smsc_log_fatal(logger, "ROS factory registration failed: %s!", _ac_cap3_sms.nick());
      return ICServiceAC::icsRcError;
    }
    _wCfg.schedMgr = (TaskSchedulerFactoryITF*)_icsHost->getInterface(ICSIdent::icsIdScheduler);
  }
  //initialize CDR storage
  if (_wCfg.prm->cdrMode) {
    _wCfg.bfs.reset(new InBillingFileStorage(_wCfg.prm->cdrDir, 0, logger));
    int oldfs = _wCfg.bfs->RFSOpen(true);
    if (oldfs < 0)
      return ICServiceAC::icsRcError;

    smsc_log_debug(logger, "%s: CDR storage opened%s", _logId,
                   oldfs > 0 ? ", old files rolled": "");
     //initialize external storage roller
    if (_wCfg.prm->cdrInterval) {
      _roller.reset(new InFileStorageRoller(_wCfg.bfs.get(),
                          (unsigned long)_wCfg.prm->cdrInterval));
      smsc_log_debug(logger, "%s: CDR storage roller inited", _logId);
    }
  }

  if (_wCfg.prm->needIAProvider()) {
    _wCfg.iapMgr = (const IAPManagerITF *)
                  _icsHost->getInterface(ICSIdent::icsIdIAPManager);
    //check that default policy is configured
    const AbonentPolicy * dfltPol = _wCfg.iapMgr->getPolicy(_wCfg.policyNm);
    if (!dfltPol) {
      smsc_log_fatal(logger, "%s: IAPolicy %s is not configured!", _logId,
                     _wCfg.policyNm.c_str());
      return ICServiceAC::icsRcError;
    }
  }

  //Smbilling uses up to two timeout values.
  TimeWatcherIface * icsTW = (TimeWatcherIface *)_icsHost->getInterface(ICSIdent::icsIdTimeWatcher);
  if (_wCfg.iapMgr)
    _wCfg.abtTimeout.init(*icsTW, _wCfg.prm->maxBilling);
  _wCfg.maxTimeout.init(*icsTW, _wCfg.prm->maxBilling);

  ICSTcpServerIface * tcpSrv = (ICSTcpServerIface *)_icsHost->getInterface(ICSIdent::icsIdTCPServer);
  tcpSrv->registerProtocol(*this);
  return ICServiceAC::icsRcOk;
}

ICServiceAC::RCode ICSSmBilling::_icsStart(void)
{
  if (_roller.get())
    _roller->Start();
  return ICServiceAC::icsRcOk;
}

//Stops service
void  ICSSmBilling::_icsStop(bool do_wait/* = false*/)
{
  bool waitMode = false;
  do {
    if (_roller.get())
      _roller->Stop(do_wait);

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
bool ICSSmBilling::setConnListener(const ConnectGuard & use_conn) /*throw()*/
{
  MutexGuard  grd(_sync);
  if (_icsState != ICServiceAC::icsStStarted) {
    _sync.notify();
    return false;
  }

  SmBillManager * pMgr = _sessReg.find(use_conn->getUId());
  if (pMgr) {
    smsc_log_warn(logger, "%s: session[%s] is already opened on Connect[%u]",
                  _logId,  pMgr->mgrId(), (unsigned)use_conn->getUId());
    _sync.notify();
    return false;
  }
  //create new connect manager
  pMgr = new SmBillManager(_wCfg, _iProtoDef, ++_lastSessId, logger);
  _sessReg.insert(use_conn->getUId(), pMgr);
  use_conn->setPckPool(_pckPool); //switch to own buffers pool
  pMgr->bind(&use_conn);
  pMgr->start(); //switches Connect to asynchronous mode
  _sync.notify();
  return true;
}

//Notifies that given connection is to be closed, no more socket events will be reported.
void ICSSmBilling::onDisconnect(const ConnectGuard & use_conn) /*throw()*/
{
  std::auto_ptr<SmBillManager> pMgr;
  {
    MutexGuard  grd(_sync);
    pMgr.reset(_sessReg.extract(use_conn->getUId()));
    if (!pMgr.get()) {
      _sync.notify();
      return;
    }
  }
  pMgr->bind(NULL);
  pMgr->stop(false);
  smsc_log_debug(logger, "%s: closing session[%s] on Connect[%u]",
                _logId,  pMgr->mgrId(), (unsigned)use_conn->getUId());
  _sync.notify();
  //pMgr.reset(); //SmBillManager is destroyed at this point
}

} //smbill
} //inman
} //smsc

