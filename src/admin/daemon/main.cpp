//#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>

#include <admin/AdminException.h>
#include <admin/daemon/DaemonSocketListener.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/util/SignalHandler.h>
#include <util/config/Manager.h>
#include <util/Logger.h>
#include <util/signal.h>

using smsc::admin::AdminException;
using smsc::admin::daemon::DaemonSocketListener;
using smsc::admin::service::AdminSocketManager;
using smsc::admin::util::SignalHandler;
using smsc::util::Logger;
using smsc::util::setExtendedSignalHandler;
using smsc::util::config::Manager;

namespace smsc {
namespace admin {
namespace daemon {

void daemonInit(const char * const home)
{
	if (getppid() != 1)
	{
		signal(SIGTSTP, SIG_IGN);
		signal(SIGTTIN, SIG_IGN);
		signal(SIGTTOU, SIG_IGN);
		if (fork() != 0)
		{
			exit(0);
		} else {
			setsid();
		}
	}
	struct rlimit flim;
	getrlimit(RLIMIT_NOFILE, &flim);
	for (int i=0; i<flim.rlim_max; i++)
	{
		close(i);
	}
	chdir(home);
/*
	openlog("SMSC Start/Stop daemon", LOG_PID | LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Starting");
	closelog();
*/
}

}
}
}

using smsc::admin::daemon::daemonInit;
using smsc::admin::daemon::DaemonCommandDispatcher;

int main(int argc, char **argv)
{
/*	if (argc != 2)
	{
		std::cout << "Usage: smsc_admin_daemon <path to home directory>"
			<< std::endl;
		return -1;
	}

	try
	{
		daemonInit(argv[1]);
	}
	catch (...)
	{
		printf("Some exception in daemon init\n\r");
		exit(-1);
	}
*/	
	log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon"));
	
	try
	{
		logger.info("Start");
		Manager::init("daemon.xml");
		Manager manager = Manager::getInstance();
/*		AdminSocketManager::start(manager.getString("admin.daemon.host"),
															port,
															"smsc.admin.daemon.DaemonSocketListener");
*/		
		DaemonSocketListener listener("smsc.admin.daemon.DaemonSocketListener");
		listener.init(manager.getString("admin.daemon.host"),
									manager.getInt("admin.daemon.port"));
		listener.Start();

		DaemonCommandDispatcher::activateChildSignalHandler();
		logger.info("Started");
		listener.WaitFor();
		logger.info("Stopped");
	}
	catch (AdminException &e)
	{
		logger.info("EXCEPTION: %s", e.what());
	}
	catch (...)
	{
		logger.info("EXCEPTION: <unknown>");
	}
}

