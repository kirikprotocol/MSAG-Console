#ifndef SMSC_ADMIN_DAEMON_SERVICE
#define SMSC_ADMIN_DAEMON_SERVICE

#include <netinet/in.h>
#include <admin/AdminException.h>
#include <logger/Logger.h>
#include <util/cstrings.h>
#include "ServiceInfo.h"
#include "util/sleep.h"

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

  Service(const char * const services_dir,
    const ServiceInfo& svcInfo,
    const pid_t servicePID = 0,
    const run_status serviceStatus = stopped
    )
    : logger(Logger::getInstance("admdmn.Svc")),autostarted(false)
  {
    init(services_dir, svcInfo,servicePID, serviceStatus);
    lastStart=0;
    restartRetryCount=0;
  }

  pid_t start() throw (AdminException);
  void kill() throw (AdminException);
  void shutdown() throw (AdminException);

  const char * const getId() const {return info.id.c_str();}
  //const char * const getName() const {return name.get();}
  const pid_t getPid() const {return pid;}
  void setPid(const pid_t newPid) {pid = newPid; status = pid == 0 ? stopped : running;}
  const char * const getArgs() const {return info.args.c_str();}

  void setArgs(const char * const serviceArgs) throw (AdminException)
  {
    if (status != stopped)
    {
      throw AdminException("Changing service arguments not permitted: service is running");
    }
    info.args=serviceArgs;
  }

  void setHost(const char* host)
  {
    info.hostName=host;
  }
  const char* getHost()
  {
    return info.hostName.c_str();
  }

  const char * const getServiceDir()
  {
    return serviceDir.c_str();
  }

  void autostartNotify()
  {
    autostarted=true;
  }

  void autoDelay()
  {
    smsc_log_info(logger,"autodelay:%s,%d",autostarted?"true":"false",info.autostartDelay);
    if(autostarted && info.autostartDelay!=0)
    {
      millisleep(info.autostartDelay*1000);
      autostarted=false;
    }
  }

  ServiceInfo getInfo()const{return info;}

  run_status getStatus()const {return status;}
  void setStatus(run_status newStatus) { status = newStatus;}
  bool isAutostart()const {return info.autoStart;}

  ServiceInfo::ServiceType getType()const{return info.serviceType;}


  static const char* hostUp;
  static const char* hostDown;

protected:

  Service(const Service & copy);
  Service& operator = (const Service &copy);


  static const char * const service_exe;

  char ** createArguments();

  pid_t pid;
  std::string serviceDir;

  ServiceInfo info;
  Logger *logger;

  run_status status;
  time_t lastStart;
  int restartRetryCount;

  bool autostarted;

  void init(const char * const services_dir,
  const ServiceInfo& svcInfo,
  const pid_t servicePID = 0,
  const run_status serviceStatus = stopped
  );
};

}
}
}
#endif // ifndef SMSC_ADMIN_DAEMON_SERVICE
