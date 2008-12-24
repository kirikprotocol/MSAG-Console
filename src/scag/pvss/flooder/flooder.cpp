#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <exception>

#include "logger/Logger.h"
#include "util/Exception.hpp"
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "scag/pvss/base/PersClientException.h"
#include "scag/pvss/client/PvssStreamClient.h"
#include "PvssFlooder.h"

using std::string;
using namespace scag2::pvss::flooder;
using namespace scag2::pvss;
using smsc::logger::Logger;

int resultCode = 0;
PvssFlooder* lcClient = 0;

extern "C" void appSignalHandler(int sig)
{
    Logger* logger = Logger::getInstance("lcclient");
    smsc_log_debug(logger, "Signal %d handled !", sig);
    if (sig==SIGTERM || sig==SIGINT)
    {
        if(lcClient) lcClient->shutdown();
        smsc_log_info(logger, "Stopping ...");
    }
    else if(sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

int main(int argc, char* argv[]) {

  int speed = 1000; //req/sec
  Logger* logger;
  Logger::Init();

  atexit(atExitHandler);

  sigset_t set;
  sigfillset(&set);
  sigdelset(&set, SIGTERM);
  sigdelset(&set, SIGINT);
  sigdelset(&set, SIGSEGV);
  sigdelset(&set, SIGBUS);
  sigdelset(&set, SIGHUP);
  sigprocmask(SIG_SETMASK, &set, NULL);
  sigset(SIGTERM, appSignalHandler);
  sigset(SIGINT, appSignalHandler);
  sigset(SIGHUP, appSignalHandler);   

  try {
    logger = Logger::getInstance("flooder");

    if (argc > 1) {
      speed = atoi(argv[1]);
    }
    smsc_log_info(logger,  "Starting up pvss flooder...");

    smsc::util::config::Manager::init("config.xml");
    smsc::util::config::Manager& manager = smsc::util::config::Manager::getInstance();

    smsc::util::config::ConfigView clientConfig(manager, "PvssClient");

    string host("phoenix");
    try { 
      host = clientConfig.getString("host");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.host> missed. Defaul value is %s", host.c_str());
    };
    int port = 27880;
    try { 
      port = clientConfig.getInt("port");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.port> missed. Defaul value is %d", port);
    };
    int timeout = 300;
    try { 
      timeout = clientConfig.getInt("ioTimeout");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.ioTimeout> missed. Defaul value is %d", timeout);
    };
    int pingtimeout = 300;
    try { 
      pingtimeout = clientConfig.getInt("pingTimeout");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.pingTimeout> missed. Defaul value is %d", pingtimeout);
    };
    int recontimeout = 10;
    try { 
      recontimeout = clientConfig.getInt("reconnectTimeout");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.reconnectTimeout> missed. Defaul value is %d", recontimeout);
    };
    int maxwait = 1000;
    try { 
      maxwait = clientConfig.getInt("maxWaitingRequestsCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.maxWaitingRequestsCount> missed. Defaul value is %d", maxwait);
    };
    int connects = 1;
    try { 
      connects = clientConfig.getInt("connections");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.connections> missed. Defaul value is %d", connects);
    };
    bool async = false;
    try { 
      async = clientConfig.getBool("async");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.async> missed. Defaul value is false");
    };

    PvssStreamClient* pc = new PvssStreamClient;
    pc->init( host.c_str(),
              port,
              timeout,
              pingtimeout,
              recontimeout,
              maxwait,
              connects,
              async );

    smsc::util::config::ConfigView flooderConfig(manager, "Flooder");
    
    try { 
      speed = flooderConfig.getInt("speed");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.speed> missed. Defaul value is %d", speed);
    };
    int getsetCount = 1;
    try { 
      getsetCount = flooderConfig.getInt("getsetCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.getsetCount> missed. Defaul value is %d", getsetCount);
    };
    int addressesCount = 1000000;
    try { 
      addressesCount = flooderConfig.getInt("addressesCount");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <Flooder.addressesCount> missed. Defaul value is %d", addressesCount);
    };

    lcClient = new PvssFlooder(PersClient::Instance(), speed);
    CallsCounter counter(lcClient, 5);

    lcClient->execute(addressesCount, getsetCount);
    counter.stop();
    delete lcClient;
  }
  catch (const PersClientException& exc) 
  {
      smsc_log_error(logger, "PersClientException: %s Exiting.", exc.what());
      resultCode = -2;
  }
  catch (const smsc::util::Exception& exc) 
  {
      smsc_log_error(logger, "Top level Exception: %s Exiting.", exc.what());
      resultCode = -3;
  }
  catch (const std::exception& exc) 
  {
      smsc_log_error(logger, "Top level exception: %s Exiting.", exc.what());
      resultCode = -4;
  }
  catch (...) 
  {
      smsc_log_error(logger, "Unknown exception: '...' caught. Exiting.");
      resultCode = -5;
  }
  return resultCode;
}

