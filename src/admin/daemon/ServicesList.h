#ifndef SMSC_ADMIN_DAEMON_SERVICES_LIST
#define SMSC_ADMIN_DAEMON_SERVICES_LIST

#include <admin/AdminException.h>
#include <admin/daemon/Service.h>
#include <core/buffers/Hash.hpp>
#include <util/cstrings.h>
#include <util/Logger.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::AdminException;
using smsc::core::buffers::Hash;
using smsc::util::cStringCopy;
using smsc::util::Logger;

/**
 * Thread-safe list of services.
 */
class ServicesList
{
public:
	ServicesList()
	{}

	void add(Service *service) throw (AdminException &);
	void remove(const char * const serviceName) throw (AdminException &);
	Service * get(const char * const serviceName);
	char * getText() const;

	Service* operator[](const char * const serviceName)
		throw (AdminException &)
	{
		return get(serviceName);
	}

	void markServiceAsStopped(pid_t old_pid);

protected:
	typedef Hash<Service*> _ServiceList;
	_ServiceList services;
};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_SERVICES_LIST

