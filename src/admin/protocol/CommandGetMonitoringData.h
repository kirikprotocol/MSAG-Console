#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_MONITORING_DATA
#define SMSC_ADMIN_PROTOCOL_COMMAND_GET_MONITORING_DATA

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandGetMonitoringData : public Command
{
public:
	CommandGetMonitoringData(DOM_Document doc);

protected:

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_GET_MONITORING_DATA

