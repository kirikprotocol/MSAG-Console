#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::atih::initMAPATIH3Components;
using smsc::inman::comp::_ac_map_anyTimeInfoHandling_v3;

#include "inman/utilbin/atih_srv.hpp"
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;
using smsc::inman::inap::TCAPDispatcherITF;
using smsc::inman::inap::SSNBinding;

using smsc::inman::comp::UnifiedCSI;
using smsc::inman::iaprvd::CSIRecord;
using smsc::inman::iaprvd::IAPProperty;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ServiceATIH implementation:
 * ************************************************************************** */
ServiceATIH::ServiceATIH(const ServiceATIH_CFG & in_cfg, Logger * uselog/* = NULL*/)
  : logger(uselog), _logId("ATIHSrv"), mapSess(0), disp(new TCAPDispatcher())
  , running(false), _cfg(in_cfg)
{
  if (!logger)
    logger = Logger::getInstance("smsc.inman.ATIH");
  disp->Init(_cfg.mapCfg.ss7);
  _reqCfg.setCSI(UnifiedCSI::csi_O_BC);
}

ServiceATIH::~ServiceATIH()
{
  smsc_log_debug(logger, "%s: Releasing ..", _logId);
  {
    MutexGuard  grd(_sync);
    if (!workers.empty()) {
      smsc_log_error(logger, "%s: there %u interrogators active, killing ..",
                     _logId, workers.size());
      for (IntrgtrMAP::iterator it = workers.begin(); it != workers.end(); ++it) {
        ATIInterrogator * worker = it->second;
        worker->cancel();
        pool.push_back(worker);
      }
      workers.clear();
    }
  }
  stop(true);
  //release workers
  {
    MutexGuard  grd(_sync);
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); ++it) {
      ATIInterrogator * worker = (*it);
      delete worker;
    }
    pool.clear();
    delete disp;
  }
  smsc_log_debug(logger, "%s: Released.", _logId);
}

bool ServiceATIH::start(void)
{
  MutexGuard  grd(_sync);
  if (running)
    return true;
  if ((running = (disp->Start() && getSession()))) {
    smsc_log_debug(logger, "%s: Started.", _logId);
  }
  return running;
}

void ServiceATIH::stop(bool do_wait/* = false*/)
{
  MutexGuard  grd(_sync);
  if (running) {
    smsc_log_debug(logger, "%s: Stopping TCAP dispatcher ..", _logId);
    disp->Stop(do_wait);
    running = false;
  }
  if (do_wait)
    disp->Stop(true);
  smsc_log_debug(logger, "%s: Stopped.", _logId);
}

//Must be called with _sync locked
bool ServiceATIH::getSession(void)
{
  if (!mapSess) { //openSSN, initialize TCSessionMA
    if (disp->ss7State() != TCAPDispatcherITF::ss7CONNECTED) {
      smsc_log_error(logger, "%s: TCAPDispatcher is not connected!", _logId);
      return false;
    }
    if (!disp->acRegistry()->getFactory(_ac_map_anyTimeInfoHandling_v3)
        && !disp->acRegistry()->regFactory(initMAPATIH3Components)) {
      smsc_log_fatal(logger, "%s: ROS factory registration failed: %s!", _logId,
                     _ac_map_anyTimeInfoHandling_v3.nick());
      return false;
    }
    SSNSession * session = disp->openSSN(_cfg.mapCfg.usr.ownSsn, _cfg.mapCfg.usr.maxDlgId);
    if (!session) {
      smsc_log_error(logger, "%s: SSN[%u] is unavailable!", _logId, 
                     (unsigned)_cfg.mapCfg.usr.ownSsn);
      return false;
    }
    if (!(mapSess = session->newMAsession(_cfg.mapCfg.usr.ownAddr.toString().c_str(),
                                          _ac_map_anyTimeInfoHandling_v3, 6, _cfg.mapCfg.usr.fakeSsn))) {
      smsc_log_error(logger, "%s: Unable to init MAP session: %s -> %u:*", _logId,
                     _cfg.mapCfg.usr.ownAddr.toString().c_str(), 6);
      return false;
    }
    smsc_log_info(logger, "%s: TCMA[%u:%u] inited", _logId,
                  (unsigned)_cfg.mapCfg.usr.ownSsn, mapSess->getUID());
  }
  return(mapSess != 0);
}


bool ServiceATIH::requestCSI(const TonNpiAddress & subcr_addr)
{
  MutexGuard  grd(_sync);
  IntrgtrMAP::iterator it = workers.find(subcr_addr);
  if (it == workers.end()) {
    if (mapSess && (mapSess->bindStatus() >= SSNBinding::ssnPartiallyBound)) {
      ATIInterrogator * worker = newWorker();
      if (worker->interrogate(_reqCfg, subcr_addr)) {
        workers.insert(IntrgtrMAP::value_type(subcr_addr, worker));
        return true;
      }
      pool.push_back(worker);
    } else
      smsc_log_error(logger, "%s: SSN is not bound!", _logId);
  } else
    smsc_log_error(logger, "%s: CSI request already active!", _logId);
  return false;
}

