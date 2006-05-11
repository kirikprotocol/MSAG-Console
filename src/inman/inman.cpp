static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>
#include <string>

#include "version.hpp"
#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::initCAP3SMSComponents;

#include "service.hpp"
#include "util/config/ConfigView.h"
#include "util/mirrorfile/mirrorfile.h"
using smsc::util::config::ConfigView;
using smsc::util::config::CStrSet;

#include "db/DataSourceLoader.h"
using smsc::db::DataSourceLoader;
using smsc::db::DataSourceFactory;

#include "inman/cachedb/cacheDb.hpp"
using smsc::inman::cache::db::DBSourceCFG;
using smsc::inman::cache::db::DBAbonentProvider;

namespace smsc {
  namespace inman {
    namespace inap {
        Logger* inapLogger;
        extern Logger * _EINSS7_logger_DFLT;
    } //inap
  }
};
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::_EINSS7_logger_DFLT;

using smsc::inman::Service;
using smsc::inman::InService_CFG;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

static const unsigned int _in_CFG_DFLT_CLIENT_CONNS = 3;
static const long _in_CFG_MIN_BILLING_INTERVAL = 10; //in seconds
static const unsigned int _in_CFG_MAX_BILLINGS = 100000;
static const unsigned int _in_CFG_DFLT_BILLINGS = 500;
static const unsigned short _in_CFG_DFLT_CAP_TIMEOUT = 20;
static const unsigned short _in_CFG_DFLT_BILL_TIMEOUT = 120;
static const unsigned short _in_CFG_DFLT_ABTYPE_TIMEOUT = 5;
static const unsigned char _in_CFG_DFLT_SS7_USER_ID = 3; //USER03_ID
static const unsigned short _in_CFG_DFLT_TCAP_DIALOGS = 2000;
static const long _in_CFG_DFLT_CACHE_INTERVAL = 1440;
static const int  _in_CFG_DFLT_CACHE_RECORDS = 10000;
#define RP_MO_SM_transfer_rejected 21       //3GPP TS 24.011 Annex E-2

static char     _runService = 0;
static Service* g_pService = 0;


extern "C" static void sighandler(int signal)
{
    _runService = 0;
}

//CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2
static const char * const _CDRmodes[3] = {"none", "all", "postpaid"};
//BILL_ALL = 0, BILL_USSD, BILL_SMS
static const char * const _BILLmodes[4] = {"none", "all", "ussd", "sms"};

static const char * const _PolicyModes[3] = {"IN", "DB", "HLR"};

static struct _DS_PARMS {
    const char* prmId;
    char*       prmVal;
}   _dsParm[3] = {
     { "type", NULL }
    ,{ "abonentQueryFunc", NULL }
    ,{ "queryFuncArg", NULL }
};
#define  _DS_IDENT_VAL  _dsParm[0].prmVal


struct INBillConfig : public InService_CFG {
    DBSourceCFG *dbProvPrm;
public:
    INBillConfig()
    {
        dbProvPrm = NULL; bill.abCache = NULL; bill.abProvider = NULL;
        sock.host = bill.ssf_addr = bill.scf_addr = bill.cdrDir = NULL;
        sock.port = bill.ssn = bill.serviceKey = 0;
        bill.cdrInterval = cachePrm.interval = cachePrm.RAM = 0;
        cachePrm.nmDir = NULL;
        bill.billMode = smsc::inman::BILL_ALL;
        bill.policy = smsc::inman::policyIN;
        bill.cdrMode =  BillingCFG::CDR_ALL;

        //OPTIONAL PARAMETERS:
        sock.maxConn = _in_CFG_DFLT_CLIENT_CONNS;
        bill.userId = _in_CFG_DFLT_SS7_USER_ID;
        bill.maxBilling = _in_CFG_DFLT_BILLINGS;
        bill.capTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
        bill.maxDlgId = _in_CFG_DFLT_TCAP_DIALOGS;
        sock.timeout = bill.maxTimeout = _in_CFG_DFLT_BILL_TIMEOUT;
        bill.abtTimeout = _in_CFG_DFLT_ABTYPE_TIMEOUT;
        bill.rejectRPC.push_back(RP_MO_SM_transfer_rejected);
        cachePrm.fileRcrd = _in_CFG_DFLT_CACHE_RECORDS;
    }

    ~INBillConfig()
    {
        if (dbProvPrm) {
             if (dbProvPrm->ds)
                 delete dbProvPrm->ds;
             delete dbProvPrm;
        }
    }

