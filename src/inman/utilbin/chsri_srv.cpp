#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::chsri::initMAPCHSRI3Components;
using smsc::inman::comp::_ac_map_locInfoRetrieval_v3;

#include "inman/utilbin/chsri_srv.hpp"
using smsc::inman::comp::MAPServiceRC;
using smsc::inman::comp::_RCS_MAPService;
using smsc::inman::inap::TCAPDispatcherITF;
using smsc::inman::inap::SSNBinding;

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ServiceCHSRI implementation:
 * ************************************************************************** */
ServiceCHSRI::ServiceCHSRI(const ServiceCHSRI_CFG & in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(in_cfg), mapSess(0), disp(new TCAPDispatcher())
    , _logId("SRISrv"), running(false)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.CHSRI");

    disp->Init(_cfg.mapCfg.ss7);
}

ServiceCHSRI::~ServiceCHSRI()
{
    smsc_log_debug(logger, "%s: Releasing ..", _logId);
    {
        MutexGuard  grd(_sync);
        if (!workers.empty()) {
            smsc_log_error(logger, "%s: there %u interrogators active, killing ..",
                           _logId, workers.size());
            for (IntrgtrMAP::iterator it = workers.begin(); it != workers.end(); ++it) {
                SRIInterrogator * worker = it->second;
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
            SRIInterrogator * worker = (*it);
            delete worker;
        }
        pool.clear();
        delete disp;
    }
    smsc_log_debug( logger, "SRISrv: Released." );
}

bool ServiceCHSRI::start()
{
    MutexGuard  grd(_sync);
    if (running)
        return true;
    if ((running = (disp->Start() && getSession())))
        smsc_log_debug(logger, "%s: Started.", _logId);
    return running;
}

void ServiceCHSRI::stop(bool do_wait/* = false*/)
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
bool ServiceCHSRI::getSession(void)
{
    if (!mapSess) { //openSSN, initialize TCSessionMA
        if (disp->ss7State() != TCAPDispatcherITF::ss7CONNECTED) {
            smsc_log_error(logger, "%s: TCAPDispatcher is not connected!", _logId);
            return false;
        }
        if (!disp->acRegistry()->getFactory(_ac_map_locInfoRetrieval_v3)
            && !disp->acRegistry()->regFactory(initMAPCHSRI3Components)) {
            smsc_log_fatal(logger, "%s: ROS factory registration failed: %s!", _logId,
                            _ac_map_locInfoRetrieval_v3.nick());
            return false;
        }
        SSNSession * session = disp->openSSN(_cfg.mapCfg.usr.ownSsn, _cfg.mapCfg.usr.maxDlgId);
        if (!session) {
            smsc_log_error(logger, "%s: SSN[%u] is unavailable!", _logId, 
                           (unsigned)_cfg.mapCfg.usr.ownSsn);
            return false;
        }
        if (!(mapSess = session->newMAsession(_cfg.mapCfg.usr.ownAddr.toString().c_str(),
                                _ac_map_locInfoRetrieval_v3, 6, _cfg.mapCfg.usr.fakeSsn))) {
            smsc_log_error(logger, "%s: Unable to init MAP session: %s -> %u:*", _logId,
                                  _cfg.mapCfg.usr.ownAddr.toString().c_str(), 6);
            return false;
        }
        smsc_log_info(logger, "%s: TCMA[%u:%u] inited", _logId,
                      (unsigned)_cfg.mapCfg.usr.ownSsn, mapSess->getUID());
    }
    return (mapSess != 0);
}

bool ServiceCHSRI::requestCSI(const std::string &subcr_addr)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it == workers.end()) {
        if (mapSess && (mapSess->bindStatus() >= SSNBinding::ssnPartiallyBound)) {
            SRIInterrogator * worker = newWorker();
            if (worker->interrogate(subcr_addr)) {
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
 * SRI_CSIListener interface
 * ------------------------------------------------------------------------ */
void ServiceCHSRI::onCSIresult(const std::string & subcr_addr, 
                               const char * subcr_imsi, const GsmSCFinfo* scfInfo)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        SRIInterrogator * worker = it->second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIresult(subcr_addr, subcr_imsi, scfInfo);
        pool.push_back(worker);
    }
}

void ServiceCHSRI::onCSIabort(const std::string &subcr_addr, RCHash ercode)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        SRIInterrogator * worker = it->second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIabort(subcr_addr, ercode);
        pool.push_back(worker);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
SRIInterrogator * ServiceCHSRI::newWorker(void)
{
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); ++it) {
        SRIInterrogator * worker = (*it);
        if (!worker->isActive()) {
            pool.erase(it);
            return worker;
        }
    }
    return new SRIInterrogator(mapSess, this);
}

