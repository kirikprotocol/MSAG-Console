#ifndef SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER
#define SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER

#include <admin/service/AdminSocketManager.h>
#include <admin/util/SignalHandler.h>

namespace smsc {
namespace admin {
namespace service {
namespace test {

using smsc::admin::service::AdminSocketManager;
using smsc::admin::util::SignalHandler;

class DumbServiceShutdownHandler : public SignalHandler
{
public:
	virtual void handleSignal() throw ()
	{
		AdminSocketManager::stop();
	}
};

}
}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER

