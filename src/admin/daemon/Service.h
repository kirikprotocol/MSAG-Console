#ifndef SMSC_ADMIN_DAEMON_SERVICE
#define SMSC_ADMIN_DAEMON_SERVICE

#include <netinet/in.h>
#include <admin/AdminException.h>
#include <util/Logger.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::util::cStringCopy;
using smsc::admin::AdminException;
using smsc::util::Logger;

class Service
{
public:
	enum Status {stopped, running};

	Service(const char * const serviceName,
					const char * const serviceCommandLine,
					in_port_t serviceAdminPort,
					const char * const * const serviceArgs,
					pid_t servicePID = 0)
	{
		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(...)");
		init(serviceName, serviceCommandLine, serviceAdminPort, serviceArgs,
				 servicePID);
	}

	Service()
	{	
		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(0)");
		init(0, 0, 0, 0, 0);
	}

	Service(const Service & copy)
	{
		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(copy)");
		init(copy.name, copy.command_line, copy.port, copy.args, copy.pid);
	}

	~Service()
	{
		Logger::getCategory("smsc.admin.daemon.Service").debug("~Service");
		deinit();
	}

	pid_t start() throw (AdminException &);
	void kill() throw (AdminException &);

	const char * const getName() const {return name;}
	const char * const getCommandLine() const {return command_line;}
	const pid_t getPid() const {return pid;}
	const Status getStatus() const {return pid == 0 ? stopped : running;}
	const char * const * const getArgs() const {return args;}
	const in_port_t getPort() const {return port;}

	Service &operator = (Service &copy)
	{
		init(copy.name, copy.command_line, copy.port, copy.args, copy.pid);
		return *this;
	}

protected:
	char * name;
	char * command_line;
	pid_t pid;
	char ** args;
	in_port_t port;

	void init(const char * const serviceName,
					const char * const serviceCommandLine,
					in_port_t serviceAdminPort,
					const char * const * const serviceArgs,
					pid_t servicePID = 0);
	void deinit();
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
