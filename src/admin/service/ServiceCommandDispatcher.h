#ifndef	SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER
#define SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

#include <iostream>

#include <admin/protocol/Command.h>
#include <admin/protocol/CommandCall.h>
#include <admin/protocol/CommandListComponents.h>
#include <admin/protocol/Response.h>
#include <admin/service/Component.h>
#include <admin/util/CommandDispatcher.h>
#include <admin/AdminException.h>
#include <core/network/Socket.hpp>
#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::core::network::Socket;
using namespace smsc::admin;
using namespace smsc::admin::protocol;
using namespace smsc::admin::util;
using namespace smsc::core::synchronization;

class ServiceCommandDispatcher : public CommandDispatcher
{
public:
	ServiceCommandDispatcher(Socket * admSocket) : CommandDispatcher(admSocket, "smsc.admin.service.ServiceCommandDispatcher")
	{}

	virtual Response * handle(const Command * const command) throw (AdminException);
protected:
	Response * call(const CommandCall * const command) throw (AdminException);
	Response * listComponents(const CommandListComponents * const command) throw (AdminException);
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

