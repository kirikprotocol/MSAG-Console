#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN
#define SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandShutdown : public Command
{
public:
	CommandShutdown()
	{
		setId(shutdown);
		data = 0;
	}

protected:

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN

