#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN
#define SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandShutdown : public Command
{
public:
	CommandShutdown(DOM_Document doc)
	{
		setId(shutdown_service);
		setData(doc);
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN

