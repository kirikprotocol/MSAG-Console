#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SWITCHOVER
#define SMSC_ADMIN_PROTOCOL_COMMAND_SWITCHOVER

#include <admin/protocol/Command.h>

namespace smsc {
namespace admin {
namespace protocol {

class CommandSwitchOver : public CommandService
{
public:
  CommandSwitchOver(const DOMDocument *doc) throw ()
    : CommandService(switchover_service, doc)
  {
  }
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SHUTDOWN
