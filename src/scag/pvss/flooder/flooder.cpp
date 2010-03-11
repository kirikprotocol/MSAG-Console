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
#include "scag/pvss/api/core/ConfigException.h"
#include "PvssFlooder.h"
#include "FlooderConfig.h"
#include "ConfigUtil.h"
#include "scag/util/io/Drndm.h"


using std::string;
using namespace scag2::pvss::flooder;
using namespace scag2::pvss::client;
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

int usage()
{
    fprintf(stderr,"Usage: flooder [options]\n");
    fprintf(stderr," -s | --speed SPEED   specify the speed of flood\n");
    fprintf(stderr," -k | --skip  SKIP    specify how many profiles to skip\n");
    fprintf(stderr," -h | --host  HOST    specify the host to connect\n");
    fprintf(stderr," -p | --post  PORT    specify the port to connect\n");
    return -1;
}


int main(int argc, char* argv[]) {

  int speed = 0; //req/sec
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
    int connPerThread = 5;
    try { 
      connPerThread = clientConfig.getInt("connPerThread");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.connPerThread> missed. Defaul value is %d", connPerThread);
    };
    bool async = true;
    try { 
      async = clientConfig.getBool("async");
    } catch (...) {
      smsc_log_warn(logger, "Parameter <PvssClient.async> missed. Defaul value is %d", async ? 1 : 0 );
    };

    PvssStreamClient* pc = new PvssStreamClient;

      // reading command-line arguments
      unsigned skip = unsigned(-1);
      for ( int i = 1; i < argc; ++i ) {
          const std::string arg = argv[i];
          if ( arg == "-s" || arg == "--speed" ) {
              if ( ++i >= argc ) return usage();
              speed = atoi(argv[i]);
              printf("speed specified %d\n",speed);
          } else if ( arg == "-k" || arg == "--skip" ) {
              if ( ++i >= argc ) return usage();
              skip = unsigned(atoi(argv[i]));
              printf("skip specified %u\n",skip);
          } else if ( arg == "-h" || arg == "--host" ) {
              if ( ++i >= argc ) return usage();
              host = argv[i];
              printf("host specified %s\n",host.c_str());
          } else if ( arg == "-p" || arg == "--port" ) {
              if ( ++i >= argc ) return usage();
              port = atoi(argv[i]);
              printf("port specified %u\n",port);
          } else {
              return usage(); 
          }
      }


    if ( skip == unsigned(-1) ) {
        scag2::util::Drndm::getRnd().setSeed(uint64_t(time(0)));
    }

    FlooderConfig flooderConfig;
    try {
        flooderConfig.setAsyncMode(true);
        flooderConfig.setSpeed(100);
        flooderConfig.setAddressesCount(1000000);
        flooderConfig.setCommands("s0g0");
        flooderConfig.setAddressFormat(".1.1.791%08u");
        flooderConfig.setOneCommandPerAbonent(false);
        flooderConfig.setMaxSpeed(false);
    } catch ( scag2::pvss::core::ConfigException& e ) {
        smsc_log_error(logger, "cannot set default value: %s", e.what() );
        fprintf(stderr,"cannot set default value: %s\n", e.what() );
        abort();
    }

    ::readFlooderConfig( logger, flooderConfig, manager );
                             
    lcClient = new PvssFlooder(PersClient::Instance(), flooderConfig, skip);

    pc->init( host.c_str(),
              port,
              timeout,
              pingtimeout,
              recontimeout,
              maxwait,
              connects,
              connPerThread,
              async );

    CallsCounter counter(lcClient, 5);

    lcClient->execute();
    counter.stop();
    smsc_log_info(logger, "stopping pers client...");
    pc->Stop();
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

