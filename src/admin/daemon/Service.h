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
using log4cpp::Category;

//typedef std::vector<char*> ServiceArguments;

class Service
{
public:
	Service(const char * const services_dir,
					const char * const serviceId,
					//const char * const serviceName,
					const in_port_t serviceAdminPort,
					const char * const serviceArgs,
					const pid_t servicePID = 0)
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(...)");
		init(services_dir, serviceId, /*serviceName, */serviceAdminPort, serviceArgs, servicePID);
	}

	Service()
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{	
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(0)");
		init(0, 0, /*0, */0, 0, 0);
	}

	Service(const Service & copy)
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{
//		Logger::getCategory("smsc.admin.daemon.Service").debug("Service(copy)");
		init(copy.service_dir.get(), copy.id.get(), /*copy.name.get(), */copy.port, copy.args.get(), copy.pid);
	}

	pid_t start() throw (AdminException);
	void kill() throw (AdminException);
	void shutdown() throw (AdminException);

	const char * const getId() const {return id.get();}
	//const char * const getName() const {return name.get();}
	const pid_t getPid() const {return pid;}
	void setPid(const pid_t newPid) {pid = newPid;}
	const bool isRunning() const {return pid != 0;}
	const char * const getArgs() const {return args.get();}
	const in_port_t getPort() const {return port;}

	Service &operator = (Service &copy)
	{
		init(copy.service_dir.get(), copy.id.get(), /*copy.name.get(), */copy.port, copy.args.get(), copy.pid);
		return *this;
	}

	/*void setName(const char * const serviceName) throw (AdminException)
	{
		if (isRunning())
		{
			throw AdminException("Changing service name not permitted: service is running");
		}
		name.reset(cStringCopy(serviceName));
	}*/

	void setPort(const in_port_t serviceAdminPort) throw (AdminException)
	{
		if (isRunning())
		{
			throw AdminException("Changing service port not permitted: service is running");
		}
		port = serviceAdminPort;
	}

	void setArgs(const char * const serviceArgs) throw (AdminException)
	{
		if (isRunning())
		{
			throw AdminException("Changing service arguments not permitted: service is running");
		}
		args.reset(cStringCopy(serviceArgs));
	}

  const char * const getServiceDir()
  {
    return service_dir.get();
  }


protected:
	char ** createArguments();
	std::auto_ptr<char> id;
	//std::auto_ptr<char> name;
	pid_t pid;
	std::auto_ptr<char> args;
	in_port_t port;
	static const char * const service_exe;
	std::auto_ptr<char> service_dir;
	Category &logger;

	void init(const char * const services_dir,
						const char * const serviceId,
						//const char * const serviceName,
						const in_port_t serviceAdminPort,
						const char * const serviceArgs,
						const pid_t servicePID = 0);
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
