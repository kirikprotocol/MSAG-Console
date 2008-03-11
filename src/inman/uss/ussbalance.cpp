/* ************************************************************************** *
 * USSBalance service: utilizes the USSMan functionality for handling abonents
 * balance requests.
 * ************************************************************************** */
#ifndef MOD_IDENT_OFF
static char const ident[] = "$Id$";
#endif /* MOD_IDENT_OFF */

#include "inman/common/XCFManager.hpp"
using smsc::util::config::XCFManager;

#include "UssSrvCfgReader.hpp"
using smsc::inman::uss::UssService_CFG;
using smsc::inman::uss::USSSrvCfgReader;

#include "USSManService.hpp"
#include "UssURCRegInit.hpp"

static smsc::inman::uss::USSManService* service = 0;

volatile bool wasStopped = false;

extern "C" static void sighandler( int signal )
{
  wasStopped = true;
}

/*
 * USSBalance service entry point. Uses by default config.xml
 * USAGE: ussman [xml_config_file_name]
 */
static const char * const _ussSrv = "ussbalance";
static const char * const _ussLogId = "USSBalance";
int main(int argc, char** argv)
{
  const char * cfgFile = "config.xml";

  tzset();
  Logger::Init();
  Logger * logger = Logger::getInstance("smsc.ussman");

  smsc_log_info(logger,"*******************************");
  smsc_log_info(logger,"* SIBINCO %s MANAGER *", _ussLogId);
  smsc_log_info(logger,"*******************************");

  if (argc > 1)
    cfgFile = argv[1];
  smsc_log_info(logger,"* Config file: %s", cfgFile);
  smsc_log_info(logger,"*******************************");

  URCRegistryGlobalInit();
  std::auto_ptr<UssService_CFG> cfg;
  try {
      std::auto_ptr<Config> config(XCFManager::getInstance().getConfig(cfgFile)); //throws
      USSSrvCfgReader parser(_ussSrv, logger);
      parser.readConfig(*config.get()); //throws
      cfg.reset(parser.rlseConfig());
  } catch (const ConfigException & exc) {
      smsc_log_error(logger, "%s: %s", _ussSrv, exc.what());
      smsc_log_error(logger, "%s: Exiting!", _ussSrv);
      exit(-1);
  }

  try {
    sigset_t allBlockedSigs, oldMask;
    sigfillset(&allBlockedSigs);
    sigdelset(&allBlockedSigs, SIGABRT);
    sigdelset(&allBlockedSigs, SIGBUS);
    sigdelset(&allBlockedSigs, SIGFPE);
    sigdelset(&allBlockedSigs, SIGILL);
    sigdelset(&allBlockedSigs, SIGSEGV);
    sigdelset(&allBlockedSigs, SIGALRM);

    if ( pthread_sigmask(SIG_SETMASK, &allBlockedSigs, &oldMask) ) {
      smsc_log_error(logger, "%s: call to pthread_sigmask was failed", _ussSrv);
      return 1;
    }
    service = new smsc::inman::uss::USSManService(*cfg.get(), _ussLogId, logger);

    if ( !service->start() ) {
      smsc_log_error(logger, "%s: service startup failure!", _ussSrv);
      return 1;
    }

    sigset( SIGTERM, sighandler );
    pthread_sigmask(SIG_SETMASK, &oldMask, NULL);
#ifndef LOCAL_TESTING
    while( !wasStopped )
      usleep( 1000 * 1000 );
    service->stop();
    delete service;
#endif /* LOCAL_TESTING */
  } catch(const std::exception& error) {
    smsc_log_fatal(logger, "%s", error.what() );
    fprintf( stderr, "Fatal error: %s\n", error.what() );
    delete service;
    return 1;
  }

  return(0);
}
