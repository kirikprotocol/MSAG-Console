#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE

#include <admin/protocol/CommandService.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandRemoveService : public CommandService
{
public:
	CommandRemoveService(DOM_Document doc) : CommandService(doc)
	{
		setId(remove_service);
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_REMOVE_SERVICE

