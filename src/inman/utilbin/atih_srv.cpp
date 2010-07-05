#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/comp/map_atih/MapATIHFactory.hpp"
using smsc::inman::comp::atih::initMAPATIH3Components;
using smsc::inman::comp::_ac_map_anyTimeInfoHandling_v3;

#include "inman/utilbin/atih_srv.hpp"
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;
using smsc::inman::inap::TCAPDispatcherITF;
using smsc::inman::inap::SSNBinding;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ServiceATIH implementation:
 * ************************************************************************** */
ServiceATIH::ServiceATIH(const ServiceATIH_CFG & in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(in_cfg), mapSess(0), disp(new TCAPDispatcher())
    , _logId("ATIHSrv"), running(false)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.ATIH");
    disp->Init(_cfg.mapCfg.ss7);
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
    if ((running = (disp->Start() && getSession())))
        smsc_log_debug(logger, "%s: Started.", _logId);
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
    return (mapSess != 0);
}


bool ServiceATIH::requestCSI(const std::string &subcr_addr, bool imsi/* = true*/)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it == workers.end()) {
        if (mapSess && (mapSess->bindStatus() >= SSNBinding::ssnPartiallyBound)) {
            ATIInterrogator * worker = newWorker();
            if (worker->interrogate(subcr_addr, imsi)) {
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
 * ATCSIListener interface
 * ------------------------------------------------------------------------ */
void ServiceATIH::onCSIresult(const std::string & subcr_addr, const GsmSCFinfo* scfInfo)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        ATIInterrogator * worker = it->second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIresult(subcr_addr, scfInfo);
        pool.push_back(worker);
    }
}

void ServiceATIH::onCSIabort(const std::string &subcr_addr, RCHash ercode)
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
ATIInterrogator::ATIInterrogator(TCSessionMA* pSession, ATCSIListener * csi_listener,
                                  Logger * uselog/* = NULL*/)
    : tcSesssion(pSession), csiHdl(csi_listener), mapDlg(NULL)
    , _active(false), logger(uselog)
{ 
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.atsi");
}

void ATIInterrogator::rlseMapDialog(void)
{
    if (mapDlg) {
        while (!mapDlg->Unbind()) //MAPDlg refers this query
            _sync.wait();
        delete mapDlg;
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
bool ATIInterrogator::interrogate(const std::string &subcr_addr, bool imsi/* = false*/)
{
    MutexGuard  grd(_sync);
    scfInfo.serviceKey = 0;
    scfInfo.scfAddress.clear();
    try {
        mapDlg = new MapATSIDlg(tcSesssion, this);
        smsc_log_debug(logger, "Intrgtr[%s]: requesting subscription ..", subcr_addr.c_str());
        mapDlg->subsciptionInterrogation(subcr_addr.c_str(), imsi);
        _active = true;
        subcrAddr = subcr_addr;
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "Intrgtr[%s]: %s", subcr_addr.c_str() , exc.what());
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
void ATIInterrogator::onATSIResult(ATSIRes* arg)
{
    MutexGuard  grd(_sync);
    if (!arg->getSCFinfo(RequestedCAMEL_SubscriptionInfo_o_CSI, &scfInfo))
        scfInfo.scfAddress.clear();
}
 //dialog finalization/error handling:
void ATIInterrogator::onEndATSI(RCHash ercode/* =0*/)
{
    MutexGuard  grd(_sync);
    rlseMapDialog();
    if (!ercode)
        csiHdl->onCSIresult(subcrAddr, &scfInfo);
    else
        csiHdl->onCSIabort(subcrAddr, ercode);
}



} // namespace inman
} // namespace smsc


