#ifndef	SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER
#define SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

#include <iostream>
#include <log4cpp/Category.hh>

#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <admin/service/Component.h>
#include <admin/util/CommandDispatcher.h>
#include <admin/util/Shutdownable.h>
#include <admin/AdminException.h>
#include <core/network/Socket.hpp>
#include <core/synchronization/Mutex.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::Response;
using smsc::admin::util::Shutdownable;
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

	virtual void shutdown();

	Response * handle(const Command * const command) throw (AdminException &);
};

/*class CommandDispatcher : public smsc::core::threads::ThreadedTask
{
public:
	CommandDispatcher(ServiceSocketListener * parentListener,
										int admSocket,
										const char * const client_addr,
										ServiceCommandHandler * commandHandler);
	virtual ~CommandDispatcher();
	virtual int Execute();
	virtual const char* taskName(){return task_name;}

protected:
	void init();

private:
	int sock;
	log4cpp::Category &logger;
	const static char * const task_name;
	char cl_addr[16];
	CommandReader reader;
	ResponseWriter writer;
	ServiceCommandHandler *handler;
	ServiceSocketListener * parent;
};*/

}
}
}
#endif // ifndef SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

