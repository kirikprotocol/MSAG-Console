#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE

#include <admin/protocol/CommandService.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandKillService : public CommandService
{
public:
	CommandKillService(DOM_Document doc) throw ()
		: CommandService(kill_service, doc)
	{
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_KILL_SERVICE


