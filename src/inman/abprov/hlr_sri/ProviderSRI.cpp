#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/abprov/IAPLoader.hpp"
using smsc::inman::ICSIdent;

#include "inman/abprov/hlr_sri/ProviderSRI.hpp"
using smsc::inman::inap::TCAPDispatcherITF;

#include "inman/inap/TCXCfgParser.hpp"
using smsc::inman::inap::TCAPUsrCfgParser;

#include "inman/inap/session.hpp"
using smsc::inman::inap::SSNSession;

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::_ac_map_locInfoRetrieval_v3;
using smsc::inman::comp::chsri::initMAPCHSRI3Components;


using smsc::util::URCRegistry;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

//This is the HLR(CH-SRI) Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(XConfigView* hlrCfg, Logger * use_log) throw(ConfigException)
{
    const char * cstr = NULL;
    try { cstr = hlrCfg->getString("tcapUser");
    } catch (const ConfigException & exc) { }

    if (!cstr || !cstr[0])
        throw ConfigException("parameter 'tcapUser' isn't set!");

    Config & rootSec = hlrCfg->relConfig();
    if (!rootSec.findSection(cstr))
        throw ConfigException("section %s' is missing!", cstr);
    smsc_log_info(use_log, "Reading settings from '%s' ..", cstr);

    IAPCreatorSRI_CFG cfg;
    TCAPUsrCfgParser parser(use_log, cstr);
    parser.readConfig(rootSec, cfg.sriCfg); //throws
    cfg.qryCfg.mapTimeout = cfg.sriCfg.rosTimeout;
    /**/
    return new IAProviderCreatorSRI(cfg, use_log);
}

/* ************************************************************************** *
 * class IAProviderCreatorSRI implementation:
 * ************************************************************************** */
IAProviderCreatorSRI::IAProviderCreatorSRI(const IAPCreatorSRI_CFG & use_cfg,
                                         Logger * use_log/* = NULL*/)
    : cfg(use_cfg), icsHost(0)
{
    std::string ctgr(use_log ? use_log->getName() : "smsc.inman");
    ctgr += ".iap.sri";
    logger = Logger::getInstance(ctgr.c_str());
    prvdCfg.init_threads = use_cfg.init_threads;
    prvdCfg.max_queries = use_cfg.sriCfg.maxDlgId;
    prvdCfg.qryMultiRun = true; //MapCHSRI dialogs are reused !!!
    prvdCfg.qryPlant = NULL;    //will be inited later
    //
    icsDeps.insert(ICSIdent::icsIdTCAPDisp);
}

IAProviderCreatorSRI::~IAProviderCreatorSRI()
{
    if (prvdCfg.qryPlant)
        delete prvdCfg.qryPlant;
}

IAProviderITF * IAProviderCreatorSRI::startProvider(const ICServicesHostITF * use_host)
{
    MutexGuard grd(_sync);
    if (!prvdCfg.qryPlant) { //openSSN, initialize TCSessionMA
        icsHost = use_host;
        TCAPDispatcherITF * disp = (TCAPDispatcherITF *)
                                icsHost->getInterface(ICSIdent::icsIdTCAPDisp);
        if (!disp || (disp->ss7State() != TCAPDispatcherITF::ss7CONNECTED)) {
            smsc_log_error(logger, "iapSRI: TCAPDispatcher is not connected!");
            return NULL;
        }
        if (!disp->acRegistry()->getFactory(_ac_map_locInfoRetrieval_v3)
            && !disp->acRegistry()->regFactory(initMAPCHSRI3Components)) {
            smsc_log_fatal(logger, "iapSRI: ROS factory registration failed: %s!",
                            _ac_map_locInfoRetrieval_v3.nick());
            return NULL;
        }
        SSNSession * session = disp->openSSN(cfg.sriCfg.ownSsn, cfg.sriCfg.maxDlgId, 1, logger);
        if (!session) {
            smsc_log_error(logger, "iapSRI: SSN[%u] is unavailable!", (unsigned)cfg.sriCfg.ownSsn);
            return NULL;
        }
        if (!(cfg.qryCfg.mapSess = session->newMAsession(cfg.sriCfg.ownAddr.toString().c_str(),
            _ac_map_locInfoRetrieval_v3, 6, cfg.sriCfg.fakeSsn))) {
            smsc_log_error(logger, "iapSRI: Unable to init MAP session: %s -> %u:*",
                                  cfg.sriCfg.ownAddr.toString().c_str(), 6);
            return NULL;
        }
        smsc_log_info(logger, "iapSRI: TCMA[%u:%u] inited",
                      (unsigned)cfg.sriCfg.ownSsn, cfg.qryCfg.mapSess->getUID());
        prvdCfg.qryPlant = new IAPQuerySRIFactory(cfg.qryCfg, cfg.qryCfg.mapTimeout, logger);
        prvd.reset(new IAProviderThreaded(prvdCfg, logger));
        prvd->Start();
    }
    return prvd.get();
}

void IAProviderCreatorSRI::stopProvider(bool do_wait/* = false*/)
{
    MutexGuard grd(_sync);
    if (prvd.get())
        prvd->Stop(do_wait);
}

