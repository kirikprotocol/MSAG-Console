#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

//#include <inman/common/XCFManager.hpp>
//using smsc::util::config::XCFManager;
using smsc::util::config::Config;
using smsc::util::config::ConfigException;

#include <inman/uss/UssSrvCfgReader.hpp>
using smsc::inman::uss::UssService_CFG;
using smsc::inman::uss::USSSrvCfgReader;

#include "USSManEmulService.hpp"
#include "USSManEmulConnect.hpp"
#include "MessagesProcessor.hpp"

using smsc::logger::Logger;

static smsc::inman::uss::USSManEmulService* service = 0;

volatile bool wasStopped = false;

extern "C" void sighandler( int signal )
{
  wasStopped = true;
}

static const char * const _ussSrv = "ussbalance";
static const char * const _ussLogId = "USSBalanceEm";

static void
fillServerSocketCfg(const smsc::util::config::ConfigView& tpConfig, smsc::inman::interaction::ServSocketCFG* socketCfg)
{
  socketCfg->host = tpConfig.getString("host");
  socketCfg->port = tpConfig.getInt("port");
  try {
    socketCfg->maxConn = tpConfig.getInt("maxConn");
  } catch (ConfigException & ex) {
    socketCfg->maxConn = 10;
  }
  try {
    socketCfg->timeout = tpConfig.getInt("timeout");
  } catch (ConfigException & ex) {
    socketCfg->timeout = 5;
  }
}

int main(int argc, char** argv)
{
  const char * cfgFile = "config.xml";

  tzset();
  Logger::Init();
  Logger * logger = Logger::getInstance("smsc.ussmanemu");

  if (argc > 1)
    cfgFile = argv[1];
  smsc_log_info(logger,"* Config file: %s", cfgFile);

  try {
    smsc::util::config::Manager::init(cfgFile);
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();
    smsc::util::config::ConfigView tpConfig(manager, "ussbalance");

    smsc::inman::interaction::ServSocketCFG socketCfg;
    fillServerSocketCfg(tpConfig, &socketCfg);

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

    smsc::util::config::ConfigView rulesConfig(manager, "RequestProcessingRules");
    service = new smsc::inman::uss::USSManEmulService(socketCfg, _ussLogId, logger);

    if ( !service->start() ) {
      smsc_log_error(logger, "%s: service startup failure!", _ussSrv);
      return 1;
    }

    smsc::inman::uss::MessagesProcessor::getInstance().init(rulesConfig);
    sigset( SIGTERM, sighandler );
    sigset( SIGPIPE, SIG_IGN );
    pthread_sigmask(SIG_SETMASK, &oldMask, NULL);

    while( !wasStopped )
      usleep( 1000 * 1000 );
    service->stop();
    delete service;

  } catch (const ConfigException & exc) {
    smsc_log_error(logger, "%s: %s", _ussSrv, exc.what());
    smsc_log_error(logger, "%s: Exiting!", _ussSrv);
    exit(-1);
  } catch(const std::exception& error) {
    smsc_log_fatal(logger, "%s", error.what() );
    fprintf( stderr, "Fatal error: %s\n", error.what() );
    delete service;
    return 1;
  }

  return 0;
}
