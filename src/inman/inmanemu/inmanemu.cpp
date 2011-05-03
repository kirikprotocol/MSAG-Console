#include <stdio.h>
#include <signal.h>
#include <string>

#include <util/Exception.hpp>
#include "BillingServer.h"
#include "InmanemuConf.h"


using namespace inmanemu::server;
using inmanemu::util::InmanemuConfig;

BillingServer * billingServer = 0;
Logger * logger = 0;

extern "C" void appSignalHandler(int sig)
{
    if (sig==SIGTERM || sig==SIGINT)
    {
        if (billingServer) billingServer->Stop();

        delete billingServer;
        billingServer = 0;
        //smsc::logger::Logger::Shutdown();
    }
}

extern "C" void atExitHandler(void)
{
}



int main(int argc,char* argv[])
{
    atexit(atExitHandler);

    sigset_t set;
    sigfillset(&set);
        sigdelset(&set, SIGTERM);
        sigdelset(&set, SIGINT);
        sigdelset(&set, SIGSEGV);
        sigdelset(&set, SIGBUS);
    sigprocmask(SIG_SETMASK, &set, NULL);
        sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);


  smsc::logger::Logger::Init();
  logger = Logger::getInstance("inmanemu");

  billingServer = new BillingServer();
  if (!billingServer)
  {
      smsc_log_error(logger, "Cannot init Inmanemu server. Details: cannot create BillingServer instance");
      return 0;
  }

  InmanemuConfig cfg;

  try
  {
      cfg.Init();
      billingServer->Init(cfg.host, cfg.port, cfg.cdr_dir);
  } catch (const smsc::util::Exception& e)
  {
      smsc_log_error(logger, "Cannot init Inmanemu server. Details: ", e.what());
      
      delete billingServer;
      return 0;
  }
  

  billingServer->Run();
  smsc_log_info(logger, "Server stoped");


  delete billingServer;
  billingServer = 0;
  return 0;
}