/* ************************************************************************** *
 * class SRIInterrogator implementation:
 * ************************************************************************** */
SRIInterrogator::SRIInterrogator(TCSessionMA* pSession, SRI_CSIListener * csi_listener,
                                  Logger * uselog/* = NULL*/)
    : tcSesssion(pSession), csiHdl(csi_listener), sriDlg(NULL)
    , _active(false), logger(uselog)
{ 
    if (!logger)
        logger = Logger::getInstance("smsc.inman.inap.atih.Intrgtr");
}

void SRIInterrogator::rlseSRIDialog(void)
{
    if (sriDlg) {
        while (!sriDlg->Unbind()) //MAPDlg refers this query
            _sync.wait();
        delete sriDlg;
        sriDlg = NULL;
    }
    _active = false;
}

SRIInterrogator::~SRIInterrogator()
{
    MutexGuard  grd(_sync);
    rlseSRIDialog();
}

bool SRIInterrogator::isActive(void)
{
    MutexGuard  grd(_sync);
    return _active;
}

//sets subscriber identity: MSISDN addr
bool SRIInterrogator::interrogate(const std::string &subcr_addr)
{
    MutexGuard  grd(_sync);
    subcrImsi.clear();
    scfInfo.serviceKey = 0;
    scfInfo.scfAddress.clear();
    try {
        sriDlg = new MapCHSRIDlg(tcSesssion, this);
        subcrAddr = subcr_addr;
        smsc_log_debug(logger, "Intrgtr[%s]: requesting subscription ..", subcr_addr.c_str());
        sriDlg->reqRoutingInfo(subcr_addr.c_str());
        _active = true;
    } catch (const std::exception & exc) {
        smsc_log_error(logger, "Intrgtr[%s]: %s", subcr_addr.c_str() , exc.what());
        rlseSRIDialog();
    }
    return _active;
}

void SRIInterrogator::cancel(void)
{
    MutexGuard  grd(_sync);
    if (sriDlg) {
        while (!sriDlg->Unbind()) //MAPDlg refers this query
            _sync.wait();
        sriDlg->endMapDlg();
    }
    _active = false;
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */

/* CHSRIhandler interface -------------------------------------------------- */
void SRIInterrogator::onMapResult(CHSendRoutingInfoRes* arg)
{
    MutexGuard  grd(_sync);
    if (!arg->getIMSI(subcrImsi))
        smsc_log_error(logger, "Intrgtr[%s]: IMSI not determined.", subcrAddr.c_str());
    else {
        if (!arg->getSCFinfo(&scfInfo))
            smsc_log_debug(logger, "Intrgtr[%s]: SCF not set.", subcrAddr.c_str());
        else
            smsc_log_debug(logger, "Intrgtr[%s]: SCF determined.", subcrAddr.c_str());
    }
}

 //dialog finalization/error handling:
void SRIInterrogator::onEndMapDlg(RCHash ercode/* =0*/)
{
    MutexGuard  grd(_sync);
    rlseSRIDialog();
    if (!ercode) {
        if (!subcrImsi.empty())
            csiHdl->onCSIresult(subcrAddr, subcrImsi.c_str(),
                                scfInfo.scfAddress.length ? &scfInfo : NULL);
        else
            csiHdl->onCSIabort(subcrAddr, 
                    _RCS_MAPService->mkhash(MAPServiceRC::noServiceResponse));
    } else
        csiHdl->onCSIabort(subcrAddr, ercode);
}



} // namespace inman
} // namespace smsc


