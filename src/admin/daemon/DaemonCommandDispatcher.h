#ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
#define SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER

#include <admin/AdminException.h>
#include <admin/daemon/DaemonCommandHandler.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <admin/util/CommandDispatcher.h>
#include <admin/util/Shutdownable.h>

namespace smsc {
namespace admin {
namespace daemon {

using smsc::admin::AdminException;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::Response;
using smsc::admin::util::CommandDispatcher;
using smsc::admin::util::Shutdownable;

class DaemonCommandDispatcher : public CommandDispatcher
{
public:
	DaemonCommandDispatcher(Shutdownable * parentListener,
													int admSocket,
													const char * const client_addr,
													DaemonCommandHandler * commandHandler)
		: CommandDispatcher(parentListener, admSocket, client_addr, "smsc.admin.daemon.CommandDispatcher")
	{
		handler = commandHandler;
	}

	Response * handle(const Command * const command) throw (AdminException &);

protected:
	DaemonCommandHandler * handler;

	Response * start_service (const Command * const command) throw (AdminException &);
	Response * kill_service  (const Command * const command) throw (AdminException &);
	Response * add_service   (const Command * const command) throw (AdminException &);
	Response * remove_service(const Command * const command) throw (AdminException &);
	Response * list_services (const Command * const command) throw (AdminException &);
};

}
}
}

#endif // ifndef SMSC_ADMIN_DAEMON_DAEMON_COMMAND_DISPATCHER
