#include "Service.h"

#include <sys/types.h>
#include <sys/stat.h>
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

const char * const Service::service_exe = "bin/service";

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
		chdir(service_dir.get());
		chmod(service_exe, S_IRWXU);
		#ifdef SMSC_DEBUG
			freopen("smsc_service.err", "a",  stderr);
		#endif
		execv(service_exe, createArguments());
		logger.error("Couldn't start service (\"%s/%s\"), nested: %u: %s",
								 service_dir.get(), service_exe, errno, strerror(errno));
		exit(-1);
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

void Service::init(const char * const services_dir,
									 const char * const serviceId,
									 const char * const serviceName,
									 const in_port_t serviceAdminPort,
									 const char * const serviceArgs,
									 const pid_t servicePID = 0)
{
	service_dir.reset(new char[strlen(services_dir) + 1 + strlen(serviceId) + 1]);
	strcpy(service_dir.get(), services_dir);
	strcat(service_dir.get(), "/");
	strcat(service_dir.get(), serviceId);

	id.reset(cStringCopy(serviceId));
	name.reset(cStringCopy(serviceName));
	port = serviceAdminPort;
	pid = servicePID;
	args.reset(cStringCopy(serviceArgs));
}

char * substr(const char * from, const char * to)
{
	char *tmpbuf = new char[to - from +1];
	memcpy(tmpbuf, from, to - from);
	tmpbuf[to - from] = 0;
	return tmpbuf;
}

char ** Service::createArguments()
{
	char * port_str = new char[sizeof(in_port_t)*3+1];
	snprintf(port_str, sizeof(in_port_t)*3+1, "%lu", (unsigned long) port);
	
	typedef std::vector<char *> chrvec;
	chrvec args_vector;
	args_vector.push_back(cStringCopy(service_exe));
	args_vector.push_back(port_str);

	///////////////////////////////////////
	// parse arguments
	///////////////////////////////////////
	{
		const char *p1 = args.get(), *p2 = args.get();
		while (*p1!= 0)
		{
			if (isspace(*p1))
			{
				if (p1 == p2)
				{
					p2++;
				} else {
					args_vector.push_back(substr(p2, p1));
					p2 = p1+1;
				}
			}
			p1++;
		}
		if (*p2 != 0 && p2 != p1)
			args_vector.push_back(substr(p2, p1));
	}

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

