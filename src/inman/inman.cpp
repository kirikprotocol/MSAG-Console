static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>
#include <string>

#include "service.hpp"
#include "util/config/ConfigView.h"
using smsc::util::config::ConfigView;

#include "db/DataSourceLoader.h"
using smsc::db::DataSourceLoader;
using smsc::db::DataSourceFactory;

#include "inman/cachedb/cacheDb.hpp"
using smsc::inman::cache::db::DBSourceCFG;
using smsc::inman::cache::db::AbonentCacheDB;




static const UCHAR_T VER_HIGH    = 0;
static const UCHAR_T VER_LOW     = 1;

static const long _in_CFG_MIN_BILLING_INTERVAL = 10; //in seconds
static const unsigned int _in_CFG_MAX_BILLINGS = 10000;
static const unsigned int _in_CFG_DFLT_BILLINGS = 500;
static const unsigned short _in_CFG_DFLT_CAP_TIMEOUT = 20;
static const unsigned short _in_CFG_DFLT_TCP_TIMEOUT = 30;
static const unsigned char _in_CFG_DFLT_SS7_USER_ID = 3; //USER03_ID
static const long _in_CFG_DFLT_CACHE_INTERVAL = 1440;
#define RP_MO_SM_transfer_rejected 21       //3GPP TS 24.011 Annex E-2

namespace smsc {
  namespace inman {
    namespace inap {
        Logger* inapLogger;
        extern Logger * _EINSS7_logger_DFLT;
    }
  }
};
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::_EINSS7_logger_DFLT;

using smsc::inman::Service;
using smsc::inman::InService_CFG;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;


static char     _runService = 0;
static Service* g_pService = 0;


extern "C" static void sighandler( int signal )
{
    _runService = 0;
}

//CDR_NONE = 0, CDR_ALL = 1, CDR_POSTPAID = 2
static const char * const _CDRmodes[3] = {"none", "all", "postpaid"};
//BILL_ALL = 0, BILL_USSD, BILL_SMS
static const char * const _BILLmodes[4] = {"none", "all", "ussd", "sms"};

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
    DBSourceCFG *cacheDb;