    void read(Manager& manager) throw(ConfigException)
    {
        uint32_t tmo = 0;
        char *   cstr = NULL;
        std::string cppStr;

        /* ********************* *
         * InService parameters: *
         * ********************* */

        try {
            sock.host = manager.getString("host");
            sock.port = manager.getInt("port");
            smsc_log_info(inapLogger, "INMan: %s:%d", sock.host, sock.port);
        } catch (ConfigException& exc) {
            sock.host = 0; sock.port = 0;
            throw ConfigException("INMan host or port missing");
        }
        tmo = 0;
        try { tmo = (uint32_t)manager.getInt("maxClients"); }
        catch (ConfigException& exc) { }
        if (tmo)
            sock.maxConn = (unsigned short)tmo;
        smsc_log_info(inapLogger, "maxClients: %s%u", !tmo ? "default ":"", sock.maxConn);

        /* ******************** *
         * Billing parameters:  *
         * ******************** */
        if (!manager.findSection("Billing"))
            throw ConfigException("\'Billing\' section is missed");

        ConfigView billCfg(manager, "Billing");
        cstr = NULL;
        try { cstr = billCfg.getString("billMode");
        } catch (ConfigException& exc) {
            throw ConfigException("'billMode' is unknown or missing");
        }
        if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_SMS]))
            bill.billMode = smsc::inman::BILL_SMS;
        else if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_USSD]))
            bill.billMode = smsc::inman::BILL_USSD;
        else if (!strcmp(cstr, _BILLmodes[smsc::inman::BILL_NONE]))
            bill.billMode = smsc::inman::BILL_NONE;
        else if (strcmp(cstr, _BILLmodes[smsc::inman::BILL_ALL]))
            throw ConfigException("'billMode' is unknown or missing");
        smsc_log_info(inapLogger, "billMode: %s [%d]", cstr, bill.billMode);

        cstr = NULL;
        try { cstr = billCfg.getString("abonentPolicy");
        } catch (ConfigException& exc) {
            throw ConfigException("'abonentPolicy' is unknown or missing");
        }
        if (!strcmp(cstr, _PolicyModes[smsc::inman::policyDB]))
            bill.policy = smsc::inman::policyDB;
        else if (!strcmp(cstr, _PolicyModes[smsc::inman::policyHLR])) {
            throw ConfigException("'abonentPolicy' HLR is not implemented");
//            bill.policy = smsc::inman::policyHLR;
        } else if (strcmp(cstr, _PolicyModes[smsc::inman::policyIN]))
            throw ConfigException("'abonentPolicy' is unknown or missing");
        smsc_log_info(inapLogger, "abonentPolicy: %s [%d]", cstr, bill.policy);

        cstr = NULL;
        try { cstr = billCfg.getString("cdrMode");
        } catch (ConfigException& exc) {
            throw ConfigException("'cdrMode' is unknown or missing");
        }
        if (!strcmp(cstr, _CDRmodes[BillingCFG::CDR_POSTPAID]))
            bill.cdrMode = BillingCFG::CDR_POSTPAID;
        else if (!strcmp(cstr, _CDRmodes[BillingCFG::CDR_NONE]))
            bill.cdrMode = BillingCFG::CDR_NONE;
        else if (strcmp(cstr, _CDRmodes[BillingCFG::CDR_ALL]))
            throw ConfigException("'cdrMode' is unknown or missing");
        smsc_log_info(inapLogger, "cdrMode: %s [%d]", cstr, bill.cdrMode);

        if (bill.cdrMode != BillingCFG::CDR_NONE) {
            try {
                bill.cdrDir = billCfg.getString("cdrDir");
                bill.cdrInterval = billCfg.getInt("cdrInterval");
            } catch (ConfigException& exc) {
                bill.cdrDir = NULL; bill.cdrInterval = 0;
                throw ConfigException("'cdrDir' or 'cdrInterval' is invalid or missing");
            }
            if (bill.cdrInterval < _in_CFG_MIN_BILLING_INTERVAL) {
                bill.cdrDir = NULL; bill.cdrInterval = 0;
                throw ConfigException("'cdrInterval' should be grater than %ld seconds",
                                      _in_CFG_MIN_BILLING_INTERVAL);
            }
            smsc_log_info(inapLogger, "cdrDir: %s", bill.cdrDir);
            smsc_log_info(inapLogger, "cdrInterval: %d secs", bill.cdrInterval);
        }
        //cache parameters
        try {
            if (!(cachePrm.interval = (long)billCfg.getInt("cacheInterval")))
                cachePrm.interval = _in_CFG_DFLT_CACHE_INTERVAL;
            smsc_log_info(inapLogger, "cacheInterval: %d minutes", cachePrm.interval);
            //convert minutes to seconds
            cachePrm.interval *= 60;
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheInterval' is missing");
        }
        try {
            if (!(cachePrm.RAM = (long)billCfg.getInt("cacheRAM")))
                throw ConfigException("'cacheRAM' is missing or invalid");
            smsc_log_info(inapLogger, "cacheRAM: %d Mb", cachePrm.RAM);
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheRAM' is missing or invalid");
        }
        try {
            cachePrm.nmDir = billCfg.getString("cacheDir");
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheDir' is missing");
        }
        tmo = 0;
        try { tmo = (uint32_t)billCfg.getInt("cacheRecords"); }
        catch (ConfigException& exc) { }
        if (tmo)
            cachePrm.fileRcrd = (int)tmo;
        smsc_log_info(inapLogger, "cacheRecords: %s%d", !tmo ? "default ":"", cachePrm.fileRcrd);

        /*  optional Billing parameters */

        tmo = 0;    //maxBillings
        try { tmo = (uint32_t)billCfg.getInt("maxBillings"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo > _in_CFG_MAX_BILLINGS)
                throw ConfigException(format("'maxBillings' should be less than %u",
                                             _in_CFG_MAX_BILLINGS).c_str());
            bill.maxBilling = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "maxBillings: %s%u per connect", !tmo ? "default ":"", bill.maxBilling);

        tmo = 0;    //maxTimeout
        try { tmo = (uint32_t)billCfg.getInt("maxTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65535) || (tmo < 5))
                throw ConfigException("'maxTimeout' should fall into the range [5 ..65535] seconds");
            sock.timeout = bill.maxTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "maxTimeout: %s%u secs", !tmo ? "default ":"", bill.maxTimeout);

        tmo = 0;    //abtTimeout
        try { tmo = (uint32_t)billCfg.getInt("abonentTypeTimeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'abonentTypeTimeout' should fall into the range [1 ..65535] seconds");
            bill.abtTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "abonentTypeTimeout: %s%u secs", !tmo ? "default ":"", bill.abtTimeout);

        /* ************************** *
         * IN interaction parameters: *
         * ************************** */
        if (!manager.findSection("IN_interaction"))
            throw ConfigException("\'IN_interaction\' section is missed");

        ConfigView inss7Cfg(manager, "IN_interaction");
        try {
            bill.ssf_addr = inss7Cfg.getString("ssfAddress");
            bill.ssn = inss7Cfg.getInt("ssn");
            smsc_log_info(inapLogger, "SSF : GT=%s, SSN=%d", bill.ssf_addr, bill.ssn);
        } catch (ConfigException& exc) {
            bill.ssf_addr = 0; bill.ssn = 0;
            throw ConfigException("SSF address or SSN missing");
        }
        try {
            bill.scf_addr = inss7Cfg.getString("scfAddress");
            smsc_log_info(inapLogger, "SCF : GT=%s", bill.scf_addr);
        } catch (ConfigException& exc) {
            bill.scf_addr = 0;
            throw ConfigException("SCF address missing");
        }
        try {
            bill.serviceKey = (unsigned int)inss7Cfg.getInt("serviceKey");
            smsc_log_info(inapLogger, "serviceKey: %d", bill.serviceKey);
        } catch (ConfigException& exc) {
            bill.serviceKey = 0;
            throw ConfigException("serviceKey is invalid or missing");
        }
        /*  optional IN interaction parameters */

        tmo = 0;    //ss7UserId
        try { tmo = (uint32_t)inss7Cfg.getInt("ss7UserId"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (!tmo || (tmo > 20))
                throw ConfigException("'ss7UserId' should fall into the range [1..20]");
            bill.userId = (unsigned char)tmo;
        }
        smsc_log_info(inapLogger, "ss7UserId: %s%u", !tmo ? "default ":"", bill.userId);

        tmo = 0;    //IN_Timeout
        try { tmo = (uint32_t)inss7Cfg.getInt("IN_Timeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'IN_Timeout' should be less than 65535 seconds");
            bill.capTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "IN_Timeout: %s%u secs", !tmo ? "default ":"", bill.capTimeout);

        tmo = 0;    //maxDialogs
        try { tmo = (uint32_t)inss7Cfg.getInt("maxDialogs"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if ((tmo >= 65530) || (tmo < 2))
                throw ConfigException("'maxDialogs' should fall into the range [2..65530]");
            bill.maxDlgId = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "maxDialogs: %s%u", !tmo ? "default ":"", bill.maxDlgId);

        cstr = NULL; cppStr = "RPCList_reject: ";
        try { cstr = inss7Cfg.getString("RPCList_reject"); }
        catch (ConfigException& exc) { }
        if (cstr) {
            try { bill.rejectRPC.init(cstr);
            } catch (std::exception& exc) {
                throw ConfigException(format("RPCList_reject: %s", exc.what()).c_str());
            }
        }
        if (!bill.rejectRPC.print(cppStr))
            cppStr += "unsupported";
        smsc_log_info(inapLogger, cppStr.c_str());

        cstr = NULL; cppStr = "RPCList_postpaid: ";
        try { cstr = inss7Cfg.getString("RPCList_postpaid"); }
        catch (ConfigException& exc) { }
        if (cstr) {
            try { bill.postpaidRPC.init(cstr); }
            catch (std::exception& exc) {
                throw ConfigException(format("RPCList_postpaid: %s", exc.what()).c_str());
            }
        } else if (bill.policy == smsc::inman::policyIN)
            throw ConfigException("'RPCList_postpaid' must be defined for abonentPolicy: IN");

        if (!bill.postpaidRPC.print(cppStr))
            cppStr += "unsupported";
        smsc_log_info(inapLogger, cppStr.c_str());


        /* *********************** *
         * DB sources parameters:  *
         * *********************** */
        if (bill.policy != smsc::inman::policyDB) {
            smsc_log_info(inapLogger, "DataProvider: not in use by abonentPolicy");
            return;
        }
        if (!manager.findSection("DataProvider"))
            throw ConfigException("'DataProvider' section is missed (abonentPolicy: DB)");
        
        ConfigView dbCfg(manager, "DataProvider");
        //load drivers first
        try { DataSourceLoader::loadup(&dbCfg);
        } catch (Exception& exc) {  //ConfigException or LoadupException
            throw ConfigException(exc.what());
        }
        //read DB connection parameters
        if (!manager.findSection("DataProvider.DataSource"))
            throw ConfigException("'DataProvider.DataSource' section is missed");

        ConfigView* dsCfg = dbCfg.getSubConfig("DataSource");
        for (int i = 0; i < 3; i++) {
            if (!(_dsParm[i].prmVal = dsCfg->getString(_dsParm[i].prmId)))
                throw ConfigException("'%s' isn't set!", _dsParm[i].prmId);
        }
        dbProvPrm = new DBSourceCFG;
        dbProvPrm->rtId = _dsParm[1].prmVal;
        dbProvPrm->rtKey = _dsParm[2].prmVal;

        dbProvPrm->ds = DataSourceFactory::getDataSource(_DS_IDENT_VAL);
        if (!dbProvPrm->ds)
            throw ConfigException("'%s' 'DataSource' isn't registered!", _DS_IDENT_VAL);
        dbProvPrm->ds->init(dsCfg);

        dbProvPrm->max_queries = (unsigned)dsCfg->getInt("maxQueries");
        dbProvPrm->init_threads = 1;
        bool wdog = false;
        try { wdog = dsCfg->getBool("watchdog"); }
        catch (ConfigException & exc) { }

        if (wdog)
            dbProvPrm->timeout = (unsigned)dsCfg->getInt("timeout");
        smsc_log_info(inapLogger, "Query timeout: %u secs", dbProvPrm->timeout);
        /**/
        return;
    }
};

int main(int argc, char** argv)
{
    int     rval = 0;
    char *  cfgFile = (char*)"config.xml";

    tzset();
    Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    _EINSS7_logger_DFLT = Logger::getInstance("smsc.inman.inap");

    smsc_log_info(inapLogger,"******************************");
    smsc_log_info(inapLogger,"* SIBINCO IN MANAGER v%u.%u.%u *",
                  INMAN_VER_HIGH, INMAN_VER_LOW, INMAN_VER_FIX);
    smsc_log_info(inapLogger,"******************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inapLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inapLogger,"******************************");

    INBillConfig cfg;
    try {
        Manager::init((const char *)cfgFile);
        Manager& manager = Manager::getInstance();
        cfg.read(manager);
    } catch (ConfigException& exc) {
        smsc_log_error(inapLogger, "Config: %s", exc.what());
        smsc_log_error(inapLogger, "Configuration invalid. Exiting.");
        exit(-1);
    }
    //INman uses the CAP3SMS application context for interaction with IN-point
    assert(
        ApplicationContextFactory::Init(ACOID::id_ac_cap3_sms_AC, initCAP3SMSComponents)/*;*/
    );

    try {
        if (cfg.dbProvPrm) {
            cfg.bill.abProvider = new DBAbonentProvider(cfg.dbProvPrm, inapLogger);
            assert(cfg.bill.abProvider);
            smsc_log_info(inapLogger, "AbonentsProvider inited.");
        }
        g_pService = new Service(&cfg, inapLogger);
        assert(g_pService);
        _runService = 1;
        if (g_pService->start()) {
            //handle SIGTERM only in main thread
            sigset(SIGTERM, sighandler);
        } else {
            smsc_log_fatal(inapLogger, "InmanSrv: startup failure. Exiting.");
            _runService = 0;
        }

        while(_runService)
            usleep(1000 * 200); //sleep 200 ms
        g_pService->stop();

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(inapLogger, "IN MANAGER shutdown complete");
    return rval;
}
