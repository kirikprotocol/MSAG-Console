#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_CONFIG
#define SMSC_ADMIN_PROTOCOL_COMMAND_GET_CONFIG

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandGetConfig : public Command
{
public:
	CommandGetConfig(DOM_Document doc);

protected:

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_CONFIG
