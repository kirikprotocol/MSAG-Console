#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include <assert.h>

#include "version.hpp"
#include "InmanCfg.hpp"

#include "inman/comp/cap_sms/CapSMSFactory.hpp"
using smsc::inman::comp::initCAP3SMSComponents;

namespace smsc {
  namespace inman {
    namespace inap {
        Logger* inmanLogger;
        extern Logger * _EINSS7_logger_DFLT;
    } //inap
  }
};
using smsc::inman::inap::inmanLogger;
using smsc::inman::inap::_EINSS7_logger_DFLT;

using smsc::inman::INManConfig;
using smsc::inman::Service;

static char     _runService = 0;
static Service* g_pService = 0;

extern "C" static void sighandler(int signal)
{
    _runService = 0;
}


int main(int argc, char** argv)
{
    int     rval = 0;
    char *  cfgFile = (char*)"config.xml";

    tzset();
    Logger::Init();
    inmanLogger = Logger::getInstance("smsc.inman");
    _EINSS7_logger_DFLT = Logger::getInstance("smsc.inman.inap");

    smsc_log_info(inmanLogger,"******************************");
    smsc_log_info(inmanLogger,"* SIBINCO IN MANAGER v%u.%u.%u *",
                  INMAN_VER_HIGH, INMAN_VER_LOW, INMAN_VER_FIX);
    smsc_log_info(inmanLogger,"******************************");
    if (argc > 1)
        cfgFile = argv[1];
    smsc_log_info(inmanLogger,"* Config file: %s", cfgFile);
    smsc_log_info(inmanLogger,"******************************");

    std::auto_ptr<INManConfig> pCfg(new INManConfig(inmanLogger));
    try {
        Manager::init((const char *)cfgFile);
        pCfg->read(Manager::getInstance());
        const char * nm_xcfg = pCfg->hasExtraConfig();
        if (nm_xcfg) {
            Manager::deinit();
            smsc_log_info(inmanLogger, "Reading smsExtra config %s ..", nm_xcfg);
            Manager::init(nm_xcfg);
            pCfg->readExtraConfig(Manager::getInstance());
        }
    } catch (ConfigException& exc) {
        smsc_log_error(inmanLogger, "Config: %s", exc.what());
        smsc_log_error(inmanLogger, "Configuration invalid. Exiting.");
        exit(-1);
    }
    //INman uses the CAP3SMS application context for interaction with IN-point
    assert(
        ApplicationContextFactory::Init(ACOID::id_ac_cap3_sms_AC, initCAP3SMSComponents)/*;*/
    );

    try {
        g_pService = new Service(pCfg.release(), inmanLogger);
        assert(g_pService);
        _runService = 1;
        if (g_pService->start()) {
            //handle SIGTERM only in main thread
            sigset(SIGTERM, sighandler);
        } else {
            smsc_log_fatal(inmanLogger, "InmanSrv: startup failure. Exiting.");
            _runService = 0;
        }

        while(_runService)
            usleep(1000 * 200); //sleep 200 ms
        g_pService->stop();

    } catch(const std::exception& error) {
        smsc_log_fatal(inmanLogger, "%s", error.what() );
        fprintf( stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (g_pService)
        delete g_pService;
    smsc_log_info(inmanLogger, "IN MANAGER shutdown complete");
    return rval;
}

