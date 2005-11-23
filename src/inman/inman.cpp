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
        Logger* dumpLogger;
      }
  }
};

using smsc::inman::Service;
using smsc::inman::InService_CFG;
using smsc::inman::inap::inapLogger;
using smsc::inman::inap::tcapLogger;
using smsc::inman::inap::dumpLogger;
using smsc::util::config::Manager;
using smsc::util::config::ConfigException;


static Service* g_pService = 0;

static void init_logger()
{
  Logger::Init();
    inapLogger = Logger::getInstance("smsc.inman");
    tcapLogger = Logger::getInstance("smsc.inman.inap.dump");
    dumpLogger = Logger::getInstance("smsc.inman.inap.dump");
}

extern "C" static void sighandler( int signal )
{
  assert( g_pService );
  g_pService->stop();
  delete g_pService;
  g_pService = 0;
}

static const char * const _CDRmodes[3] = { "all", "postpaid", "none" };

struct INBillConfig : public InService_CFG
{

public:
    INBillConfig() 
    {
        ssf_addr = scf_addr = host = billingDir = NULL;
        ssn = port = billingInterval = 0;
        billMode = InService_CFG::BILL_ALL;
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
            smsc_log_info(inapLogger, "SMSC: %s:%d", host, port);
        } catch (ConfigException& exc) {
            host = 0; port = 0;
            throw ConfigException("SMSC host or port missing");
        }
        try {
            char* cdrs = manager.getString("billMode");
            if (!strcmp(cdrs, _CDRmodes[InService_CFG::BILL_POSTPAID]))
                billMode = InService_CFG::BILL_POSTPAID;
            if (!strcmp(cdrs, _CDRmodes[InService_CFG::BILL_NONE]))
                billMode = InService_CFG::BILL_NONE;
            else if (strcmp(cdrs, _CDRmodes[InService_CFG::BILL_ALL]))
                throw ConfigException("billMode unknown or missing");
            smsc_log_info(inapLogger, "billMode: %s", cdrs);
        } catch (ConfigException& exc) {
            throw ConfigException("billMode unknown or missing");
        }
        if (billMode != InService_CFG::BILL_NONE) {
            try {
                billingDir = manager.getString("billingDir");
                billingInterval = manager.getInt("billingInterval");
                if (billingInterval < _inman_MIN_BILLING_INTERVAL) {
                    billingDir = NULL; billingInterval = 0;
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
        g_pService = new Service(&cfg);
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
