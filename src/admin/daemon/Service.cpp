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
		return pid = p;
	}
	else
	{	// child process
		#ifdef SMSC_DEBUG
			freopen("smsc_service.err", "a",  stderr);
		#endif
		execv(command_line, createArguments());
		return 0;
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
									 const char * const serviceArgs,
									 const pid_t servicePID = 0)
{
	name = cStringCopy(serviceName);
	command_line = cStringCopy(serviceCommandLine);
	config_file = cStringCopy(serviceconfigFileName);
	port = serviceAdminPort;
	pid = servicePID;
	args = cStringCopy(serviceArgs);
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

	if (args != 0)
		delete[] args;
}

char ** Service::createArguments()
{
	char * port_str = new char[sizeof(in_port_t)*3+1];
	snprintf(port_str, sizeof(in_port_t)*3+1, "%lu", (unsigned long) port);
	
	typedef std::vector<char *> chrvec;
	chrvec args_vector;
	args_vector.push_back(command_line);
	args_vector.push_back(port_str);
	args_vector.push_back(config_file);

	///////////////////////////////////////
	// parse arguments
	///////////////////////////////////////
	args_vector.push_back(args);

	char ** arguments = new (char*)[args_vector.size()+1];
	for (chrvec::size_type i = 0; i < args_vector.size(); i++)
	{
		arguments[i] = args_vector.at(i);
	}
	return arguments;
}

}
}
}

