#ifndef SMSC_ADMIN_DAEMON_SERVICES_LIST
#define SMSC_ADMIN_DAEMON_SERVICES_LIST

#include <admin/AdminException.h>
#include <admin/hsdaemon/Service.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>
#include <logger/Logger.h>

namespace smsc {
namespace admin {
namespace hsdaemon {

using smsc::admin::AdminException;
using smsc::core::buffers::Hash;
using smsc::util::cStringCopy;
using smsc::logger::Logger;

/**
 * Thread-safe list of services.
 */
class ServicesList
{
public:
  ServicesList()
  {}

  void add(Service *service) throw (AdminException);
  void remove(const char * const serviceId) throw (AdminException);
  Service * get(const char * const serviceId);
  char * getText() const;

  Service* operator[](const char * const serviceId)
    throw (AdminException)
  {
    return get(serviceId);
  }

  const char * const markServiceAsStopped(pid_t old_pid);

  bool isServiceExists(const char* svcName)
  {
    return services.Exists(svcName);
  }

  void First() {
    services.First();
  }

  int Next(char *& serviceId, Service *& servicePtr)
  {
    return services.Next(serviceId, servicePtr);
  }

protected:
  typedef Hash<Service*> _ServiceList;

  _ServiceList services;
};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_SERVICES_LIST
