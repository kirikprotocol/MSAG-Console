#ifndef MOD_IDENT_OFF
static const char ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/abprov/IAPLoader.hpp"
#include "inman/abprov/hlr_sri/ProviderSRI.hpp"
#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::chsri::initMAPCHSRI3Components;

using smsc::util::URCRegistry;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

static const unsigned int _CFG_DFLT_QUERIES = 500;
static const unsigned int _CFG_DFLT_MAP_TIMEOUT = 20;

//This is the HLR(CH-SRI) Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(ConfigView* hlrCfg, Logger * use_log) throw(ConfigException)
{
    IAPCreatorSRI_CFG cfg;
    char * cstr = NULL;

    if (!(cstr = hlrCfg->getString("ownAddress")))
        throw ConfigException("'ownAddress' isn't set!");
    if (!cfg.owdAddr.fromText(cstr) || !cfg.owdAddr.fixISDN())
        throw ConfigException("'ownAddress' is invalid: %s !", cstr);

    if (!(cfg.ownSsn = (UCHAR_T)hlrCfg->getInt("ownSsn")))
        throw ConfigException("'ownSsn' is not set!", cstr);

    UCHAR_T tmp = 0;
    try { tmp = (UCHAR_T)hlrCfg->getInt("fakeSsn"); }
    catch (ConfigException & exc) { }
    if (tmp && (tmp != cfg.ownSsn))
        cfg.fakeSsn = tmp;

    try { cfg.max_queries = (unsigned)hlrCfg->getInt("maxQueries");
    } catch (ConfigException & exc) { }
    if (!cfg.max_queries) {
        cfg.defVal.queries = true;
        cfg.max_queries = _CFG_DFLT_QUERIES;
    }

    try { cfg.qryCfg.mapTimeout = (unsigned)hlrCfg->getInt("mapTimeout");
    } catch (ConfigException & exc) { }
    if (!cfg.qryCfg.mapTimeout) {
        cfg.defVal.mapTmo = true;
        cfg.qryCfg.mapTimeout = _CFG_DFLT_MAP_TIMEOUT;
    }

    if (!ApplicationContextFactory::Init(ACOID::id_ac_map_locInfoRetrieval_v3,
                                        initMAPCHSRI3Components))
        throw ConfigException("map_locInfoRetrieval_v3 component factory failure!");
    return new IAProviderCreatorSRI(cfg, use_log);
}

/* ************************************************************************** *
 * class IAProviderCreatorSRI implementation:
 * ************************************************************************** */
IAProviderCreatorSRI::IAProviderCreatorSRI(const IAPCreatorSRI_CFG & use_cfg,
                                         Logger * use_log/* = NULL*/)
    : cfg(use_cfg)
{
    std::string ctgr(use_log ? use_log->getName() : "smsc.inman");
    ctgr += ".iap.sri";
    logger = Logger::getInstance(ctgr.c_str());
    prvdCfg.init_threads = use_cfg.init_threads;
    prvdCfg.max_queries = use_cfg.max_queries;
    prvdCfg.qryMultiRun = true; //MapCHSRI dialogs are reused !!!
    prvdCfg.qryPlant = NULL;    //will be inited later
}

IAProviderCreatorSRI::~IAProviderCreatorSRI()
{
    if (prvdCfg.qryPlant)
        delete prvdCfg.qryPlant;
}

IAProviderITF * IAProviderCreatorSRI::getProvider(void)
{
    if (!prvdCfg.qryPlant) { //openSSN, initialize TCSessionMA
        TCAPDispatcher * disp = TCAPDispatcher::getInstance();
        if (disp->getState() != TCAPDispatcher::ss7CONNECTED) {
            smsc_log_error(logger, "TCAPDispatcher is not connected!");
            return NULL;
        }
        SSNSession * session = disp->openSSN(cfg.ownSsn, cfg.max_queries);
        if (!session) {
            smsc_log_error(logger, "SSN[%u] is unavailable!", (unsigned)cfg.ownSsn);
            return NULL;
        }
        if (!(cfg.qryCfg.mapSess = session->newMAsession(cfg.owdAddr.toString().c_str(),
            ACOID::id_ac_map_locInfoRetrieval_v3, 6, cfg.fakeSsn))) {
            smsc_log_error(logger, "Unable to init MAP session: %s -> %u:*",
                                  cfg.owdAddr.toString().c_str(), 6);
            return NULL;
        }
        smsc_log_info(logger, "iapSRI: TCMA[%u:%u] inited",
                      (unsigned)cfg.ownSsn, cfg.qryCfg.mapSess->getUID());
        prvdCfg.qryPlant = new IAPQuerySRIFactory(cfg.qryCfg, cfg.qryCfg.mapTimeout, logger);
        prvd.reset(new IAProviderThreaded(prvdCfg, logger));
    }
    return prvd.get();
}

void  IAProviderCreatorSRI::logConfig(Logger * use_log/* = NULL*/) const
{
    if (!use_log)
        use_log = logger;

    if (cfg.fakeSsn)
        smsc_log_info(use_log, "iapSRI: GT=%s, SSN=%u(fake=%u)",
                        cfg.owdAddr.getSignals(), (unsigned)cfg.ownSsn, (unsigned)cfg.fakeSsn);
    else
        smsc_log_info(use_log, "iapSRI: GT=%s, SSN=%u",
                        cfg.owdAddr.getSignals(), (unsigned)cfg.ownSsn);
    if (cfg.qryCfg.mapSess)
        smsc_log_info(use_log, "iapSRI: TCMA[%u:%u]",
                      (unsigned)cfg.ownSsn, cfg.qryCfg.mapSess->getUID());
    else
        smsc_log_info(use_log, "iapSRI: TCMA uninitialized yet");

    smsc_log_info(use_log, "iapSRI: Max.queries: %u%s", prvdCfg.max_queries,
                  cfg.defVal.queries ? " (default)" : "");
    smsc_log_info(use_log, "iapSRI: Query timeout: %u secs%s", cfg.qryCfg.mapTimeout,
                  cfg.defVal.mapTmo ? " (default)" : "");
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

