#ifndef	SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER
#define SMSC_ADMIN_SERVICE_COMMAND_DISPATCHER

#include <iostream>
#include <log4cpp/Category.hh>

#include <admin/protocol/Command.h>
//#include <admin/protocol/CommandReader.h>
#include <admin/protocol/Response.h>
//#include <admin/protocol/ResponseWriter.h>
#include <admin/service/ServiceCommandHandler.h>
#include <admin/util/CommandDispatcher.h>
#include <admin/util/Shutdownable.h>
#include <admin/AdminException.h>
//#include <core/threads/ThreadedTask.hpp>

namespace smsc {
namespace admin {
namespace service {

using smsc::admin::AdminException;
using smsc::admin::service::ServiceCommandHandler;
using smsc::admin::protocol::Command;
//using smsc::admin::protocol::CommandReader;
using smsc::admin::protocol::Response;
//using smsc::admin::protocol::ResponseWriter;
using smsc::admin::util::Shutdownable;

class CommandDispatcher : public smsc::admin::util::CommandDispatcher
{
public:
	CommandDispatcher(Shutdownable * parentListener,
										int admSocket,
										const char * const client_addr,
										ServiceCommandHandler * commandHandler)
		: smsc::admin::util::CommandDispatcher(parentListener,
																					 admSocket,
																					 client_addr,
																					 "smsc.admin.service.CommandDispatcher")
	{
		handler = commandHandler;
	}

	Response * handle(const Command * const command) throw (AdminException &);

protected:
	ServiceCommandHandler * handler;
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

