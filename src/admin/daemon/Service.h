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
  enum run_status { stopped, starting, running, stopping };

	Service(const char * const services_dir,
					const char * const serviceId,
					//const char * const serviceName,
					const in_port_t serviceAdminPort,
					const char * const serviceArgs,
					const pid_t servicePID = 0,
          const run_status serviceStatus = stopped)
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{
		init(services_dir, serviceId, serviceAdminPort, serviceArgs, servicePID, serviceStatus);
	}

	Service()
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{	
		init(0, 0, 0, 0, 0, stopped);
	}

	Service(const Service & copy)
		: logger(Logger::getCategory("smsc.admin.daemon.Service"))
	{
		init(copy.service_dir.get(), copy.id.get(), copy.port, copy.args.get(), copy.pid);
	}

	pid_t start() throw (AdminException);
	void kill() throw (AdminException);
	void shutdown() throw (AdminException);

	const char * const getId() const {return id.get();}
	//const char * const getName() const {return name.get();}
	const pid_t getPid() const {return pid;}
	void setPid(const pid_t newPid) {pid = newPid; status = pid == 0 ? stopped : running;}
	const char * const getArgs() const {return args.get();}
	const in_port_t getPort() const {return port;}

	Service &operator = (Service &copy)
	{
		init(copy.service_dir.get(), copy.id.get(), copy.port, copy.args.get(), copy.pid, copy.status);
		return *this;
	}

  void setPort(const in_port_t serviceAdminPort) throw (AdminException)
	{
		if (status != stopped)
		{
			throw AdminException("Changing service port not permitted: service is running");
		}
		port = serviceAdminPort;
	}

	void setArgs(const char * const serviceArgs) throw (AdminException)
	{
		if (status != stopped)
		{
			throw AdminException("Changing service arguments not permitted: service is running");
		}
		args.reset(cStringCopy(serviceArgs));
	}

  const char * const getServiceDir()
  {
    return service_dir.get();
  }

  run_status getStatus() {return status;}
  void setStatus(run_status newStatus) { status = newStatus;}


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

  run_status status;

	void init(const char * const services_dir,
						const char * const serviceId,
						//const char * const serviceName,
						const in_port_t serviceAdminPort,
						const char * const serviceArgs,
						const pid_t servicePID = 0,
            const run_status serviceStatus = stopped);
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
