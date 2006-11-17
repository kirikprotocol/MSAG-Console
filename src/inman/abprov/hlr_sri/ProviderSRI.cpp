static const char ident[] = "$Id$";

#include "inman/abprov/hlr_sri/ProviderSRI.hpp"
#include "inman/inap/dispatcher.hpp"
using smsc::inman::inap::TCAPDispatcher;

#include "inman/comp/map_chsri/MapCHSRIFactory.hpp"
using smsc::inman::comp::chsri::initMAPCHSRI3Components;

namespace smsc {
namespace inman {
namespace iaprvd {
namespace sri {

static const unsigned int _CFG_DFLT_QUERIES = 500;
static const unsigned int _CFG_DFLT_MAP_TIMEOUT = 20;

//This is the HLR(SRI) Provider dynamic library entry point
extern "C" IAProviderCreatorITF * 
    loadupAbonentProvider(ConfigView* hlrCfg, Logger * use_log) throw(ConfigException)
{
    IAPCreatorSRI_CFG cfg;
    char * cstr = NULL;

    if (!(cstr = hlrCfg->getString("ownAddress")))
        throw ConfigException("'ownAddress' isn't set!");
    if (!cfg.owdAddr.fromText(cstr) || (cfg.owdAddr.numPlanInd != NUMBERING_ISDN)
        || (cfg.owdAddr.typeOfNumber > ToN_INTERNATIONAL))
        throw ConfigException("'ownAddress' is invalid: %s !", cstr);
    cfg.owdAddr.typeOfNumber = ToN_INTERNATIONAL;  //correct isdn unknown

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
        cfg.defVal.s.queries = 1;
        cfg.max_queries = _CFG_DFLT_QUERIES;
    }

    try { cfg.qryCfg.mapTimeout = (unsigned)hlrCfg->getInt("mapTimeout");
    } catch (ConfigException & exc) { }
    if (!cfg.qryCfg.mapTimeout) {
        cfg.defVal.s.mapTmo = 1;
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
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.iap.sri");
    prvdCfg.init_threads = use_cfg.init_threads;
    prvdCfg.max_queries = use_cfg.max_queries;
    prvdCfg.qryMultiRun = true; //MapCHSRI dialogs are reused !!!
    prvdCfg.qryPlant = NULL;    //will be inited later
}

IAProviderCreatorSRI::~IAProviderCreatorSRI()
{
    ProvidersLIST::iterator it = prvdList.begin();
    for (; it != prvdList.end(); it++)
        delete (*it);
    prvdList.clear();

    if (prvdCfg.qryPlant)
        delete prvdCfg.qryPlant;
}

IAProviderITF * IAProviderCreatorSRI::create(Logger * use_log)
{
    if (!use_log)
        use_log = logger;

    if (!prvdCfg.qryPlant) { //openSSN, initialize TCSessionMA
        TCAPDispatcher * disp = TCAPDispatcher::getInstance();
        if (disp->getState() != TCAPDispatcher::ss7CONNECTED) {
            smsc_log_error(use_log, "TCAPDispatcher is not connected!");
            return NULL;
        }
        SSNSession * session = disp->openSSN(cfg.ownSsn, cfg.max_queries);
        if (!session) {
            smsc_log_error(use_log, "SSN[%u] is unavailable!", (unsigned)cfg.ownSsn);
            return NULL;
        }
        if (!(cfg.qryCfg.mapSess = session->newMAsession(cfg.owdAddr.toString().c_str(),
            ACOID::id_ac_map_locInfoRetrieval_v3, 6, cfg.fakeSsn))) {
            smsc_log_error(use_log, "Unable to init MAP session: %s -> %u:*",
                                  cfg.owdAddr.toString().c_str(), 6);
            return NULL;
        }
        smsc_log_info(use_log, "iapSRI: TCMA[%u:%u] inited",
                      (unsigned)cfg.ownSsn, cfg.qryCfg.mapSess->getUID());
        prvdCfg.qryPlant = new IAPQuerySRIFactory(cfg.qryCfg, cfg.qryCfg.mapTimeout, logger);
    }
    IAProviderThreaded * prov = new IAProviderThreaded(prvdCfg, use_log);
    prvdList.push_back(prov);
    return prov;
}

void  IAProviderCreatorSRI::logConfig(Logger * use_log) const
{
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
                  cfg.defVal.s.queries ? " (default)" : "");
    smsc_log_info(use_log, "iapSRI: Query timeout: %u secs%s", cfg.qryCfg.mapTimeout,
                  cfg.defVal.s.mapTmo ? " (default)" : "");
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
    logger = use_log ? use_log : Logger::getInstance("smsc.inman.inap");
}

int IAPQuerySRI::Execute(void)
{ 
    {
        MutexGuard tmp(_mutex);
        if (isStopping || !_owner->hasListeners(abonent))
            return 0; //query was cancelled by either QueryManager or ThreadPool
    }
    int status = 0;
    try {
        sriDlg = new MapCHSRIDlg(_cfg.mapSess, this);
        sriDlg->reqRoutingInfo(abonent, _cfg.mapTimeout);
        if (qsig.Wait(_cfg.mapTimeout*1000 + 10) != 0)
            status = -2;
    } catch (std::exception & exc) {
        smsc_log_error(logger, "QrySRI[%u:%lu](%s): %s",
                       _qId, usage, abonent.getSignals(), exc.what());
        status = -1;
    }
    {
        MutexGuard  grd(_mutex);
        if (sriDlg) {
            delete sriDlg;
            sriDlg = NULL;
        }
    }
    return status;
}
// ****************************************
//-- CHSRIhandler interface:
// ****************************************
void IAPQuerySRI::onMapResult(CHSendRoutingInfoRes* arg)
{
    MutexGuard  grd(_mutex);
    if (!arg->getIMSI(subcrImsi))
        smsc_log_error(logger, "QrySRI[%u:%lu](%s): IMSI not determined.",
                        _qId, usage, abonent.getSignals());
    else {
        if (!arg->getSCFinfo(&abRec.gsmSCF)) {
            abRec.ab_type = smsc::inman::cache::btPostpaid;
            smsc_log_debug(logger, "QrySRI[%u:%lu](%s): gsmSCF not set.",
                           _qId, usage, abonent.getSignals());
        } else {
            abRec.ab_type = smsc::inman::cache::btPrepaid;
            smsc_log_debug(logger, "QrySRI[%u:%lu](%s): gsmSCF %s:%u",
                           _qId, usage, abonent.getSignals(),
                           abRec.gsmSCF.scfAddress.getSignals(), abRec.gsmSCF.serviceKey);
        }
    }
}

void IAPQuerySRI::onEndMapDlg(unsigned short ercode, InmanErrorType errLayer)
{
    MutexGuard  grd(_mutex);
    if (sriDlg) {
        delete sriDlg;
        sriDlg = NULL;
    }
    if (errLayer != smsc::inman::errOk)
        smsc_log_error(logger, "QrySRI[%u:%lu](%s): query failed: code %u, layer %s\n",
                       _qId, usage, abonent.getSignals(), ercode, _InmanErrorSource[errLayer]);
    qsig.Signal();
}

} //sri
} //iaprvd
} //inman
} //smsc

