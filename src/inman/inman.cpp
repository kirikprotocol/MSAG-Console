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
static const long _inman_MIN_BILLING_INTERVAL = 10; //in seconds

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


static Service* g_pService = 0;

static void init_logger()
{
  Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    tcapLogger = Logger::getInstance("smsc.inman.inap");
}

extern "C" static void sighandler( int signal )
{
    assert( g_pService );
    g_pService->stop();
    delete g_pService;
    g_pService = 0;
    smsc_log_info(inapLogger, "Service shutdown complete");
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
        ssf_addr = scf_addr = host = billingDir = NULL;
        ssn = port = billingInterval = 0;
        billMode = smsc::inman::BILL_ALL;
        cdrMode =  InService_CFG::CDR_ALL;
        capTimeout = tcpTimeout = 0;
    }

    void read(Manager& manager)
    {
        try {
            ssf_addr = manager.getString("ssfAddress");
            ssn = manager.getInt("ssn");
            smsc_log_info( inapLogger, "SSF : GT=%s,SSN=%d", ssf_addr,ssn );
        } catch(ConfigException& exc) {
            ssf_addr = 0; ssn = 0;
            throw ConfigException("ssfAddress or ssn missing");
        }
        try {
            scf_addr = manager.getString("scfAddress");
            smsc_log_info( inapLogger, "SCF : GT=%s", scf_addr );
        } catch (ConfigException& exc) {
            scf_addr = 0;
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
                billMode = smsc::inman::BILL_SMS;
            else if (!strcmp(bmode, _BILLmodes[smsc::inman::BILL_USSD]))
                billMode = smsc::inman::BILL_USSD;
            else if (!strcmp(bmode, _BILLmodes[smsc::inman::BILL_NONE]))
                billMode = smsc::inman::BILL_NONE;
            else if (strcmp(bmode, _BILLmodes[smsc::inman::BILL_ALL]))
                throw ConfigException("billMode unknown or missing");
            smsc_log_info(inapLogger, "billMode: %s [%d]", bmode, billMode);
        } catch (ConfigException& exc) {
            throw ConfigException("billMode unknown or missing");
        }
        try {
            char* cdrs = manager.getString("cdrMode");
            if (!strcmp(cdrs, _CDRmodes[InService_CFG::CDR_POSTPAID]))
                cdrMode = InService_CFG::CDR_POSTPAID;
            else if (!strcmp(cdrs, _CDRmodes[InService_CFG::CDR_NONE]))
                cdrMode = InService_CFG::CDR_NONE;
            else if (strcmp(cdrs, _CDRmodes[InService_CFG::CDR_ALL]))
                throw ConfigException("cdrMode unknown or missing");
            smsc_log_info(inapLogger, "cdrMode: %s [%d]", cdrs, cdrMode);
        } catch (ConfigException& exc) {
            throw ConfigException("cdrMode unknown or missing");
        }
        if (cdrMode != InService_CFG::CDR_NONE) {
            try {
                billingDir = manager.getString("billingDir");
                billingInterval = manager.getInt("billingInterval");
                if (billingInterval < _inman_MIN_BILLING_INTERVAL) {
                    billingDir = NULL; billingInterval = 0;
                    smsc_log_error(inapLogger, "Parameter 'billingInterval' should be grater than %ld seconds",
                                          _inman_MIN_BILLING_INTERVAL);
                    throw ConfigException("Parameter 'billingInterval' should be grater than %ld seconds",
                                          _inman_MIN_BILLING_INTERVAL);
                }
                smsc_log_info(inapLogger, "billingDir: %s", billingDir);
                smsc_log_info(inapLogger, "billingInterval: %d", billingInterval);
            } catch (ConfigException& exc) {
                billingDir = NULL; billingInterval = 0;
                throw ConfigException("billingDir or billingInterval invalid or missing");
            }
        }

        //OPTIONAL PARAMETERS:
        uint32_t tmo = 0;
        try {
            tmo = (uint32_t)manager.getInt("IN_Timeout");
        } catch (ConfigException& exc) {
        }
        if (tmo) {
            if (tmo >= 65535) {
                smsc_log_error(inapLogger, "Parameter 'IN_Timeout' should be less than 65535 seconds");
                throw ConfigException("Parameter 'IN_Timeout' should be less than 65535 seconds");
            } else { 
                capTimeout = (unsigned short)tmo;
                smsc_log_info(inapLogger, "IN_Timeout: %u", capTimeout);
            }
        } else 
            smsc_log_info(inapLogger, "IN_Timeout: default");

        tmo = 0;
        try {
            tmo = (uint32_t)manager.getInt("SMSC_Timeout");
        } catch (ConfigException& exc) {
        }
        if (tmo) {
            if (tmo >= 65535) {
                smsc_log_error(inapLogger, "Parameter 'SMSC_Timeout' should be less than 65535 seconds");
                throw ConfigException("Parameter 'SMSC_Timeout' should be less than 65535 seconds");
            } else { 
                tcpTimeout = (unsigned short)tmo;
                smsc_log_info(inapLogger, "SMSC_Timeout: %u", tcpTimeout);
            }
        } else
            smsc_log_info(inapLogger, "SMSC_Timeout: default");
    }
};

int main(int argc, char** argv)
{
    char *  cfgFile = (char*)"config.xml";

    init_logger();
    smsc_log_info( inapLogger,"***************************");
    smsc_log_info( inapLogger,"* SIBINCO IN MANAGER v%d.%d *", VER_HIGH, VER_LOW);
    smsc_log_info( inapLogger,"***************************");
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
        g_pService->start();

        sigset(SIGTERM, sighandler);
        while(g_pService)
            usleep(1000 * 100);

    } catch(const std::exception& error) {
        smsc_log_fatal(inapLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        delete g_pService;
        exit(1);
    }

    return(0);
}
