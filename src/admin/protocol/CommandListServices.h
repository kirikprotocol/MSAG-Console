#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_LIST_SERVICES
#define SMSC_ADMIN_PROTOCOL_COMMAND_LIST_SERVICES

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::Command;

class CommandListServices : public Command
{
public:
	CommandListServices(const DOMDocument *doc) throw ()
		: Command(list_services)
	{
		setData(doc);
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_LIST_SERVICES

