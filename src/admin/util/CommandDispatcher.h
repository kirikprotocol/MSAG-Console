#ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
#define SMSC_ADMIN_UTIL_COMMAND_DISPATCHER

#include <admin/AdminException.h>
#include <admin/protocol/CommandReader.h>
#include <admin/protocol/ResponseWriter.h>
#include <admin/protocol/Command.h>
#include <admin/protocol/Response.h>
#include <core/network/Socket.hpp>
#include <core/threads/ThreadedTask.hpp>

namespace smsc {
namespace admin {
namespace util {

using smsc::admin::AdminException;
using smsc::admin::protocol::CommandReader;
using smsc::admin::protocol::ResponseWriter;
using smsc::admin::protocol::Command;
using smsc::admin::protocol::Response;
using smsc::core::network::Socket;
using smsc::core::threads::ThreadedTask;

class CommandDispatcher : public ThreadedTask
{
public:
	CommandDispatcher(Socket * admSocket,
										//const char * const client_addr,
										const char * const loggerCatname = "smsc.admin.util.CommandDispatcher");
	virtual ~CommandDispatcher();
	virtual Response *handle(const Command * const command)
		throw (AdminException) = 0;

	virtual int Execute();
	virtual const char* taskName(){return task_name;}
	static void shutdown(){};

protected:
	Socket * sock;
	smsc::logger::Logger logger;
	const static char * const task_name;
	char cl_addr[16];
	CommandReader reader;
	ResponseWriter writer;

	void init();
};

}
}
}
#endif // ifndef SMSC_ADMIN_UTIL_COMMAND_DISPATCHER
