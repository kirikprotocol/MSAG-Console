#ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
#define SMSC_ADMIN_UTIL_COMMAND_DISPATCHER

#include <log4cpp/Category.hh>
#include <admin/AdminException.h>
#include <admin/protocol/CommandReader.h>
#include <admin/protocol/ResponseWriter.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <admin/util/Shutdownable.h>
#include <core/threads/ThreadedTask.hpp>

namespace smsc {
namespace admin {
namespace util {

using smsc::admin::AdminException;
using smsc::admin::protocol::CommandReader;
using smsc::admin::protocol::ResponseWriter;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::Response;

class CommandDispatcher : public smsc::core::threads::ThreadedTask
{
public:
	CommandDispatcher(Shutdownable * parentListener,
										int admSocket,
										const char * const client_addr,
										const char * const loggerCategory = "smsc.admin.util.CommandDispatcher");
	virtual ~CommandDispatcher();
	virtual Response *handle(const Command * const command)  throw (AdminException &) = 0;
	virtual int Execute();
	virtual const char* taskName(){return task_name;}

protected:
	int sock;
	log4cpp::Category &logger;
	const static char * const task_name;
	char cl_addr[16];
	CommandReader reader;
	ResponseWriter writer;
	Shutdownable * parent;

	void init();
};

}
}
}
#endif // ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
