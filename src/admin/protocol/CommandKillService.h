#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE

#include <admin/protocol/CommandService.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandKillService : public CommandService
{
public:
	CommandKillService(DOM_Document doc) : CommandService(doc)
	{
		setId(kill_service);
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE


