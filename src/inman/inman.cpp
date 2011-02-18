#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <assert.h>

#include "inman/version.hpp"
#include "inman/URCRegInit.hpp"
#include "inman/services/SvcHost.hpp"
using smsc::logger::Logger;
using smsc::inman::ICServiceAC;
using smsc::inman::SVCHost;
using smsc::inman::SVCHostProducer;
using smsc::inman::ICSHostCfgReader;
using smsc::inman::ICSrvCfgReaderAC;
using smsc::inman::XMFConfig;

using smsc::util::config::XCFManager;
//using smsc::util::config::Config;
using smsc::util::config::ConfigException;


namespace smsc {
  namespace inman {
    Logger* inmanLogger;
  }
};
using smsc::inman::inmanLogger;

static char         _runService = 0;
static SVCHost *    _svcHost = 0;

extern "C" void sighandler(int signal)
{
    _runService = 0;
}


int main(int argc, char** argv)
{
    int     rval = 0;
    char *  cfgFile = (char*)"config.xml";

    tzset();
    URCRegistryGlobalInit();
    Logger::Init();
    inmanLogger = Logger::getInstance("smsc.inman");

    if (argc > 1)
        cfgFile = argv[1];

    std::string prodVer(_getProductVersionStr());
    {
      char * delim = new char[prodVer.length() + 4 + 1];
      memset(delim, '*', prodVer.length() + 4);

      smsc_log_info(inmanLogger, delim);
      smsc_log_info(inmanLogger,"* %s *", prodVer.c_str());
      smsc_log_info(inmanLogger, delim);
      smsc_log_info(inmanLogger,"* Config file: %s", cfgFile);
      smsc_log_info(inmanLogger, delim);

      delete [] delim;
    }

    try {
        XMFConfig   config(XCFManager::getInstance());
        config.parseSectionsConfig(cfgFile); //throws

        SVCHostProducer   hostProd;
        ICSHostCfgReader * xcfReader = (ICSHostCfgReader *)
                              hostProd.newCfgReader(config, 0, inmanLogger);

        xcfReader->readConfig(); //throws
        if (xcfReader->icsCfgState() != ICSrvCfgReaderAC::cfgComplete)
          throw ConfigException("SVCHost configuration is incomplete");

        _svcHost = (SVCHost*)hostProd.newService(0, inmanLogger);
        smsc_log_info(inmanLogger, "INMan: configuration processed.\n");
    } catch (const ConfigException & exc) {
        smsc_log_fatal(inmanLogger, "INMan: %s", exc.what());
        smsc_log_fatal(inmanLogger, "Configuration invalid. Exiting.");
        exit(-1);
    } catch (...) {
      smsc_log_fatal(inmanLogger, "INMan: unknown exception caught");
      smsc_log_fatal(inmanLogger, "Configuration invalid. Exiting.");
      exit(-1);
    }

    try {
        if (_svcHost->ICSInit() == ICServiceAC::icsRcOk) {
            smsc_log_info(inmanLogger, "INMan: initialization complete.\n");
            _runService = 1;
            if (_svcHost->ICSStart() == ICServiceAC::icsRcOk) {
                //handle SIGTERM only in main thread
                sigset(SIGTERM, sighandler);
            } else {
                smsc_log_fatal(inmanLogger, "INMan: startup failure. Exiting.");
                _runService = 0;
            }
            while(_runService)
                usleep(1000 * 200); //sleep 200 ms
            _svcHost->ICSStop();
        } else {
            smsc_log_fatal(inmanLogger, "INMan: initialization failure. Exiting.");
        }
    } catch(const std::exception & error) {
        smsc_log_fatal(inmanLogger, "INMan: %s", error.what() );
        fprintf(stderr, "Fatal error: %s\n", error.what() );
        rval = 1;
    }
    if (_svcHost)
        delete _svcHost;
    smsc_log_info(inmanLogger, "INMan: shutdown complete");
    return rval;
}

