#include "Service.h"

#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

namespace smsc {
namespace admin {
namespace daemon {

pid_t Service::start()
	throw (AdminException &)
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
		execv(command_line, args);
	}
}

void Service::kill()
	throw (AdminException &)
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
      throw AdminException("No process or process group can be found corresponding to that specified by pid");
		default:
			throw AdminException("Unknown error");
		}
	}
	pid = 0;
}

void Service::init(const char * const serviceName,
									 const char * const serviceCommandLine,
									 in_port_t serviceAdminPort,
									 const char * const * const serviceArgs,
									 pid_t servicePID = 0)
{
	name = cStringCopy(serviceName);
	command_line = cStringCopy(serviceCommandLine);
	port = serviceAdminPort;
	pid = servicePID;
	args = new (char*)[2];
	args[0] = cStringCopy(command_line);
	args[1] = 0;
	#ifdef SMSC_DEBUG
		Logger::getCategory("smsc.admin.daemon.Service").debug("Initialized service:");
		Logger::getCategory("smsc.admin.daemon.Service").debug("  name=%s", name == 0 ? "null" : name);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  command_line=%s", command_line  == 0 ? "null" : command_line);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  port=%lu", (unsigned long) port);
		Logger::getCategory("smsc.admin.daemon.Service").debug("  pid=%lu", (unsigned long) pid);
	#endif
}

void Service::deinit()
{
	Logger::getCategory("smsc.admin.daemon.Service").debug("deinit");

	if (name != 0)
		delete name;
	name = 0;

	if (command_line != 0)
		delete command_line;
	command_line = 0;

	pid = 0;
	port = 0;

	if (args != 0)
	{
		for (int i=0; args[i] != 0; i++)
		{
			delete[] args[i];
			args[i] = 0;
		}
		delete[] args;
		args = 0;
	}
}

}
}
}
