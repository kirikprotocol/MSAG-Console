//#include <stdio.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/resource.h>

#include <admin/AdminException.h>
#include <admin/daemon/DaemonSocketListener.h>
#include <admin/service/AdminSocketManager.h>
#include <admin/util/SignalHandler.h>
#include <util/config/ConfigException.h>
#include <util/config/Manager.h>
#include <util/Logger.h>
#include <util/signal.h>

using smsc::admin::AdminException;
using smsc::admin::daemon::DaemonSocketListener;
using smsc::admin::service::AdminSocketManager;
using smsc::admin::util::SignalHandler;
using smsc::util::Logger;
using smsc::util::setExtendedSignalHandler;
using smsc::util::config::ConfigException;
using smsc::util::config::Manager;

namespace smsc {
namespace admin {
namespace daemon {

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
	for (rlim_t i=0; i<flim.rlim_max; i++)
	{
		close(i);
	}
	/*close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);*/
}

}
}
}

using smsc::admin::daemon::daemonInit;
using smsc::admin::daemon::DaemonCommandDispatcher;

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
		//daemonInit();
	
		Manager::init(argv[1]);
		Manager &manager = Manager::getInstance();

		chdir(manager.getString("admin.daemon.home"));
		FILE * redirected_stderr = freopen(manager.getString("admin.daemon.stderr"), "w", stderr);
		if (redirected_stderr == 0)
			throw ConfigException("Couldn't redirect stderr");

		Logger::Init(manager.getString("admin.daemon.logger.configFile"));
		log4cpp::Category &logger(Logger::getCategory("smsc.admin.daemon"));
	
		logger.info("Starting...");
		
		DaemonCommandDispatcher::init(&manager);
		DaemonSocketListener listener("smsc.admin.daemon.DaemonSocketListener");
		listener.init(manager.getString("admin.daemon.host"),
									manager.getInt("admin.daemon.port"));
		listener.Start();

		DaemonCommandDispatcher::activateChildSignalHandler();
		logger.info("Started");
		listener.WaitFor();
		logger.info("Stopped");
	}
	catch (ConfigException &e)
	{
		Logger::getCategory("smsc.admin.daemon").info("ConfigException: %s\n\r", e.what());
		exit(-1);
	}
	catch (AdminException &e)
	{
		Logger::getCategory("smsc.admin.daemon").info("AdminException: %s", e.what());
	}
	catch (...)
	{
		Logger::getCategory("smsc.admin.daemon").info("Exception: <unknown>");
	}
}