public:
    INBillConfig()
    {
        cacheDb = NULL; bill.cache = NULL;
        host = bill.ssf_addr = bill.scf_addr = bill.billingDir = NULL;
        port = bill.ssn = bill.billingInterval = bill.cacheInterval = bill.cacheRAM = 0;
        bill.billMode = smsc::inman::BILL_ALL;
        bill.cdrMode =  BillingCFG::CDR_ALL;
        bill.serviceKey = bill.capTimeout = bill.tcpTimeout = 0;
        
        //OPTIONAL PARAMETERS:
        bill.userId = _in_CFG_DFLT_SS7_USER_ID;
        bill.maxBilling = _in_CFG_DFLT_BILLINGS;
        bill.capTimeout = _in_CFG_DFLT_CAP_TIMEOUT;
        bill.tcpTimeout = _in_CFG_DFLT_TCP_TIMEOUT;
        bill.rejectRPC.push_back(RP_MO_SM_transfer_rejected);
    }

    ~INBillConfig() { if (cacheDb) delete cacheDb; }

    void read(Manager& manager) throw(ConfigException)
    {
        uint32_t tmo = 0;
        char *   cstr = NULL;
        std::string cppStr;

        try {
            host = manager.getString("host");
            port = manager.getInt("port");
            smsc_log_info(inapLogger, "INMan: %s:%d", host, port);
        } catch (ConfigException& exc) {
            host = 0; port = 0;
            throw ConfigException("INMan host or port missing");
        }

        /* ******************** *
         * Billing parameters:  *
         * ******************** */
        ConfigView billCfg(manager, "Billing");
        if (!billCfg.getInstance())
            throw ConfigException("\'Billing\' section is missed");

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
                bill.billingDir = billCfg.getString("billingDir");
                bill.billingInterval = billCfg.getInt("billingInterval");
            } catch (ConfigException& exc) {
                bill.billingDir = NULL; bill.billingInterval = 0;
                throw ConfigException("'billingDir' or 'billingInterval' is invalid or missing");
            }
            if (bill.billingInterval < _in_CFG_MIN_BILLING_INTERVAL) {
                bill.billingDir = NULL; bill.billingInterval = 0;
                throw ConfigException("'billingInterval' should be grater than %ld seconds",
                                      _in_CFG_MIN_BILLING_INTERVAL);
            }
            smsc_log_info(inapLogger, "billingDir: %s", bill.billingDir);
            smsc_log_info(inapLogger, "billingInterval: %d", bill.billingInterval);
        }
        //cache parameters
        try {
            if (!(bill.cacheInterval = (long)billCfg.getInt("cacheInterval")))
                bill.cacheInterval = _in_CFG_DFLT_CACHE_INTERVAL;
            smsc_log_info(inapLogger, "cacheInterval: %d minutes", bill.cacheInterval);
            //convert minutes to seconds
            bill.cacheInterval *= 60;
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheInterval' is missing");
        }
        try {
            if (!(bill.cacheRAM = (long)billCfg.getInt("cacheRAM")))
                throw ConfigException("'cacheRAM' is missing or invalid");
            smsc_log_info(inapLogger, "cacheRAM: %d Mb", bill.cacheRAM);
        } catch (ConfigException& exc) {
            throw ConfigException("'cacheRAM' is missing or invalid");
        }

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

        tmo = 0;    //SMSC_Timeout
        try { tmo = (uint32_t)billCfg.getInt("SMSC_Timeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'SMSC_Timeout' should fall into the range [1 ..65535] seconds");
            bill.tcpTimeout = (unsigned short)tmo;
        }
        smsc_log_info(inapLogger, "SMSC_Timeout: %s%u", !tmo ? "default ":"", bill.tcpTimeout);

        /* ************************** *
         * IN interaction parameters: *
         * ************************** */
        ConfigView inss7Cfg(manager, "IN_interaction");
        if (!inss7Cfg.getInstance())
            throw ConfigException("\'IN_interaction\' section is missed");

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
        smsc_log_info(inapLogger, "'ss7UserId': %s%u", !tmo ? "default ":"", bill.userId);

        tmo = 0;    //IN_Timeout
        try { tmo = (uint32_t)inss7Cfg.getInt("IN_Timeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535)
                throw ConfigException("'IN_Timeout' should be less than 65535 seconds");
            bill.capTimeout = (unsigned short)tmo;
            smsc_log_info(inapLogger, "IN_Timeout: %u", bill.capTimeout);
        } else
            smsc_log_info(inapLogger, "IN_Timeout: default %u", bill.capTimeout);

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
        }
        if (!bill.postpaidRPC.print(cppStr))
            cppStr += "unsupported";
        smsc_log_info(inapLogger, cppStr.c_str());


        /* *********************** *
         * DB sources parameters:  *
         * *********************** */
        ConfigView dbCfg(manager, "DataProvider");
        if (dbCfg.getInstance()) {
            try { DataSourceLoader::loadup(&dbCfg);
            } catch (Exception& exc) {  //ConfigException or LoadupException
                throw ConfigException(exc.what());
            }
            ConfigView* dsCfg = dbCfg.getSubConfig("DataSource");
            if (!dsCfg->getInstance())
                throw ConfigException("'DataSource' section is missed");
            
            try {
                for (int i = 0; i < 3; i++) {
                    if (!(_dsParm[i].prmVal = dsCfg->getString(_dsParm[i].prmId)))
                        throw ConfigException("'%s' isn't set!", _dsParm[i].prmId);
                }
                DataSource * ds = DataSourceFactory::getDataSource(_DS_IDENT_VAL);
                if (!ds)
                    throw ConfigException("'%s' 'DataSource' isn't registered!", _DS_IDENT_VAL);
                ds->init(dsCfg);

                cacheDb = new DBSourceCFG;
                cacheDb->ds = ds;
                cacheDb->rtId = _dsParm[1].prmVal;
                cacheDb->rtKey = _dsParm[2].prmVal;
                cacheDb->max_queries = (unsigned)dsCfg->getInt("maxQueries");
                cacheDb->init_threads = 1;

            } catch (ConfigException& exc) {
                if (cacheDb && cacheDb->ds) {
                    delete cacheDb->ds;
                    delete cacheDb;
                    cacheDb = NULL;
                }
                throw exc;
            }
        } //dbCfg
    }
};

int main(int argc, char** argv)
{
    char *  cfgFile = (char*)"config.xml";

    tzset();
    Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    _EINSS7_logger_DFLT = Logger::getInstance("smsc.inman.inap");

    smsc_log_info(inapLogger,"***************************");
    smsc_log_info(inapLogger,"* SIBINCO IN MANAGER v%d.%d *", VER_HIGH, VER_LOW);
    smsc_log_info(inapLogger,"***************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inapLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inapLogger,"****************************");

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

    try {
        if (cfg.cacheDb) {
            cfg.bill.cache = new AbonentCacheDB(cfg.cacheDb, cfg.bill.cacheInterval, inapLogger);
            assert(cfg.bill.cache);
        }
        g_pService = new Service(&cfg, inapLogger);
        assert(g_pService);
        _runService = 1;
        g_pService->start();

        //handle SIGTERM only in main thread
        sigset(SIGTERM, sighandler);
        while(_runService)
            usleep(1000 * 200); //sleep 200 ms

        g_pService->stop();
        delete g_pService;
        if (cfg.cacheDb)
            delete cfg.cacheDb;

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        delete g_pService;
        if (cfg.cacheDb)
            delete cfg.cacheDb;
        exit(1);
    }
    smsc_log_info(inapLogger, "IN MANAGER shutdown complete");
    return(0);
}
