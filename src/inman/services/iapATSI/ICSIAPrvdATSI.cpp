#ifdef MOD_IDENT_ON
static const char ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/inap/TCDspIface.hpp"
using smsc::inman::inap::TCAPDispatcherITF;

#include "inman/inap/HDSSnSession.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::_ac_map_anyTimeInfoHandling_v3;
using smsc::inman::comp::atih::initMAPATIH3Components;

#include "inman/services/iapATSI/IAPQueryATSI.hpp"
#include "inman/services/iapATSI/ICSIAPrvdATSI.hpp"

namespace smsc {
namespace inman {
namespace iaprvd {
namespace atih {

/* ************************************************************************** *
 * class ICSIAPrvdATSI implementation:
 * ************************************************************************** */

ICServiceAC::RCode ICSIAPrvdATSI::_icsInit(void)
{ //registers _ac_map_locInfoRetrieval_v3

  TCAPDispatcherITF * disp = (TCAPDispatcherITF *)
                          _icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
  if (!disp) {
    smsc_log_error(logger, "%s: TCAPDispatcher is not inited!", _logId);
    return ICServiceAC::icsRcError;
  }
  if (!disp->acRegistry()->getFactory(_ac_map_anyTimeInfoHandling_v3)
      && !disp->acRegistry()->regFactory(initMAPATIH3Components)) {
    smsc_log_fatal(logger, "%s: ROS factory registration failed: %s!", _logId,
                    _ac_map_anyTimeInfoHandling_v3.nick());
    return ICServiceAC::icsRcError;
  }
  smsc_log_debug(logger, "%s: ROS factory registered: %s", _logId,
                  _ac_map_anyTimeInfoHandling_v3.nick());
  return ICServiceAC::icsRcOk;
}

//Starts service verifying that all dependent services are started
ICServiceAC::RCode ICSIAPrvdATSI::_icsStart(void)
{  //opens SSN, initializes TCSessionMA
  TCAPDispatcherITF * disp = (TCAPDispatcherITF *)
                          _icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
  if (disp->dspState() != TCAPDispatcherITF::dspRunning) {
      smsc_log_error(logger, "%s: TCAPDispatcher is not running!", _logId);
      return ICServiceAC::icsRcError;
  }
  SSNSession * session = disp->openSSN(_cfg.atsiCfg.ownSsn, _cfg.atsiCfg.maxDlgId, logger);
  if (!session) {
      smsc_log_error(logger, "%s: SSN[%u] is unavailable!", _logId, (unsigned)_cfg.atsiCfg.ownSsn);
      return ICServiceAC::icsRcError;
  }
  if (!(_cfg.qryCfg.mapSess = session->newMAsession(_cfg.atsiCfg.ownAddr.toString().c_str(),
      _ac_map_anyTimeInfoHandling_v3, 6, _cfg.atsiCfg.fakeSsn))) {
      smsc_log_error(logger, "%s: Unable to init MAP session: %s -> %u:*", _logId,
                            _cfg.atsiCfg.ownAddr.toString().c_str(), 6);
      return ICServiceAC::icsRcError;
  }
  smsc_log_info(logger, "%s: TCMA[%u:%u] inited", _logId,
                (unsigned)_cfg.atsiCfg.ownSsn, _cfg.qryCfg.mapSess->getUID());
  _qryPlant.reset(new IAPQueryATSIFactory(_cfg.qryCfg, _cfg.qryCfg.mapTimeout, logger));
  _fcltCfg.qryPlant = _qryPlant.get();
  _prvd.reset(new IAPQueryFacility(_fcltCfg, logger));

  return _prvd->Start() ? ICServiceAC::icsRcOk : ICServiceAC::icsRcError;
}

void ICSIAPrvdATSI::logConfig(Logger * use_log/* = NULL*/) const
{
  MutexGuard grd(_sync);
  if (!use_log)
    use_log = logger;

  if (_cfg.atsiCfg.fakeSsn)
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u(fake=%u)", _logId,
                  _cfg.atsiCfg.ownAddr.getSignals(), (unsigned)_cfg.atsiCfg.ownSsn,
                  (unsigned)_cfg.atsiCfg.fakeSsn);
  else
    smsc_log_info(use_log, "%s: GT=%s, SSN=%u", _logId,
                    _cfg.atsiCfg.ownAddr.getSignals(), (unsigned)_cfg.atsiCfg.ownSsn);
  if (_cfg.qryCfg.mapSess)
    smsc_log_info(use_log, "%s: TCMA[%u:%u]", _logId,
                  (unsigned)_cfg.atsiCfg.ownSsn, _cfg.qryCfg.mapSess->getUID());
  else
    smsc_log_info(use_log, "%s: TCMA uninitialized yet", _logId);

  smsc_log_info(use_log, "%s: Max.queries: %u", _logId, _fcltCfg.maxQueries);
  smsc_log_info(use_log, "%s: Query timeout: %u secs", _logId, _cfg.qryCfg.mapTimeout);
}


} //atih
} //iaprvd
} //inman
} //smsc

