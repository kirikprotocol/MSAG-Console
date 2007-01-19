static char const ident[] = "$Id$";

#include <stdio.h>
#include <assert.h>
#include <memory>
#include <signal.h>
#include <pthread.h>

#include <logger/Logger.h>
#include <util/config/Manager.h>
#include <util/config/ConfigView.h>

#include "USSBalanceService.hpp"
#include "UssServiceCfg.hpp"
#include <inman/comp/map_uss/MapUSSFactory.hpp>

using smsc::util::config::Manager;
using smsc::util::config::ConfigException;

static smsc::inman::uss::USSBalanceService* service = 0;

smsc::logger::Logger* logger;

static void init_logger()
{
  Logger::Init();
  logger = Logger::getInstance("smsc.ussbalance");
}

volatile bool wasStopped = false;

extern "C" static void sighandler( int signal )
{
  wasStopped = true;
}

class UssServiceConfig : public smsc::inman::uss::UssService_CFG
{
public:
  UssServiceConfig()
  {
    sock.host = "";
    sock.port = sock.maxConn = sock.timeout = 0;
  }
  void read(Manager& manager)
  {
    if (!manager.findSection("ussbalance"))
      throw ConfigException("\'ussbalance\' section is missed");
    smsc::util::config::ConfigView ussBalanceCfg(manager, "ussbalance");

    sock.host = ussBalanceCfg.getString("host");
    sock.port = ussBalanceCfg.getInt("port");

    smsc_log_info(logger, "UssServiceConfig : host=%s,port=%d", sock.host.c_str(),sock.port);

    try {
      sock.maxConn = ussBalanceCfg.getInt("maxConn");
      smsc_log_info(logger, "UssServiceConfig : use maxConn value=%d", sock.maxConn);
    } catch(ConfigException& exc) {
      sock.maxConn = 1;
      smsc_log_info(logger, "UssServiceConfig : use default maxConn value=%d", sock.maxConn);
    }

    try {
      sock.timeout = ussBalanceCfg.getInt("timeout");
      smsc_log_info(logger, "UssServiceConfig : use timeout value=%d", sock.timeout);
    } catch(ConfigException& exc) {
      sock.timeout = 10;
      smsc_log_info(logger, "UssServiceConfig : use default timeout value=%d", sock.timeout);
    }
    readSS7CFG(manager, ss7);
  }
private:
  void readSS7CFG(Manager & manager, smsc::inman::uss::SS7_CFG & ss7ConfigValue) throw(ConfigException)
  {
    if (!manager.findSection("SS7"))
      throw ConfigException("\'SS7\' section is missed");

    uint32_t tmo;
    char *   cstr;
    ConfigView ss7Cfg(manager, "SS7");

    cstr = NULL;
    try { cstr = ss7Cfg.getString("ssfAddress"); }
    catch (ConfigException& exc) { }
    if (!cstr || !cstr[0])
      throw ConfigException("SSF address is missing");
    if (!ss7ConfigValue.ssf_addr.fromText(cstr)
        || (ss7ConfigValue.ssf_addr.numPlanInd != NUMBERING_ISDN)
        || (ss7ConfigValue.ssf_addr.typeOfNumber > ToN_INTERNATIONAL))
      throw ConfigException("SSF address is invalid: %s", cstr);
    ss7ConfigValue.ssf_addr.typeOfNumber = ToN_INTERNATIONAL; //correct isdn unknown
    ss7ConfigValue.own_ssn = ss7Cfg.getInt("ssn"); //throws

    smsc_log_info(logger, "SSF: %u:%s", ss7ConfigValue.own_ssn,
                  ss7ConfigValue.ssf_addr.toString().c_str());

    /*  optional SS7 interaction parameters */
    tmo = 0;    //ss7UserId
    try { tmo = (uint32_t)ss7Cfg.getInt("ss7UserId"); }
    catch (ConfigException& exc) { tmo = 3; }

    if (!tmo || (tmo > 20))
        throw ConfigException("'ss7UserId' should fall into the range [1..20]");
    ss7ConfigValue.userId = (unsigned char)tmo;

    smsc_log_info(logger, "ss7UserId: %s%u", !tmo ? "default ":"", ss7ConfigValue.userId);

    tmo = 0;    //maxTimeout
    try { tmo = (uint32_t)ss7Cfg.getInt("maxTimeout"); }
    catch (ConfigException& exc) { tmo = 20; }
    if (tmo >= 65535)
      throw ConfigException("'maxTimeout' should be less than 65535 seconds");
    ss7ConfigValue.capTimeout = (unsigned short)tmo;

    smsc_log_info(logger, "maxTimeout: %s%u secs", !tmo ? "default ":"", ss7ConfigValue.capTimeout);

    tmo = 0;    //maxDialogs
    try { tmo = (uint32_t)ss7Cfg.getInt("maxDialogs"); }
    catch (ConfigException& exc) { tmo = 2000; }

    if ((tmo >= 65530) || (tmo < 2))
      throw ConfigException("'maxDialogs' should fall into the range [2..65530]");
    ss7ConfigValue.maxDlgId = (unsigned short)tmo;

    smsc_log_info(logger, "maxDialogs: %s%u", !tmo ? "default ":"", ss7ConfigValue.maxDlgId);

    ss7ConfigValue.fake_ssn = 0;
    try {
      uint32_t fakeSSN = (uint32_t)ss7Cfg.getInt("fakeSsn");

      ss7ConfigValue.fake_ssn = static_cast<uint8_t>(fakeSSN);
      if ( ss7ConfigValue.fake_ssn != fakeSSN )
        throw ConfigException("'fakeSsn' should fall into the range [0..255]");
    } catch (ConfigException& exc) {}

    smsc_log_info(logger, "fakeSsn: %u", ss7ConfigValue.fake_ssn);

    return;
  }

};

/*
 * USSBalance service entry point. Uses by default config.xml
 * USAGE: ussman [xml_config_file_name]
 */
int main(int argc, char** argv)
{
  const char *  cfgFile = "config.xml";

  init_logger();

  assert(
         smsc::inman::comp::ApplicationContextFactory::Init(smsc::ac::ACOID::id_ac_map_networkUnstructuredSs_v2,
                                                            smsc::inman::comp::uss::initMAPUSS2Components)
         );

  smsc_log_info(logger,"*******************************");
  smsc_log_info(logger,"* SIBINCO USS BALANCE MANAGER *");
  smsc_log_info(logger,"*******************************");

  if (argc > 1)
    cfgFile = argv[1];
  smsc_log_info(logger,"* Config file: %s", cfgFile);
  smsc_log_info(logger,"*******************************");

  UssServiceConfig cfg;
  try {
    Manager::init(cfgFile);
    Manager& manager = Manager::getInstance();
    cfg.read(manager);
  } catch (ConfigException& exc) {
    smsc_log_error(logger, "Configuration invalid: %s. Exiting", exc.what());
    return 1;
  }
  try {
    sigset_t allBlockedSigs, oldMask;
    sigfillset(&allBlockedSigs);
    if ( pthread_sigmask(SIG_SETMASK, &allBlockedSigs, &oldMask) ) {
      smsc_log_error(logger, "call to pthread_sigmask was failed");
      return 1;
    }
    service = new smsc::inman::uss::USSBalanceService(cfg);
    if ( !service->start() ) {
      smsc_log_error(logger, "Can't 'tcp server' thread");
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
