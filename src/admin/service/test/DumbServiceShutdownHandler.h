#ifndef SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER
#define SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER

#include <admin/service/ServiceSocketListener.h>
#include <admin/util/SignalHandler.h>

namespace smsc {
namespace admin {
namespace service {
namespace test {

using namespace smsc::admin::util;

class DumbServiceShutdownHandler : public SignalHandler
{
public:
  DumbServiceShutdownHandler(ServiceSocketListener &socket_listener)
    : listener(socket_listener)
  {
  }

	virtual void handleSignal() throw ()
	{
		listener.shutdown();
	}

protected:
  ServiceSocketListener &listener;
};

}
}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_TEST_DUMB_SERVICE_SHUTDOWN_HANDLER

