#ifndef SMSC_ADMIN_SERVICE_SERVICE_ADMIN_INTERFACE_STARTER
#define SMSC_ADMIN_SERVICE_SERVICE_ADMIN_INTERFACE_STARTER

#include <netinet/in.h>
#include <admin/service/Component.h>
#include <admin/service/ServiceSocketListener.h>
#include <admin/AdminException.h>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;

class ServiceAdminInterfaceStarter
{
public:
/*	ServiceAdminInterfaceStarter()
	{
		listener = 0;
	}

	void addComponent(Component * component)
		throw (AdminException &)
	{
		ServiceCommandDispatcher::addComponent(component);
	}

	void start(const char * const host, in_port_t port)
		throw (AdminException &)
	{
		if (listener != 0)
		{
			throw AdminException("Service Admin Interface already started");
		}

		listener =
			new ServiceSocketListener(host, port,
																"smsc.admin.service.ServiceSocketListener");
		listener->Start();
	}

	void shutdown()
		throw (AdminException &)
	{
		if (listener == 0)
		{
			throw new AdminException("Service Admin Interface not started");
		}
		listener->shutdown();
	}

protected:
	ServiceSocketListener *listener;*/
};

}
}
}

#endif // ifndef SMSC_ADMIN_SERVICE_SERVICE_ADMIN_INTERFACE_STARTER
