#include "AdminSocketManager.h"

namespace smsc {
namespace admin {
namespace service {

ServiceSocketListener * _AdminSocketManager::listener = 0;

void _AdminSocketManager::start(const char * const host,
															 const in_port_t port)
	throw (AdminException)
{
	if (listener != 0)
	{
		throw AdminException("Socket listener already started");
	}
	listener = new ServiceSocketListener("smsc.admin.service.SocketListener");
	listener->init(host, port);
	listener->Start();
}

void _AdminSocketManager::stop()
	throw ()
{
	try
	{
		if (listener != 0)
		{
			listener->shutdown();
			delete listener;
			listener = 0;
		}
	}
	catch (Exception &e)
	{
		__trace2__("smsc::admin::service::AdminSocketManager.stop: Exception \"%s\"\n", e.what());
	}
	catch (...)
	{
		__trace2__("smsc::admin::service::AdminSocketManager.stop: Exception...\n");
	}
}

}
}
}
