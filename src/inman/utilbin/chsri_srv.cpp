static char const ident[] = "$Id$";

#include "chsri_srv.hpp"

namespace smsc  {
namespace inman {

/* ************************************************************************** *
 * class ServiceCHSRI implementation:
 * ************************************************************************** */
ServiceCHSRI::ServiceCHSRI(const ServiceCHSRI_CFG * in_cfg, Logger * uselog/* = NULL*/)
    : logger(uselog), _cfg(*in_cfg), session(0), disp(0)
{
    if (!logger)
        logger = Logger::getInstance("smsc.inman.ServiceCHSRI");

    smsc_log_debug(logger, "ServiceCHSRI: Creating ..");

    disp = TCAPDispatcher::getInstance();
    _cfg.hlr.userId += 39; //adjust USER_ID to PortSS7 units id
    if (!disp->connect(_cfg.hlr.userId))
        smsc_log_error(logger, "SRISrv: EINSS7 stack unavailable!!!");
    else {
        smsc_log_debug(logger, "SRISrv: TCAP dispatcher has connected to SS7 stack");
        if (!(session = disp->openSSN(_cfg.hlr.scf_ssn, 1000))) {
            smsc_log_error(logger, "SRISrv: SSN[%u] unavailable!!!", _cfg.hlr.scf_ssn);
        } else {
            if (!(mapSess = session->newMAsession(_cfg.hlr.scf_addr,
                    ACOID::id_ac_map_locInfoRetrieval_v3, _cfg.hlr.hlr_ssn))) {
                smsc_log_error(logger, "SRISrv: Unable to init MAP session: %s -> %u:*",
                               _cfg.hlr.scf_addr, _cfg.hlr.hlr_ssn);
            } else
                smsc_log_debug(logger, "SRISrv: TCMA[%u:%u] session inited",
                               _cfg.hlr.scf_ssn, mapSess->getUID());
        }
    }
}

ServiceCHSRI::~ServiceCHSRI()
{
    smsc_log_debug(logger, "SRISrv: Releasing ..");
    {
        MutexGuard  grd(_sync);
        if (workers.size()) {
            smsc_log_error(logger, "SRISrv: there %u interrogators active, killing ..", 
                           workers.size());
            for (IntrgtrMAP::iterator it = workers.begin(); it != workers.end(); it++) {
                SRIInterrogator * worker = (*it).second;
                worker->cancel();
                pool.push_back(worker);
            }
            workers.clear();
        }
    }
    if (running)
      stop();

    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "SRISrv: Disconnecting SS7 stack ..");
    disp->disconnect();
    smsc_log_debug( logger, "SRISrv: Released." );

    //release workers
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        SRIInterrogator * worker = (*it);
        delete worker;
    }
    pool.clear();

}

bool ServiceCHSRI::start()
{
    MutexGuard  grd(_sync);
    running = true;
    smsc_log_debug(logger, "SRISrv: Started.");
    return running;
}

void ServiceCHSRI::stop()
{
    MutexGuard  grd(_sync);
    smsc_log_debug(logger, "SRISrv: Stopping TCAP dispatcher ..");
    disp->Stop();
    running = false;
    smsc_log_debug(logger, "SRISrv: Stopped.");
}


bool ServiceCHSRI::requestCSI(const std::string &subcr_addr)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it == workers.end()) {
        if (mapSess && (mapSess->getState() == smsc::inman::inap::ssnBound)) {
            SRIInterrogator * worker = newWorker();
            if (worker->interrogate(subcr_addr)) {
                workers.insert(IntrgtrMAP::value_type(subcr_addr, worker));
                return true;
            }
            pool.push_back(worker);
        } else
            smsc_log_error(logger, "SRISrv: SSN is not bound!");
    } else
        smsc_log_error(logger, "SRISrv: CSI request already active!");
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
        SRIInterrogator * worker = (*it).second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIresult(subcr_addr, subcr_imsi, scfInfo);
        pool.push_back(worker);
    }
}

void ServiceCHSRI::onCSIabort(const std::string &subcr_addr, unsigned short ercode,
                                                             InmanErrorType errLayer)
{
    MutexGuard  grd(_sync);
    IntrgtrMAP::iterator it = workers.find(subcr_addr);
    if (it != workers.end()) {
        SRIInterrogator * worker = (*it).second;
        workers.erase(it);
        if (_cfg.client)
            _cfg.client->onCSIabort(subcr_addr, ercode, errLayer);
        pool.push_back(worker);
    }
}

/* ------------------------------------------------------------------------ *
 * Private/protected methods
 * ------------------------------------------------------------------------ */
SRIInterrogator * ServiceCHSRI::newWorker(void)
{
    SRIInterrogator * worker;
    for (IntrgtrLIST::iterator it = pool.begin(); it != pool.end(); it++) {
        worker = (*it);
        if (!worker->isActive()) {
            pool.erase(it);
            return worker;
        }
    }
    worker = new SRIInterrogator(mapSess, this);
    return worker;
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

SRIInterrogator::~SRIInterrogator()
{
    MutexGuard  grd(_sync);
    if (sriDlg)
        delete sriDlg;
    _active = false;
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
    subcrImsi[0] = 0;
    scfInfo.serviceKey = 0;
    scfInfo.scfAddress.clear();
    try {
        sriDlg = new MapCHSRIDlg(tcSesssion, this);
        subcrAddr = subcr_addr;
        smsc_log_debug(logger, "Intrgtr[%s]: requesting subscription ..", subcr_addr.c_str());
        sriDlg->reqRoutingInfo(subcr_addr.c_str());
        _active = true;
    } catch (std::exception & exc) {
        smsc_log_error(logger, "Intrgtr[%s]: %s", subcr_addr.c_str() , exc.what());
        if (sriDlg) {
            delete sriDlg;
            sriDlg = NULL;
        }
        _active = false;
    }
    return _active;
}

void SRIInterrogator::cancel(void)
{
    MutexGuard  grd(_sync);
    if (sriDlg)
        sriDlg->endMapDlg();
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
void SRIInterrogator::onEndMapDlg(unsigned short ercode, InmanErrorType errLayer)
{
    MutexGuard  grd(_sync);
    if (sriDlg) {
        delete sriDlg;
        sriDlg = NULL;
    }
    if (errLayer == smsc::inman::errOk) {
        if (subcrImsi[0])
            csiHdl->onCSIresult(subcrAddr, subcrImsi,
                                scfInfo.scfAddress.length ? &scfInfo : NULL);
        else
            csiHdl->onCSIabort(subcrAddr, MapCHSRIDlg::chsriServiceResponse,
                               smsc::inman::errMAPuser);
    } else
        csiHdl->onCSIabort(subcrAddr, ercode, errLayer);
}



} // namespace inman
} // namespace smsc


