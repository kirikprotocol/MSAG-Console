static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <string.h>

#include "service.hpp"
#include "logger/Logger.h"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"

static const UCHAR_T VER_HIGH    = 0;
static const UCHAR_T VER_LOW     = 1;

static const long _in_CFG_MIN_BILLING_INTERVAL = 10; //in seconds
static const unsigned int _in_CFG_MAX_BILLINGS = 10000;


namespace smsc
{
  namespace inman
  {
    namespace inap
    {
        Logger* inapLogger;
        Logger* tcapLogger;
    }
  }
};
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::tcapLogger;

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
static const char * const _BILLmodes[4] = {"all", "ussd", "sms", "none"};


struct INBillConfig : public InService_CFG
{

public:
    INBillConfig() 
    {
        bill.ssf_addr = bill.scf_addr = host = bill.billingDir = NULL;
        bill.ssn = port = bill.billingInterval = 0;
        bill.billMode = smsc::inman::BILL_ALL;
        bill.cdrMode =  BillingCFG::CDR_ALL;
        bill.serviceKey = bill.capTimeout = bill.tcpTimeout =  bill.maxBilling = 0;
    }

    void read(Manager& manager)
    {
        try {
            bill.ssf_addr = manager.getString("ssfAddress");
            bill.ssn = manager.getInt("ssn");
            smsc_log_info(inapLogger, "SSF : GT=%s, SSN=%d", bill.ssf_addr, bill.ssn);
        } catch(ConfigException& exc) {
            bill.ssf_addr = 0; bill.ssn = 0;
            throw ConfigException("ssfAddress or ssn missing");
        }
        try {
            bill.scf_addr = manager.getString("scfAddress");
            smsc_log_info(inapLogger, "SCF : GT=%s", bill.scf_addr);
        } catch (ConfigException& exc) {
            bill.scf_addr = 0;
            throw ConfigException("scfAddress missing");
        }
        try {
            host = manager.getString("host");
            port = manager.getInt("port");
            smsc_log_info(inapLogger, "INMan: %s:%d", host, port);
        } catch (ConfigException& exc) {
            host = 0; port = 0;
            throw ConfigException("INMan host or port missing");
        }
        try {
            char* bmode = manager.getString("billMode");
            if (!strcmp(bmode, _BILLmodes[smsc::inman::BILL_SMS]))
                bill.billMode = smsc::inman::BILL_SMS;
            else if (!strcmp(bmode, _BILLmodes[smsc::inman::BILL_USSD]))
                bill.billMode = smsc::inman::BILL_USSD;
            else if (!strcmp(bmode, _BILLmodes[smsc::inman::BILL_NONE]))
                bill.billMode = smsc::inman::BILL_NONE;
            else if (strcmp(bmode, _BILLmodes[smsc::inman::BILL_ALL]))
                throw ConfigException("billMode unknown or missing");
            smsc_log_info(inapLogger, "billMode: %s [%d]", bmode, bill.billMode);
        } catch (ConfigException& exc) {
            throw ConfigException("billMode unknown or missing");
        }
        try {
            char* cdrs = manager.getString("cdrMode");
            if (!strcmp(cdrs, _CDRmodes[BillingCFG::CDR_POSTPAID]))
                bill.cdrMode = BillingCFG::CDR_POSTPAID;
            else if (!strcmp(cdrs, _CDRmodes[BillingCFG::CDR_NONE]))
                bill.cdrMode = BillingCFG::CDR_NONE;
            else if (strcmp(cdrs, _CDRmodes[BillingCFG::CDR_ALL]))
                throw ConfigException("cdrMode unknown or missing");
            smsc_log_info(inapLogger, "cdrMode: %s [%d]", cdrs, bill.cdrMode);
        } catch (ConfigException& exc) {
            throw ConfigException("cdrMode unknown or missing");
        }
        if (bill.cdrMode != BillingCFG::CDR_NONE) {
            try {
                bill.billingDir = manager.getString("billingDir");
                bill.billingInterval = manager.getInt("billingInterval");
                if (bill.billingInterval < _in_CFG_MIN_BILLING_INTERVAL) {
                    bill.billingDir = NULL; bill.billingInterval = 0;
                    smsc_log_error(inapLogger, "Parameter 'billingInterval' should be grater than %ld seconds",
                                          _in_CFG_MIN_BILLING_INTERVAL);
                    throw ConfigException("Parameter 'billingInterval' should be grater than %ld seconds",
                                          _in_CFG_MIN_BILLING_INTERVAL);
                }
                smsc_log_info(inapLogger, "billingDir: %s", bill.billingDir);
                smsc_log_info(inapLogger, "billingInterval: %d", bill.billingInterval);
            } catch (ConfigException& exc) {
                bill.billingDir = NULL; bill.billingInterval = 0;
                throw ConfigException("billingDir or billingInterval invalid or missing");
            }
        }
        try {
            bill.serviceKey = (unsigned int)manager.getInt("serviceKey");
            smsc_log_info(inapLogger, "serviceKey: %d", bill.serviceKey);
        } catch (ConfigException& exc) {
            bill.serviceKey = 0;
            throw ConfigException("serviceKey is invalid or missing");
        }

        //OPTIONAL PARAMETERS:
        uint32_t tmo = 0;
        //
        try { tmo = (uint32_t)manager.getInt("maxBillings"); }
        catch (ConfigException& exc) { }

        if (tmo) {
            if (tmo > _in_CFG_MAX_BILLINGS) {
                smsc_log_error(inapLogger, "Parameter 'IN_Timeout' should be less than 65535 seconds");
                throw ConfigException("Parameter 'IN_Timeout' should be less than 65535 seconds");
            } else {
                bill.maxBilling = (unsigned short)tmo;
                smsc_log_info(inapLogger, "maxBillings: %u", bill.maxBilling);
            }
        } else
            smsc_log_info(inapLogger, "maxBillings: default");

        //
        tmo = 0;
        try { tmo = (uint32_t)manager.getInt("IN_Timeout"); }
        catch (ConfigException& exc) { }

        if (tmo) {
            if (tmo >= 65535) {
                smsc_log_error(inapLogger, "Parameter 'IN_Timeout' should be less than 65535 seconds");
                throw ConfigException("Parameter 'IN_Timeout' should be less than 65535 seconds");
            } else { 
                bill.capTimeout = (unsigned short)tmo;
                smsc_log_info(inapLogger, "IN_Timeout: %u", bill.capTimeout);
            }
        } else 
            smsc_log_info(inapLogger, "IN_Timeout: default");

        //
        tmo = 0;
        try { tmo = (uint32_t)manager.getInt("SMSC_Timeout"); }
        catch (ConfigException& exc) { }
        if (tmo) {
            if (tmo >= 65535) {
                smsc_log_error(inapLogger, "Parameter 'SMSC_Timeout' should be less than 65535 seconds");
                throw ConfigException("Parameter 'SMSC_Timeout' should be less than 65535 seconds");
            } else { 
                bill.tcpTimeout = (unsigned short)tmo;
                smsc_log_info(inapLogger, "SMSC_Timeout: %u", bill.tcpTimeout);
            }
        } else
            smsc_log_info(inapLogger, "SMSC_Timeout: default");
    }
};

int main(int argc, char** argv)
{
    char *  cfgFile = (char*)"config.xml";

    Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    tcapLogger = Logger::getInstance("smsc.inman.inap");

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
        smsc_log_error(inapLogger, "Configuration invalid: %s. Exiting", exc.what());
        exit(-1);
    }
    try {
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

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        delete g_pService;
        exit(1);
    }
    smsc_log_info(inapLogger, "IN MANAGER shutdown complete");
    return(0);
}
