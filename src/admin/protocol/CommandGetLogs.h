#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_LOGS
#define SMSC_ADMIN_PROTOCOL_COMMAND_GET_LOGS

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandGetLogs : public Command
{
public:
	CommandGetLogs(DOM_Document doc);

protected:

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_LOGS

