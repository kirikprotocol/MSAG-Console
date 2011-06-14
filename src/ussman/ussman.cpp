/* ************************************************************************** *
 * USSMan: Unstructured Supplementary services gateway 
 * (between IP and SS7 networks).
 * ************************************************************************** */
#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "util/config/XCFManager.hpp"
using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

#include "ussman/service/USSGCfgReader.hpp"
using smsc::ussman::USSGateway_XCFG;
using smsc::ussman::USSGCfgReader;

#include "ussman/service/USSGService.hpp"
#include "inman/uss/UssURCRegInit.hpp"
#include "ussman/ussgversion.hpp"

using smsc::logger::Logger;

//static smsc::inman::uss::USSGService * _gwService = 0;

volatile bool wasStopped = false;

extern "C" void sighandler( int signal )
{
  wasStopped = true;
}

/*
 * USSMan service entry point. Uses by default config.xml
 * USAGE: ussman [xml_config_file_name]
 */
static const char * const _ussSrv = "ussman"; //also names a configuration section in config.xml
static const char * const _ussLogId = "USSMan";
int main(int argc, char** argv)
{
  int   rval = 0;
  const char * cfgFile = "config.xml";

  tzset();
  Logger::Init();
  Logger * logger = Logger::getInstance("smsc.ussman");

  if (argc > 1)
    cfgFile = argv[1];

  std::string prodVer(_getProductVersionStr());
  {
    char * delim = new char[prodVer.length() + 4 + 1];
    memset(delim, '*', prodVer.length() + 4);

    smsc_log_info(logger, delim);
    smsc_log_info(logger,"* %s *", prodVer.c_str());
    smsc_log_info(logger, delim);
    smsc_log_info(logger,"* Config file: %s", cfgFile);
    smsc_log_info(logger, delim);

    delete [] delim;
  }

  URCRegistryGlobalInit();
  std::auto_ptr<USSGateway_XCFG> cfg;
  try {
    std::auto_ptr<Config> config(XCFManager::getInstance().getConfig(cfgFile)); //throws
    USSGCfgReader parser(_ussSrv, logger);

    parser.readConfig(*config.get()); //throws
    cfg.reset(parser.rlseConfig());
  } catch (const ConfigException & exc) {
    smsc_log_error(logger, "%s: %s", _ussSrv, exc.what());
    smsc_log_error(logger, "%s: Exiting!", _ussSrv);
    exit(-1);
  }

  sigset_t allBlockedSigs, oldMask;
  sigfillset(&allBlockedSigs);
  sigdelset(&allBlockedSigs, SIGABRT);
  sigdelset(&allBlockedSigs, SIGBUS);
  sigdelset(&allBlockedSigs, SIGFPE);
  sigdelset(&allBlockedSigs, SIGILL);
  sigdelset(&allBlockedSigs, SIGSEGV);
  sigdelset(&allBlockedSigs, SIGALRM);

  if (pthread_sigmask(SIG_SETMASK, &allBlockedSigs, &oldMask)) {
    smsc_log_fatal(logger, "%s: pthread_sigmask() was failed", _ussSrv);
    exit(-2);
  }

  std::auto_ptr<smsc::ussman::USSGService> _gwService;
  try {
    _gwService.reset(new smsc::ussman::USSGService(*cfg.get(), _ussLogId, logger));

    if (!_gwService->init()) {
      smsc_log_fatal(logger, "%s: service initialization failure!", _ussSrv);
      exit(-3);
    }
    if (!_gwService->start()) {
      smsc_log_fatal(logger, "%s: service startup failure!", _ussSrv);
      exit(-4);
    }
    sigset(SIGTERM, sighandler);
    pthread_sigmask(SIG_SETMASK, &oldMask, NULL);

//#ifndef LOCAL_TESTING
    while( !wasStopped )
      usleep( 1000 * 400 );
    _gwService->stop(true);
//#endif /* LOCAL_TESTING */
  } catch(const std::exception & error) {
    smsc_log_fatal(logger, "%s", error.what() );
    fprintf( stderr, "Fatal error: %s\n", error.what() );
    rval = -5;
  }

  _gwService.reset();
  smsc_log_info(logger, "%s shutdown complete", _ussLogId);
  return rval;
}
