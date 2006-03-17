#ifndef SMSC_ADMIN_PROTOCOL_COMMAND
#define SMSC_ADMIN_PROTOCOL_COMMAND

#include <inttypes.h>
#include <xercesc/dom/DOM.hpp>

#include "admin/AdminException.h"
#include "logger/Logger.h"

using smsc::admin::AdminException;
using smsc::logger::Logger;

namespace smsc {
namespace admin {
namespace protocol {

//class smsc::admin::protocol::CommandReader;

class Command {
//  friend class smsc::admin::protocol::CommandReader;

public:
  enum Id
  {
    undefined,
/*    get_config,
    set_config,
    get_logs,
    get_monitoring,*/
    start_service,
    shutdown_service,
    kill_service,
    add_service,
    remove_service,
    list_services,
    list_components,
    call,
    set_service_startup_parameters
  };

  Command(Id newId) throw ()
    : logger(Logger::getInstance("smsc.admin.protocol.Command"))
  {
    id = newId;
  }

  Command(const char * const commandName) throw ()
    : logger(Logger::getInstance("smsc.admin.protocol.Command"))
  {
    id = getCommandIdByName(commandName);
  }

  virtual ~Command()
  {
    id = undefined;
  }

  virtual void init() {}

  const Id getId() const  throw ()             {return id;}
  const char * const getName() const  throw () {return names[id].name;}

  static Id getCommandIdByName(const char * const name) throw ();
protected:
  const xercesc::DOMDocument * getData() const {return data;}
  struct _Command {
    const char * name;
    Id id;
  };

  Id id;
  const xercesc::DOMDocument * data;
  smsc::logger::Logger *logger;
  static const uint8_t commands_quantity = 10;
  static const _Command names[commands_quantity];

  void setId(Id newId) throw () {id = newId;}
  void setData(const xercesc::DOMDocument * newData) throw () {data = newData;}

private:
};

}
}
}

#endif // ifndef SMSC_ADMIN_PROTOCOL_COMMAND
