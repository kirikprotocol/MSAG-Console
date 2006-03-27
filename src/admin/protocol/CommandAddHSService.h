#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
#define SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE

#include "admin/hsdaemon/Service.h"
#include <admin/AdminException.h>
#include <admin/protocol/CommandService.h>
#include <util/xml/utilFunctions.h>
#include <admin/daemon/Service.h>
#include <util/cstrings.h>

namespace smsc {
namespace admin {
namespace protocol {

using smsc::admin::AdminException;
using smsc::util::xml::getNodeAttribute;
using smsc::util::xml::getNodeText;

typedef std::pair<int, char *> CmdArgument;

class CommandAddHSService : public CommandService
{
public:
  CommandAddHSService(const DOMDocument *doc) throw (AdminException)
    : CommandService(add_hsservice, doc)
  {
    smsc_log_debug(logger, "Add service command");
    info.InitFromXml(doc);
  }

  const smsc::admin::hsdaemon::ServiceInfo& getServiceInfo()const
  {
    return info;
  }

protected:
  smsc::admin::hsdaemon::ServiceInfo info;
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_ADD_SERVICE
