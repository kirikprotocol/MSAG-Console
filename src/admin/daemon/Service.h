#ifndef SMSC_ADMIN_DAEMON_SERVICE
#define SMSC_ADMIN_DAEMON_SERVICE

#include <netinet/in.h>
#include <admin/AdminException.h>
#include <logger/Logger.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::util::cStringCopy;
using smsc::admin::AdminException;
using smsc::logger::Logger;

//typedef std::vector<char*> ServiceArguments;

class Service
{
public:
  enum run_status { stopped, starting, running, stopping };

	Service(const char * const services_dir,
		const char * const serviceId,
		const char * const serviceArgs,
		const bool autostartFlag = true,
		const pid_t servicePID = 0,
		const run_status serviceStatus = stopped)
		: logger(Logger::getInstance("smsc.admin.daemon.Service")), autostart(autostartFlag)
	{
		init(services_dir, serviceId, serviceArgs, servicePID, serviceStatus);
	}

	Service()
		: logger(Logger::getInstance("smsc.admin.daemon.Service")), autostart(true)
	{
		init(0, 0, 0, 0, stopped);
	}

	Service(const Service & copy)
		: logger(Logger::getInstance("smsc.admin.daemon.Service")), autostart(copy.autostart)
	{
		init(copy.service_dir.get(), copy.id.get(), copy.args.get(), copy.pid);
	}

	pid_t start() throw (AdminException);
	void kill() throw (AdminException);
	void shutdown() throw (AdminException);

	const char * const getId() const {return id.get();}
	//const char * const getName() const {return name.get();}
	const pid_t getPid() const {return pid;}
	void setPid(const pid_t newPid) {pid = newPid; status = pid == 0 ? stopped : running;}
	const char * const getArgs() const {return args.get();}

	Service &operator = (Service &copy)
	{
		init(copy.service_dir.get(), copy.id.get(), copy.args.get(), copy.pid, copy.status);
		return *this;
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
  bool isAutostart() {return autostart;}


protected:
	char ** createArguments();
	std::auto_ptr<char> id;
	pid_t pid;
	std::auto_ptr<char> args;
	static const char * const service_exe;
	std::auto_ptr<char> service_dir;
  Logger *logger;
	bool autostart;

	run_status status;

	void init(const char * const services_dir,
		const char * const serviceId,
		const char * const serviceArgs,
		const pid_t servicePID = 0,
		const run_status serviceStatus = stopped);
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
