#include "Service.h"

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <admin/util/Shutdownable.h>
#include <admin/util/SignalHandler.h>
//#include <admin/util/ShutdownSignalListener.h>

//using smsc::admin::util::Shutdownable;
using smsc::admin::util::SignalHandler;

namespace smsc {
namespace admin {
namespace daemon {

pid_t Service::start()
	throw (AdminException)
{
	if (getStatus() != stopped)
	{
		throw AdminException("Service already running");
	}

	if (pid_t p = fork())
	{ // parent process
		pid = p;
	}
	else
	{	// child process
		char ** arguments = new (char*)[args.size()+4];
		arguments[args.size()+3] = 0;
		arguments[0] = command_line;
		arguments[1] = new char[sizeof(in_port_t)*3+1];
		snprintf(arguments[1], sizeof(in_port_t)*3+1, "%lu", (unsigned long) port);
		arguments[2] = config_file;
		for (size_t i=0; i<args.size(); i++)
		{
			arguments[i+3] = args[i];
		}
		#ifdef SMSC_DEBUG
			freopen("smsc_service.err", "a",  stderr);
		#endif
		execv(command_line, arguments);
	}
}

void Service::kill()
	throw (AdminException)
{
	if (getStatus() != running)
	{
		throw AdminException("Service is not running");
	}

	int result = sigsend(P_PID, pid, SIGKILL);
	if (result != 0)
	{
		switch (errno)
		{
		case EINVAL:
			throw AdminException("Incorrect signal number");
		case EPERM:
			throw AdminException("Does not have permission to send the signal to Service process");
		case ESRCH:
			pid = 0;
      throw AdminException("No process or process group can be found corresponding to that specified by pid");
		default:
			throw AdminException("Unknown error");
		}
	}
	pid = 0;
}

void Service::shutdown()
	throw (AdminException)
{
	if (getStatus() != running)
	{
		throw AdminException("Service is not running");
	}

	int result = sigsend(P_PID, pid, SignalHandler::SHUTDOWN_SIGNAL);
	if (result != 0)
	{
		switch (errno)
		{
		case EINVAL:
			throw AdminException("Incorrect signal number");
		case EPERM:
			throw AdminException("Does not have permission to send the signal to Service process");
		case ESRCH:
			pid = 0;
      throw AdminException("No process or process group can be found corresponding to that specified by pid");
		default:
			throw AdminException("Unknown error");
		}
	}
}

void Service::init(const char * const serviceName,
									 const char * const serviceCommandLine,
									 const char * const serviceconfigFileName,
									 const in_port_t serviceAdminPort,
									 const ServiceArguments &serviceArgs,
									 const pid_t servicePID = 0)
{
	name = cStringCopy(serviceName);
	command_line = cStringCopy(serviceCommandLine);
	config_file = cStringCopy(serviceconfigFileName);
	port = serviceAdminPort;
	pid = servicePID;
	args = serviceArgs;
/*	#ifdef SMSC_DEBUG
		Logger::getCategory("smsc.admin.daemon.Service").debug("Initialized service:");
		Logger::getCategory("smsc.admin.daemon.Service").debug("  name=%s", name == 0 ? "null" : name);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  command_line=%s", command_line  == 0 ? "null" : command_line);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  port=%lu", (unsigned long) port);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  pid=%lu", (unsigned long) pid);
	#endif
*/
}

void Service::deinit()
{
//	Logger::getCategory("smsc.admin.daemon.Service").debug("deinit");

	if (name != 0)
		delete name;
	name = 0;

	if (command_line != 0)
		delete command_line;
	command_line = 0;

	if (config_file != 0)
		delete config_file;
	config_file = 0;
	
	pid = 0;
	port = 0;

	for (size_t i=0; i<args.size(); i++)
	{
		delete[] args[i];
		args[i] = 0;
	}
	args.resize(0);
}

}
}
}
