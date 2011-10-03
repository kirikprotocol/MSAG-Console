#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/version.hpp"
#include "inman/URCRegInit.hpp"
#include "inman/services/ICSRegistry.hpp"
#include "inman/services/SvcHost.hpp"
using smsc::logger::Logger;
using smsc::inman::ICServiceAC;
using smsc::inman::ICSState;
using smsc::inman::ICSLoadupsReg;
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

#ifdef CHECKCONTENTION
namespace smsc {
namespace core {
namespace synchronization {
unsigned Mutex::contentionLimit = 200000;

void Mutex::reportContention( const char* from,
                              unsigned    howlong,
                              unsigned    oldcount ) const throw()
{
    fprintf(stderr,"%s @%s contented by %u@%s for %u usec count=%u/%+d\n",
            what, from ? from:"unk",
            unsigned(wasid),
            wasfrom?wasfrom:"unk",
            howlong,lockCount,
            int(lockCount-oldcount));
}

}
}
}
#endif /* CHECKCONTENTION */


struct PhaseParms {
  const char *  _nmPhase;
  int           _failureRC;
};

//Matches the ICSState values
static PhaseParms _phaseParms[] = {
    { NULL, 0}
  , { "Configuration", -1}
  , { "Initialization", -2}
  , { "Startup", -3}
};

static void log_phase_ok(ICSState phase_id, Logger * use_log)
{
  smsc_log_info(use_log, "INMan: %s phase complete.", _phaseParms[phase_id]._nmPhase);
}

static int log_phase_fatal(ICSState phase_id, Logger * use_log, const char * err_cause)
{
  if (err_cause) {
    fprintf(stderr, "INMan: %s\n", err_cause);
    smsc_log_fatal(use_log, "INMan: %s", err_cause);
  }
  fprintf(stderr, "INMan: %s phase failure. Exiting.\n", _phaseParms[phase_id]._nmPhase);
  smsc_log_fatal(use_log, "INMan: %s phase failure. Exiting.", _phaseParms[phase_id]._nmPhase);
  return _phaseParms[phase_id]._failureRC;
}

static const char _hlpMsg[] =
 "USAGE: %s [-help | -list-uid] | [[-parse|-init] config_file_name]\n"
 "  -help      print help on command line arguments\n"
 "  -version   print version info\n"
 "  -list-uid  print supported ICService's UIds\n"
 "  -parse     parse services configuration and exit\n"
 "  -init      perform services initialization phase and exit\n"
 "  NOTE:      by default config_file_name is 'config.xml'\n";
int main(int argc, char** argv)
{
    int     rval = 0;
    const char *  cfgFile = (const char*)"config.xml";
    ICSState  tgtPhase = ICServiceAC::icsStStarted;

    if (argc > 1) { //parse command line args
      if (*argv[1] == '-') {
        if (!strcmp(argv[1], "-help")) {
          fprintf(stdout, _hlpMsg, argv[0]);
          return 0;
        }
        if (!strcmp(argv[1], "-version")) {
          fprintf(stdout, "%s\n", _getProductVersionStr());
          return 0;
        }
        if (!strcmp(argv[1], "-list-uid")) {
          fprintf(stdout, "Known UIds: %s\n", ICSLoadupsReg::get().knownUIds().toString().c_str());
          return 0;
        }
        if (!strcmp(argv[1], "-parse")) {
          tgtPhase = ICServiceAC::icsStConfig;
        } else if (!strcmp(argv[1], "-init")) {
          tgtPhase = ICServiceAC::icsStInited;
        } else {
          fprintf(stderr, _hlpMsg, argv[0]);
          return -1;
        }
        if (argc > 2)
          cfgFile = argv[2];
      } else
        cfgFile = argv[1];
    }

    tzset();
    URCRegistryGlobalInit();
    Logger::Init();
    inmanLogger = Logger::getInstance("smsc.inman");

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
        log_phase_ok(ICServiceAC::icsStConfig, inmanLogger);
        /* */
    } catch (const std::exception & exc) {
      rval = log_phase_fatal(ICServiceAC::icsStConfig, inmanLogger, exc.what());
    } catch (...) {
      rval = log_phase_fatal(ICServiceAC::icsStConfig, inmanLogger, "unknown exception caught");
    }

    if (_svcHost && (tgtPhase > ICServiceAC::icsStConfig)) {
      try {
        if (_svcHost->ICSInit() == ICServiceAC::icsRcOk)
          log_phase_ok(ICServiceAC::icsStInited, inmanLogger);
        else
          rval = log_phase_fatal(ICServiceAC::icsStInited, inmanLogger, NULL);
      } catch(const std::exception & error) {
        rval = log_phase_fatal(ICServiceAC::icsStInited, inmanLogger, error.what());
      } catch (...) {
        rval = log_phase_fatal(ICServiceAC::icsStInited, inmanLogger, "unknown exception caught");
      }
    }

    if (_svcHost && (tgtPhase > ICServiceAC::icsStInited)) {
      try {
        if (_svcHost->ICSStart() == ICServiceAC::icsRcOk) {
          _runService = 1;
          //handle SIGTERM only in main thread
          sigset(SIGTERM, sighandler);
          log_phase_ok(ICServiceAC::icsStStarted, inmanLogger);

          while(_runService)
            usleep(1000 * 200); //sleep 200 ms
          _svcHost->ICSStop();
        } else {
          rval = log_phase_fatal(ICServiceAC::icsStStarted, inmanLogger, NULL);
        }
      } catch(const std::exception & error) {
        rval = log_phase_fatal(ICServiceAC::icsStStarted, inmanLogger, error.what());
      } catch (...) {
        rval = log_phase_fatal(ICServiceAC::icsStStarted, inmanLogger, "unknown exception caught");
      }
    }

    if (_svcHost)
      delete _svcHost;
    smsc_log_info(inmanLogger, "INMan: shutdown complete");
    return rval;
}