/* ------------------------------------------------------------------------ *
 * AT_CSIListenerIface interface
 * ------------------------------------------------------------------------ */
void ServiceATIH::onCSIresult(const AbonentInfo & ab_info)
{
  MutexGuard  grd(_sync);
  IntrgtrMAP::iterator it = workers.find(ab_info.msIsdn);
  if (it != workers.end()) {
    ATIInterrogator * worker = it->second;
    workers.erase(it);
    if (_cfg.client)
      _cfg.client->onCSIresult(ab_info);
    pool.push_back(worker);
  }
}

void ServiceATIH::onCSIabort(const TonNpiAddress & subcr_addr, RCHash ercode)
{
  MutexGuard  grd(_sync);
  IntrgtrMAP::iterator it = workers.find(subcr_addr);
  if (it != workers.end()) {
    ATIInterrogator * worker = it->second;
    workers.erase(it);
    if (_cfg.client)
      _cfg.client->onCSIabort(subcr_addr, ercode);
    pool.push_back(worker);
  }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
ATIInterrogator * ServiceATIH::newWorker(void)
{
  for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); ++it) {
    ATIInterrogator * worker = (*it);
    if (!worker->isActive()) {
      pool.erase(it);
      return worker;
    }
  }
  return new ATIInterrogator(mapSess, this);
}

/* ************************************************************************** *
 * class ATIInterrogator implementation:
 * ************************************************************************** */
ATIInterrogator::ATIInterrogator(TCSessionMA * pSession, AT_CSIListenerIface * csi_listener,
                                  Logger * uselog/* = NULL*/)
  : _active(false), tcSesssion(pSession), mapDlg(NULL), csiHdl(csi_listener)
  ,  logger(uselog)
{ 
  if (!logger)
    logger = Logger::getInstance("smsc.inman.inap.atsi");
}

void ATIInterrogator::rlseMapDialog(void)
{
  if (mapDlg) {
    while (!mapDlg->Unbind()) //MAPDlg refers this query
      _sync.wait();
    mapDlg->destroy();
    mapDlg = NULL;
  }
  _active = false;
}

ATIInterrogator::~ATIInterrogator()
{
  MutexGuard  grd(_sync);
  rlseMapDialog();
}

bool ATIInterrogator::isActive(void)
{
  MutexGuard  grd(_sync);
  return _active;
}

//sets subscriber identity: IMSI or MSISDN addr
bool ATIInterrogator::interrogate(const RequestedSubscription & req_cfg,
                                  const TonNpiAddress & subcr_adr)
{
  MutexGuard  grd(_sync);
  _abnInfo.clear();
  _abnInfo.msIsdn = subcr_adr;

  try {
    mapDlg = new MapATSIDlg(tcSesssion, this);
    smsc_log_debug(logger, "Intrgtr[%s]: requesting subscription ..",
                   _abnInfo.msIsdn.toString().c_str());
    _reqCfg = req_cfg;
    mapDlg->subsciptionInterrogation(_reqCfg, _abnInfo.msIsdn);
    _active = true;
  } catch (const std::exception & exc) {
    smsc_log_error(logger, "Intrgtr[%s]: %s",
                   _abnInfo.msIsdn.toString().c_str(), exc.what());
    rlseMapDialog();
  }
  return _active;
}

void ATIInterrogator::cancel(void)
{
  MutexGuard  grd(_sync);
  if (mapDlg) {
    while (!mapDlg->Unbind()) //MAPDlg refers this query
      _sync.wait();
    mapDlg->endMapDlg();
  }
  _active = false;
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */

/* ATSIhandler interface -------------------------------------------------- */
void ATIInterrogator::onATSIResult(ATSIRes & res)
{
  MutexGuard  grd(_sync);

  if (res.isCSIpresent(_reqCfg.getCSI())) {
    _abnInfo.abType = AbonentContractInfo::abtPrepaid;
    CSIRecord & csiRec = _abnInfo.csiSCF[_reqCfg.getCSI()];
    csiRec.iapId = IAPProperty::iapATSI;
    csiRec.csiId = _reqCfg.getCSI();
    csiRec.scfInfo = *res.getSCFinfo(_reqCfg.getCSI());
  } else {
    _abnInfo.abType = AbonentContractInfo::abtPostpaid;
  }
  smsc_log_debug(logger, "Intrgtr[%s]: %s", _abnInfo.msIsdn.toString().c_str(),
                 _abnInfo.toString().c_str());
}
 //dialog finalization/error handling:
void ATIInterrogator::onEndATSI(RCHash ercode/* =0*/)
{
  MutexGuard  grd(_sync);
  rlseMapDialog();
  if (!ercode)
    csiHdl->onCSIresult(_abnInfo);
  else
    csiHdl->onCSIabort(_abnInfo.msIsdn, ercode);
}

} // namespace inman
} // namespace smsc


