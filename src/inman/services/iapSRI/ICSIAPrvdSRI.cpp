#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/TCDspIface.hpp"
using smsc::inman::inap::TCAPDispatcherITF;

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::_ac_map_locInfoRetrieval_v3;
using smsc::inman::comp::chsri::initMAPCHSRI3Components;

#include "inman/services/iapSRI/ICSIAPrvdSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

/* ************************************************************************** *
 * class ICSIAPrvdSRI implementation:
 * ************************************************************************** */
const IAPProperty
  ICSIAPrvdSRI::_iapProperty(IAPProperty::iapCHSRI, "iapCHSRI",
                             IAPAbility::abContract | IAPAbility::abSCF
                             | IAPAbility::abIMSI | IAPAbility::abVLR);

// --------------------------------------
// -- ICServiceAC interface methods
// --------------------------------------
ICServiceAC::RCode ICSIAPrvdSRI::_icsInit(void)
{
  //register _ac_map_locInfoRetrieval_v3
  TCAPDispatcherITF * 
    tcDisp = (TCAPDispatcherITF *)_icsHost->getInterface(ICSIdent::icsIdTCAPDisp);

  if (!tcDisp->acRegistry()->getFactory(_ac_map_locInfoRetrieval_v3)
      && !tcDisp->acRegistry()->regFactory(initMAPCHSRI3Components)) {
    smsc_log_fatal(logger, "%s: ROS factory registration failed: %s!", _logId,
                    _ac_map_locInfoRetrieval_v3.nick().c_str());
    return ICServiceAC::icsRcError;
  }
  smsc_log_info(logger, "%s: ROS factory registered: %s", _logId,
                _ac_map_locInfoRetrieval_v3.nick().c_str());

  //NOTE: _qrsPool initialization requires active SSN session
  _qrsFclt.init(_qrsPool, _cfg._maxThreads);
  /* */
  return ICServiceAC::icsRcOk;
}

//Starts service verifying that all dependent services are started
ICServiceAC::RCode ICSIAPrvdSRI::_icsStart(void)
{  //opens SSN, initializes TCSessionMA
  TCAPDispatcherITF * tcDisp = (TCAPDispatcherITF *)
                          _icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
  if (tcDisp->dspState() != TCAPDispatcherITF::dspRunning) {
    smsc_log_error(logger, "%s: TCAPDispatcher is not running!", _logId);
    return ICServiceAC::icsRcError;
  }
  SSNSession * session = tcDisp->openSSN(_cfg._sriCfg.ownSsn, _cfg._sriCfg.maxDlgId, logger);
  if (!session) {
    smsc_log_error(logger, "%s: SSN[%u] is unavailable!", _logId, (unsigned)_cfg._sriCfg.ownSsn);
    return ICServiceAC::icsRcError;
  }
  if (!(_cfg._mapSess = session->newMAsession(_cfg._sriCfg.ownAddr.toString().c_str(),
                                              _ac_map_locInfoRetrieval_v3, 6, _cfg._sriCfg.fakeSsn))) {
    smsc_log_error(logger, "%s: Unable to init MAP session: %s -> %u:*", _logId,
                   _cfg._sriCfg.ownAddr.toString().c_str(), 6);
    return ICServiceAC::icsRcError;
  }
  smsc_log_info(logger, "%s: TCMA[%u:%u] inited", _logId,
                (unsigned)_cfg._sriCfg.ownSsn, _cfg._mapSess->getUID());

  _qrsPool.init(IAPQuerySRI_CFG(_cfg._sriCfg.rosTimeout, *_cfg._mapSess), logger);
  _qrsFclt.start();
  /* */
  return ICServiceAC::icsRcOk;
}

//Stops service
void  ICSIAPrvdSRI::_icsStop(bool do_wait/* = false*/)
{
  _qrsFclt.stop(do_wait);
}

// ----------------------------------
// -- IAProviderAC interface methods
// ----------------------------------
void ICSIAPrvdSRI::logConfig(Logger * use_log/* = NULL*/) const
{
  MutexGuard grd(_sync);
  if (!use_log)
    use_log = logger;

  if (_cfg._sriCfg.fakeSsn) {
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u(fake=%u)", _logId,
                  _cfg._sriCfg.ownAddr.getSignals(), (unsigned)_cfg._sriCfg.ownSsn,
                  (unsigned)_cfg._sriCfg.fakeSsn);
  } else {
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u", _logId,
                  _cfg._sriCfg.ownAddr.getSignals(), (unsigned)_cfg._sriCfg.ownSsn);
  }
  if (_cfg._mapSess) {
    smsc_log_info(use_log, "%s: TCMA[%u:%u]", _logId,
                  (unsigned)_cfg._sriCfg.ownSsn, (unsigned)_cfg._mapSess->getUID());
  } else {
    smsc_log_info(use_log, "%s: TCMA uninitialized yet", _logId);
  }

  smsc_log_info(use_log, "%s: Max.queries: %u", _logId, (unsigned)_cfg._sriCfg.maxDlgId);
  smsc_log_info(use_log, "%s: Query timeout: %u secs", _logId, (unsigned)_cfg._sriCfg.rosTimeout);
  if (_cfg._maxThreads) {
    smsc_log_info(use_log, "%s: Max.threads: %u", _logId, (unsigned)_cfg._maxThreads);
  } else {
    smsc_log_info(use_log, "%s: Max.threads: unlimited", _logId);
  }
}

// -------------------------------------------------------
// -- IAPQueryProcessorITF interface methods
// -------------------------------------------------------
//Starts query and binds listener to it.
//Returns true if query succesfully started, false otherwise
bool ICSIAPrvdSRI::startQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  MutexGuard grd(_sync);
  return (icsState() != ICServiceAC::icsStStarted) 
          ? false : _qrsFclt.startQuery(ab_number, pf_cb);
}
//Unbinds query listener, cancels query if no listeners remain.
//Returns false if listener is already targeted and query waits for its mutex.
bool ICSIAPrvdSRI::cancelQuery(const AbonentId & ab_number, IAPQueryListenerITF & pf_cb)
{
  return _qrsFclt.cancelQuery(ab_number, pf_cb);
}
//Attempts to cancel all queries.
//Returns false if at least one listener is already targeted and query waits for its mutex.
bool ICSIAPrvdSRI::cancelAllQueries(void)
{
  return _qrsFclt.cancelAllQueries();
}


} //sri
} //iaprvd
} //inman
} //smsc

