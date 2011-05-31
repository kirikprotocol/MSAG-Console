//#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>

#include <admin/AdminException.h>
#include <admin/hsdaemon/DaemonSocketListener.h>
#include <admin/hsdaemon/ConfigParameterNames.h>
#include <util/config/ConfigException.h>
#include <util/config/Manager.h>
#include <logger/Logger.h>
#include <util/signal.hpp>
#include <util/xml/init.h>
#include <system/smscsignalhandlers.h>
#include "Interconnect.h"
#include "util/mirrorfile/mirrorfile.h"

using smsc::admin::AdminException;
using namespace smsc::admin::hsdaemon;

using namespace smsc::util;
using smsc::util::config::ConfigException;
using smsc::util::config::Manager;

namespace smsc {
namespace admin {
namespace hsdaemon {

void daemonInit()
{
  if (getppid() != 1)
  {
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
/*
    if (fork() != 0)
    {
      exit(0);
    } else {
      setsid();
    }
*/
  }
  struct rlimit flim;
  getrlimit(RLIMIT_NOFILE, &flim);
  for (rlim_t i=0; i<flim.rlim_cur; i++)
  {
    close((int)i);
  }
  /*close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);*/
}

}
}
}


void InitInterconnect(Manager& cfg)
{
  using namespace smsc::admin::hsdaemon;
  icon=new Interconnect(
    cfg.getString(CONFIG_ICON_HOST),
    cfg.getInt(CONFIG_ICON_PORT),
    cfg.getString(CONFIG_ICON_OTHERHOST),
    cfg.getInt(CONFIG_ICON_OTHERPORT)
  );
  icon->Init();
  icon->Start();
}

using smsc::admin::hsdaemon::daemonInit;
using smsc::admin::hsdaemon::DaemonCommandDispatcher;

static DaemonSocketListener * main_listener = 0;

void pipehandler(int a, siginfo_t* info, void* p)
{
  fprintf(stderr, "BROKEN PIPE SIGNAL RECEIVED, SKIPPED...\n");
  //abort();
}

void shutdown_handler(int a, siginfo_t* info, void* p)
{
  fprintf(stderr, "Shutdown signal %i received\n", a);
  if (main_listener != 0)
    main_listener->shutdown();
}

extern "C" void atExitHandler(void)
{
  smsc::util::xml::TerminateXerces();
  smsc::logger::Logger::Shutdown();
}

int main(int argc, char **argv)
{
  try
  {
    if (argc != 2)
    {
      std::cout << "Usage: smsc_admin_daemon <full name of config file>"
        << std::endl;
      return -1;
    }

    daemonInit();

    Logger::Init();
    atexit(atExitHandler);

    Manager::init(argv[1]);

    chdir(Manager::getInstance().getString(CONFIG_HOME_PARAMETER));

    Manager::deinit();
    Manager::init(argv[1]);
    Manager &manager = Manager::getInstance();
    FILE * redirected_stderr = freopen(manager.getString(CONFIG_STDERR_PARAMETER), "w", stderr);
    if (redirected_stderr == 0)
      throw ConfigException("Couldn't redirect stderr");

    smsc::logger::Logger *logger(Logger::getInstance("smsc.admin.daemon"));

    smsc_log_info(logger, "Starting...");

    InitInterconnect(manager);
    try{
      icon->sendSyncCommand();
    }catch(std::exception& e)
    {
      smsc_log_warn(logger,"sync failed:%s",e.what());
    }

    DaemonCommandDispatcher::init(&manager);
    DaemonSocketListener listener("smsc.admin.daemon.DaemonSocketListener");
    listener.init(manager.getString(CONFIG_HOST_PARAMETER),
      manager.getInt(CONFIG_PORT_PARAMETER));
    listener.Start();
    DaemonCommandDispatcher::startAllServices();

    smsc_log_info(logger, "Started");

    main_listener = &listener;
    setExtendedSignalHandler(smsc::system::SHUTDOWN_SIGNAL, shutdown_handler);
    setExtendedSignalHandler(SIGPIPE, pipehandler);

    listener.WaitFor();
    smsc_log_info(logger, "Stopped");
    fprintf(stderr, "Daemon stopped\n");
  }
  catch (ConfigException &e)
  {
    smsc_log_info(Logger::getInstance("smsc.admin.daemon"), "ConfigException: %s\n\r", e.what());
    exit(-1);
  }
  catch (AdminException &e)
  {
    smsc_log_info(Logger::getInstance("smsc.admin.daemon"), "AdminException: %s", e.what());
  }
  catch(std::exception& e)
  {
    smsc_log_info(Logger::getInstance("smsc.admin.daemon"), "Exception: %s",e.what());
  }
  catch (...)
  {
    smsc_log_info(Logger::getInstance("smsc.admin.daemon"), "Exception: <unknown>");
  }
}
