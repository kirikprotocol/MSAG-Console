#ifndef SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER
#define SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER

#include <admin/util/SignalHandler.h>
#include <admin/service/ServiceSocketListener.h>

namespace smsc {
namespace admin {
namespace smsc_service {

using smsc::admin::service::ServiceSocketListener;
using smsc::admin::util::SignalHandler;

class SmscShutdownHandler : public SignalHandler
{
public:
	SmscShutdownHandler(ServiceSocketListener &socket_listener)
		: listener(socket_listener)
	{
	}

	virtual void handleSignal() throw ()
	{
		listener.shutdown();
	}

private:
	ServiceSocketListener &listener;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER

