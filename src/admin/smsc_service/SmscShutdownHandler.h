#ifndef SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER
#define SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER

#include <admin/service/AdminSocketManager.h>
#include <admin/smsc_service/SmscComponent.h>
#include <admin/util/SignalHandler.h>

namespace smsc {
namespace admin {
namespace smsc_service {

using smsc::admin::service;
using smsc::admin::util::SignalHandler;
using smsc::admin::service::AdminSocketManager;

class SmscShutdownHandler : public SignalHandler
{
public:
	SmscShutdownHandler(SmscComponent & component)
		: smsc_component(component)
	{
	}

	virtual void handleSignal() throw ()
	{
		AdminSocketManager::stop();
		//smsc_component.stop();
	}

private:
	SmscComponent & smsc_component;
};

}
}
}
#endif // ifndef SMSC_ADMIN_SMSC_SMSCSHUTDOWNHANDLER

