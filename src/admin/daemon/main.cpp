//#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>

#include <admin/AdminException.h>
#include <admin/daemon/DaemonSocketListener.h>
#include <admin/daemon/DaemonCommandHandler.h>
#include <util/config/Manager.h>
#include <util/Logger.h>

using smsc::admin::AdminException;
using smsc::admin::daemon::DaemonSocketListener;
using smsc::admin::daemon::DaemonCommandHandler;
using smsc::util::Logger;
using smsc::util::config::Manager;

void daemonInit()
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
	chdir("/export/home/igork/build/bin/admin/daemon/");
	//chdir("/");
	openlog("SMSC Start/Stop daemon", LOG_PID | LOG_CONS, LOG_DAEMON);
	syslog(LOG_INFO, "Starting");
	closelog();
}

int main(int argc, char **argv)
{

	try
	{
		//daemonInit();
	}
	catch (...)
	{
		printf("Some exception in daemon init\n\r");
		exit(-1);
	}
	
	log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon"));
	
	try
	{
		logger.info("Start");
		Manager::init("daemon.xml");
		Manager manager = Manager::getInstance();
		int32_t port = manager.getInt("admin.daemon.port");
		DaemonCommandHandler handler;
		DaemonSocketListener listener(port, &handler,
																	"smsc.admin.daemon.DaemonSocketListener");
		listener.run();
		logger.info("Stop");
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

