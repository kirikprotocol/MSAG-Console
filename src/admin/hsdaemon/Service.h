#ifndef SMSC_ADMIN_DAEMON_SERVICE
#define SMSC_ADMIN_DAEMON_SERVICE

#include <netinet/in.h>
#include <admin/AdminException.h>
#include <logger/Logger.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace hsdaemon {

using smsc::util::cStringCopy;
using smsc::admin::AdminException;
using smsc::logger::Logger;

//typedef std::vector<char*> ServiceArguments;

class Service
{
public:
  enum run_status { stopped, starting, running, stopping };
  enum service_type { failover, standalone };

  Service(const char * const services_dir,
    const char * const serviceId,
    const char * const serviceArgs,
    const service_type serviceType,
    const bool autostartFlag = true,
    const pid_t servicePID = 0,
    const run_status serviceStatus = stopped
    )
    : logger(Logger::getInstance("admdmn.Svc")), autoStart(autostartFlag)
  {
    init(services_dir, serviceId, serviceArgs, serviceType,servicePID, serviceStatus);
  }

  Service()
    : logger(Logger::getInstance("admdmn.Svc")), autoStart(true)
  {
    init(0, 0, 0, failover, 0,stopped);
  }

  Service(const Service & copy)
    : logger(Logger::getInstance("admdmn.Svc")), autoStart(copy.autoStart)
  {
    init(copy.serviceDir.c_str(), copy.id.c_str(), copy.args.c_str(),copy.svcType, copy.pid);
  }

  pid_t start() throw (AdminException);
  void kill() throw (AdminException);
  void shutdown() throw (AdminException);

  const char * const getId() const {return id.c_str();}
  //const char * const getName() const {return name.get();}
  const pid_t getPid() const {return pid;}
  void setPid(const pid_t newPid) {pid = newPid; status = pid == 0 ? stopped : running;}
  const char * const getArgs() const {return args.c_str();}

  Service &operator = (const Service &copy)
  {
    init(copy.serviceDir.c_str(), copy.id.c_str(), copy.args.c_str(), copy.svcType, copy.pid, copy.status);
    return *this;
  }

  void setArgs(const char * const serviceArgs) throw (AdminException)
  {
    if (status != stopped)
    {
      throw AdminException("Changing service arguments not permitted: service is running");
    }
    args=serviceArgs;
  }

  void setHost(const char* host)
  {
    hostName=host;
  }
  const char* getHost()
  {
    return hostName.c_str();
  }

  const char * const getServiceDir()
  {
    return serviceDir.c_str();
  }

  run_status getStatus() {return status;}
  void setStatus(run_status newStatus) { status = newStatus;}
  bool isAutostart() {return autoStart;}


  static const char* hostUp;
  static const char* hostDown;

protected:

  static const char * const service_exe;

  char ** createArguments();

  pid_t pid;
  std::string id;
  std::string args;
  std::string serviceDir;
  std::string hostName;
  bool autoStart;
  Logger *logger;

  run_status status;
  service_type svcType;

  void init(const char * const services_dir,
  const char * const serviceId,
  const char * const serviceArgs,
  const service_type serviceType,
  const pid_t servicePID = 0,
  const run_status serviceStatus = stopped
  );
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
