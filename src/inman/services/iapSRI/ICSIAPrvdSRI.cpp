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

#include "inman/services/iapSRI/IAPQuerySRI.hpp"
#include "inman/services/iapSRI/ICSIAPrvdSRI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

/* ************************************************************************** *
 * class ICSIAPrvdSRI implementation:
 * ************************************************************************** */

ICServiceAC::RCode ICSIAPrvdSRI::_icsInit(void)
{ //registers _ac_map_locInfoRetrieval_v3

  TCAPDispatcherITF * disp = (TCAPDispatcherITF *)
                          _icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
  if (!disp) {
    smsc_log_error(logger, "%s: TCAPDispatcher is not inited!", _logId);
    return ICServiceAC::icsRcError;
  }
  if (!disp->acRegistry()->getFactory(_ac_map_locInfoRetrieval_v3)
      && !disp->acRegistry()->regFactory(initMAPCHSRI3Components)) {
    smsc_log_fatal(logger, "%s: ROS factory registration failed: %s!", _logId,
                    _ac_map_locInfoRetrieval_v3.nick());
    return ICServiceAC::icsRcError;
  }
  smsc_log_debug(logger, "%s: ROS factory registered: %s", _logId,
                  _ac_map_locInfoRetrieval_v3.nick());
  return ICServiceAC::icsRcOk;
}

//Starts service verifying that all dependent services are started
ICServiceAC::RCode ICSIAPrvdSRI::_icsStart(void)
{  //opens SSN, initializes TCSessionMA
  TCAPDispatcherITF * disp = (TCAPDispatcherITF *)
                          _icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
  if (disp->dspState() != TCAPDispatcherITF::dspRunning) {
      smsc_log_error(logger, "%s: TCAPDispatcher is not running!", _logId);
      return ICServiceAC::icsRcError;
  }
  SSNSession * session = disp->openSSN(_cfg.sriCfg.ownSsn, _cfg.sriCfg.maxDlgId, logger);
  if (!session) {
      smsc_log_error(logger, "%s: SSN[%u] is unavailable!", _logId, (unsigned)_cfg.sriCfg.ownSsn);
      return ICServiceAC::icsRcError;
  }
  if (!(_cfg.qryCfg.mapSess = session->newMAsession(_cfg.sriCfg.ownAddr.toString().c_str(),
      _ac_map_locInfoRetrieval_v3, 6, _cfg.sriCfg.fakeSsn))) {
      smsc_log_error(logger, "%s: Unable to init MAP session: %s -> %u:*", _logId,
                            _cfg.sriCfg.ownAddr.toString().c_str(), 6);
      return ICServiceAC::icsRcError;
  }
  smsc_log_info(logger, "%s: TCMA[%u:%u] inited", _logId,
                (unsigned)_cfg.sriCfg.ownSsn, _cfg.qryCfg.mapSess->getUID());
  _qryPlant.reset(new IAPQuerySRIFactory(_cfg.qryCfg, _cfg.qryCfg.mapTimeout, logger));
  _fcltCfg.qryPlant = _qryPlant.get();
  _prvd.reset(new IAPQueryFacility(_fcltCfg, logger));

  return _prvd->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
}

void ICSIAPrvdSRI::logConfig(Logger * use_log/* = NULL*/) const
{
  MutexGuard grd(_sync);
  if (!use_log)
    use_log = logger;

  if (_cfg.sriCfg.fakeSsn)
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u(fake=%u)", _logId,
                  _cfg.sriCfg.ownAddr.getSignals(), (unsigned)_cfg.sriCfg.ownSsn,
                  (unsigned)_cfg.sriCfg.fakeSsn);
  else
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u", _logId,
                    _cfg.sriCfg.ownAddr.getSignals(), (unsigned)_cfg.sriCfg.ownSsn);
  if (_cfg.qryCfg.mapSess)
    smsc_log_info(use_log, "%s: TCMA[%u:%u]", _logId,
                  (unsigned)_cfg.sriCfg.ownSsn, _cfg.qryCfg.mapSess->getUID());
  else
    smsc_log_info(use_log, "%s: TCMA uninitialized yet", _logId);

  smsc_log_info(use_log, "%s: Max.queries: %u", _logId, _fcltCfg.max_queries);
  smsc_log_info(use_log, "%s: Query timeout: %u secs", _logId, _cfg.qryCfg.mapTimeout);
}


} //sri
} //iaprvd
} //inman
} //smsc

