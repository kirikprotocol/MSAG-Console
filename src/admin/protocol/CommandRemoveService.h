#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE

#include <admin/protocol/CommandService.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandRemoveService : public CommandService
{
public:
	CommandRemoveService(const DOMDocument *doc) throw ()
		: CommandService(remove_service, doc)
	{
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE

