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

//typedef std::vector<char*> ServiceArguments;

class Service
{
public:
	enum Status {stopped, running};

	Service(const char * const serviceName,
					const char * const serviceCommandLine,
          const char * const serviceconfigFileName,
					const in_port_t serviceAdminPort,
					const char * const serviceArgs,
					const pid_t servicePID = 0)
	{
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(...)");
		init(serviceName, serviceCommandLine, serviceconfigFileName, serviceAdminPort, serviceArgs,
				 servicePID);
	}

	Service()
	{	
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(0)");
		init(0, 0, 0, 0, 0, 0);
	}

	Service(const Service & copy)
	{
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(copy)");
		init(copy.name, copy.command_line, copy.config_file, copy.port, copy.args, copy.pid);
	}

	~Service()
	{
//		Logger::getCategory("smsc.admin.daemon.Service").debug("~Service");
		deinit();
	}

	pid_t start() throw (AdminException);
	void kill() throw (AdminException);
	void shutdown() throw (AdminException);

	const char * const getName() const {return name;}
	const char * const getCommandLine() const {return command_line;}
	const char * const getConfigFileName() const {return config_file;}
	const pid_t getPid() const {return pid;}
	void setPid(const pid_t newPid) {pid = newPid;}
	const Status getStatus() const {return pid == 0 ? stopped : running;}
	const char * const getArgs() const {return args;}
	const in_port_t getPort() const {return port;}

	Service &operator = (Service &copy)
	{
		init(copy.name, copy.command_line, copy.config_file, copy.port, copy.args, copy.pid);
		return *this;
	}

protected:
	char ** createArguments();
	char * name;
	char * command_line;
	char * config_file;
	pid_t pid;
	char * args;
	in_port_t port;

	void init(const char * const serviceName,
					const char * const serviceCommandLine,
          const char * const serviceconfigFileName,
					const in_port_t serviceAdminPort,
					const char * const serviceArgs,
					const pid_t servicePID = 0);
	void deinit();
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