void IAProviderCreatorSRI::logConfig(Logger * use_log/* = NULL*/) const
{
    MutexGuard grd(_sync);
    if (!use_log)
        use_log = logger;

    if (cfg.sriCfg.fakeSsn)
        smsc_log_info(use_log, "iapSRI: GT=%s, SSN=%u(fake=%u)",
                      cfg.sriCfg.ownAddr.getSignals(), (unsigned)cfg.sriCfg.ownSsn,
                      (unsigned)cfg.sriCfg.fakeSsn);
    else
        smsc_log_info(use_log, "iapSRI: GT=%s, SSN=%u",
                        cfg.sriCfg.ownAddr.getSignals(), (unsigned)cfg.sriCfg.ownSsn);
    if (cfg.qryCfg.mapSess)
        smsc_log_info(use_log, "iapSRI: TCMA[%u:%u]",
                      (unsigned)cfg.sriCfg.ownSsn, cfg.qryCfg.mapSess->getUID());
    else
        smsc_log_info(use_log, "iapSRI: TCMA uninitialized yet");

    smsc_log_info(use_log, "iapSRI: Max.queries: %u", prvdCfg.max_queries);
    smsc_log_info(use_log, "iapSRI: Query timeout: %u secs", cfg.qryCfg.mapTimeout);
}

/* ************************************************************************** *
 * class IAPQuerySRIFactory implementation:
 * ************************************************************************** */
IAPQuerySRIFactory::IAPQuerySRIFactory(const IAPQuerySRI_CFG &in_cfg,
                            unsigned timeout_secs, Logger * use_log/* = NULL*/)
    : _cfg(in_cfg)
{
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.sri");
}

IAPQueryAC * IAPQuerySRIFactory::newQuery(unsigned q_id, IAPQueryManagerITF * owner,
                                         Logger * use_log)
{
    return new IAPQuerySRI(q_id, owner, use_log, _cfg);
}

/* ************************************************************************** *
 * class IAPQuerySRI implementation:
 * ************************************************************************** */
IAPQuerySRI::IAPQuerySRI(unsigned q_id, IAPQueryManagerITF * owner, 
                        Logger * use_log, const IAPQuerySRI_CFG & use_cfg)
    : IAPQueryAC(q_id, owner, use_cfg.mapTimeout, use_log), _cfg(use_cfg)
{
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iaprvd.sri");
    mkTaskName();
}

int IAPQuerySRI::Execute(void)
{ 
    {
        MutexGuard tmp(_mutex);
        if (isStopping || !_owner->hasListeners(abonent)) {
            //query was cancelled by either QueryManager or ThreadPool
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqCancelled);
            return _qStatus;
        }
        //MAP_SRI serves only ISDN International numbers
        if (!abonent.interISDN()) {
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqBadArg);
            return _qStatus;
        }

        try {
            sriDlg = new MapCHSRIDlg(_cfg.mapSess, this); //binds this as user
            sriDlg->reqRoutingInfo(abonent, _cfg.mapTimeout); //throws

            if (_mutex.wait(_cfg.mapTimeout*1000 + 100) != 0) //Unlocks, waits, locks
                _qStatus = IAPQStatus::iqTimeout;
        } catch (const std::exception & exc) {
            smsc_log_error(logger, "%s(%s): %s", taskName(),
                       abonent.getSignals(), exc.what());
            _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqError);
            _exc = exc.what();
        }
        if (sriDlg) {
            while (!sriDlg->Unbind()) //MAPDlg refers this query
                _mutex.wait();
        }
    }
    if (sriDlg) {
        delete sriDlg;  //synchronization point, waits for sriDlg mutex
        sriDlg = NULL;
    }
    return _qStatus;
}

void IAPQuerySRI::stop(void)
{
    MutexGuard  grd(_mutex);
    isStopping = true;
    _qError = _RCS_IAPQStatus->mkhash(_qStatus = IAPQStatus::iqCancelled);
    _mutex.notify();
}
// ****************************************
// -- CHSRIhandlerITF implementation:
// ****************************************
void IAPQuerySRI::onMapResult(CHSendRoutingInfoRes* arg)
{
    MutexGuard  grd(_mutex);
    if (!arg->getIMSI(abInfo.abRec.abImsi)) //abonent is unknown
        smsc_log_warn(logger, "%s(%s): IMSI not determined.", taskName(), abonent.getSignals());
    else {
        //NOTE: CH-SRI returns only O-Bcsm tDP serviceKeys
        if (!arg->hasOCSI()) {
            abInfo.abRec.ab_type = AbonentRecord::abtPostpaid;
            smsc_log_debug(logger, "%s(%s): %s, IMSI %s, MSC <unknown>, SCFs: <none>",
                            taskName(), abonent.getSignals(), 
                            abInfo.abRec.type2Str(), abInfo.abRec.imsiCStr());
        } else {
            abInfo.abRec.ab_type = AbonentRecord::abtPrepaid;
            arg->getSCFinfo(&abInfo.abRec.tdpSCF[TDPCategory::dpMO_BC]);
            arg->getVLRN(abInfo.vlrNum);
            smsc_log_debug(logger, "%s(%s): %s, IMSI %s, MSC %s, %s",
                            taskName(), abonent.getSignals(),
                            abInfo.abRec.type2Str(),
                            abInfo.abRec.imsiCStr(), abInfo.vlr2Str().c_str(),
                            abInfo.abRec.tdpSCF.toString().c_str()
                );
        }
    }
}

void IAPQuerySRI::onEndMapDlg(RCHash ercode/* = 0*/)
{
    MutexGuard  grd(_mutex);
    if (ercode) {
        _qStatus = IAPQStatus::iqError;
        _qError = ercode;
        _exc = URCRegistry::explainHash(ercode);
        smsc_log_error(logger, "%s(%s): query failed: code 0x%x, %s",
                        taskName(), abonent.getSignals(), ercode, _exc.c_str());
    } else
        smsc_log_debug(logger, "%s(%s): query succeeded",
                        taskName(), abonent.getSignals());
}

} //sri
} //iaprvd
} //inman
} //smsc

