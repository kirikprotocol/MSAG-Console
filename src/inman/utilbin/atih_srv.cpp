static char const ident[] = "$Id$";
//#include <assert.h>

#include "atih_srv.hpp"

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ServiceATIH implementation:
 * ************************************************************************** */
ServiceATIH::ServiceATIH(const ServiceATIH_CFG * in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(*in_cfg), session(0), disp(0)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.ServiceATIH");

    smsc_log_debug(logger, "ServiceATIH: Creating ..");

    disp = TCAPDispatcher::getInstance();
    _cfg.hlr.userId += 39; //adjust USER_ID to PortSS7 units id
    if (!disp->connect(_cfg.hlr.userId))
        smsc_log_error(logger, "ATIHSrv: EINSS7 stack unavailable!!!");
    else {
        smsc_log_debug(logger, "ATIHSrv: TCAP dispatcher has connected to SS7 stack");
        if (!(session = disp->openSSN(_cfg.hlr.scf_ssn, 1000))) {
            smsc_log_error(logger, "ATIHSrv: SSN[%u] unavailable!!!", _cfg.hlr.scf_ssn);
        } else {
            if (!(mapSess = session->newMAsession(_cfg.hlr.scf_addr,
                    ACOID::id_ac_map_anyTimeInfoHandling_v3, _cfg.hlr.hlr_ssn))) {
                smsc_log_error(logger, "ATIHSrv: Unable to init MAP session: %s -> %u:*",
                               _cfg.hlr.scf_addr, _cfg.hlr.hlr_ssn);
            } else
                smsc_log_debug(logger, "ATIHSrv: TCMA[%u:%u] session inited",
                               _cfg.hlr.scf_ssn, mapSess->getUID());
        }
    }
}

ServiceATIH::~ServiceATIH()
{
    smsc_log_debug(logger, "ATIHSrv: Releasing ..");
    {
        MutexGuard  grd(_sync);
        if (workers.size()) {
            smsc_log_error(logger, "ATIHSrv: there %u interrogators active, killing ..", 
                           workers.size());
            for (IntrgtrMAP::iterator it = workers.begin(); it != workers.end(); it++) {
                ATIInterrogator * worker = (*it).second;
                worker->cancel();
                pool.push_back(worker);
            }
            workers.clear();
        }
    }
    if (running)
      stop();

    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "ATIHSrv: Disconnecting SS7 stack ..");
    disp->disconnect();
    smsc_log_debug( logger, "ATIHSrv: Released." );

    //release workers
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        ATIInterrogator * worker = (*it);
        delete worker;
    }
    pool.clear();

}

bool ServiceATIH::start()
{
    MutexGuard  grd(_sync);
    running = true;
    smsc_log_debug(logger, "ATIHSrv: Started.");
    return running;
}

void ServiceATIH::stop()
{
    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "ATIHSrv: Stopping TCAP dispatcher ..");
    disp->Stop();
    running = false;
    smsc_log_debug(logger, "ATIHSrv: Stopped.");
}


bool ServiceATIH::requestCSI(const std::string &subcr_addr, bool imsi/* = true*/)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it == workers.end()) {
        if (mapSess && (mapSess->getState() == smsc::inman::inap::ssnBound)) {
            ATIInterrogator * worker = newWorker();
            if (worker->interrogate(subcr_addr, imsi)) {
                workers.insert(IntrgtrMAP::value_type(subcr_addr, worker));
                return true;
            }
            pool.push_back(worker);
        } else
            smsc_log_error(logger, "ATIHSrv: SSN is not bound!");
    } else
        smsc_log_error(logger, "ATIHSrv: CSI request already active!");
    return false;
}

/* ------------------------------------------------------------------------ *
 * ATCSIListener interface
 * ------------------------------------------------------------------------ */
void ServiceATIH::onCSIresult(const std::string & subcr_addr, const MAPSCFinfo* scfInfo)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        ATIInterrogator * worker = (*it).second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIresult(subcr_addr, scfInfo);
        pool.push_back(worker);
    }
}

void ServiceATIH::onCSIabort(const std::string &subcr_addr, unsigned short ercode,
                                                             InmanErrorType errLayer)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        ATIInterrogator * worker = (*it).second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIabort(subcr_addr, ercode, errLayer);
        pool.push_back(worker);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
ATIInterrogator * ServiceATIH::newWorker(void)
{
    ATIInterrogator * worker;
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        worker = (*it);
        if (!worker->isActive()) {
            pool.erase(it);
            return worker;
        }
    }
    worker = new ATIInterrogator(mapSess, this);
    return worker;
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

ATIInterrogator::~ATIInterrogator()
{
    MutexGuard  grd(_sync);
    if (mapDlg)
        delete mapDlg;
    _active = false;
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
    } catch (std::exception & exc) {
        smsc_log_error(logger, "Intrgtr[%s]: %s", subcr_addr.c_str() , exc.what());
        if (mapDlg) {
            delete mapDlg;
            mapDlg = NULL;
        }
        _active = false;
    }
    return _active;
}

void ATIInterrogator::cancel(void)
{
    MutexGuard  grd(_sync);
    if (mapDlg)
        mapDlg->endATSI();
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
void ATIInterrogator::onEndATSI(unsigned short ercode, InmanErrorType errLayer)
{
    MutexGuard  grd(_sync);
    if (mapDlg) {
        delete mapDlg;
        mapDlg = NULL;
    }
    if (errLayer == smsc::inman::errOk)
        csiHdl->onCSIresult(subcrAddr, &scfInfo);
    else
        csiHdl->onCSIabort(subcrAddr, ercode, errLayer);
}



} // namespace inman
} // namespace smsc


