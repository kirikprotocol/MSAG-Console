#ifndef	SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER
#define SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

#include <iostream>
#include <log4cpp/Category.hh>

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

using smsc::admin::AdminException;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::CommandCall;
using smsc::admin::protocol::CommandListComponents;
using smsc::admin::protocol::Response;
using smsc::core::network::Socket;
using smsc::core::synchronization::Mutex;
using smsc::core::synchronization::MutexGuard;

class ServiceCommandDispatcher : public smsc::admin::util::CommandDispatcher
{
public:
	ServiceCommandDispatcher(Socket * admSocket)
		: CommandDispatcher(admSocket,
												//client_addr,
												"smsc.admin.service.ServiceCommandDispatcher")
	{
	}

	virtual Response * handle(const Command * const command) throw (AdminException);
protected:
	Response * call(const CommandCall * const command) throw (AdminException);
	Response * listComponents(const CommandListComponents * const command) throw (AdminException);
};

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

