#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_START_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_START_SERVICE

#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandStartService : public CommandService
{
public:
	CommandStartService(DOM_Document doc) throw ()
		: CommandService(start_service, doc)
	{
	}
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_START_SERVICE

