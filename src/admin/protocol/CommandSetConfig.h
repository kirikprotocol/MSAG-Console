#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_CONFIG
#define SMSC_ADMIN_PROTOCOL_COMMAND_SET_CONFIG

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandSetConfig : public Command
{
public:
	CommandSetConfig(DOM_Document doc);

protected:

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_CONFIG

