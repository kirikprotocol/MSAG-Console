#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_LIST_COMPONENTS
#define SMSC_ADMIN_PROTOCOL_COMMAND_LIST_COMPONENTS

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::protocol::Command;

class CommandListComponents : public CommandService
{
public:
	CommandListComponents(DOM_Document doc) throw ()
		: CommandService(list_components, doc)
	{
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_LIST_COMPONENTS

