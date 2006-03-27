#ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_HSSERVICE_STARTUP_PARAMETERS
#define SMSC_ADMIN_PROTOCOL_COMMAND_SET_HSSERVICE_STARTUP_PARAMETERS

#include "admin/hsdaemon/ServiceInfo.h"

namespace smsc {
namespace admin {
namespace protocol {

class CommandSetHSServiceStartupParameters : public CommandService
{
public:
  CommandSetHSServiceStartupParameters(const DOMDocument *doc)  throw (AdminException)
    : CommandService(set_service_startup_parameters, doc)
  {
    smsc_log_debug(logger, "Set service startup parameters command");
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

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND_SET_SERVICE_STARTUP_PARAMETERS
