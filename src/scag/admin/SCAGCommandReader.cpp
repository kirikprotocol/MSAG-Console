// 
// File:   SCAGCommandReader.cc
// Author: igork
//
// Created on 27 Май 2004 г., 16:49
//

#include "SCAGCommandReader.h"
#include "CommandIds.h"
#include "CommandApply.h"
#include "CommandUpdateSmeInfo.h"
#include "CommandAddSme.h"
#include "CommandStatusSme.h"
#include "CommandRegSmsc.h"
#include "CommandUnregSme.h"
#include "CommandDeleteSme.h"
#include "CommandTraceRoute.h"
#include "CommandLoadRoutes.h"
#include "CommandModifySmsc.h"
#include "CommandUpdateRule.h"
#include "CommandRemoveRule.h"

namespace scag {
namespace admin {

SCAGCommandReader::SCAGCommandReader(Socket * admSocket)
  : CommandReader(admSocket)
{
  commandlist["addSme"] = CommandIds::addSme;
  commandlist["statusSme"] = CommandIds::statusSme;
  commandlist["apply"] = CommandIds::apply;
  commandlist["deleteSme"] = CommandIds::deleteSme;
  commandlist["loadRoutes"] = CommandIds::loadRoutes;
  commandlist["traceRoute"] = CommandIds::traceRoute;
  commandlist["updateSmeInfo"] = CommandIds::updateSmeInfo;
  commandlist["regSmsc"] = CommandIds::regSmsc;
  commandlist["unregSme"] = CommandIds::unregSme;
  commandlist["modifySmsc"] = CommandIds::modifySmsc;
  commandlist["updateRule"] = CommandIds::updateRule;
  commandlist["removeRule"] = CommandIds::removeRule;
}

SCAGCommandReader::~SCAGCommandReader()
{
}

int SCAGCommandReader::getCommandIdByName(const char * const command_name)
{
  fprintf(stderr,"---- Command received %s\n",command_name);

  if (commandlist.Exists(command_name)) {
      return commandlist[command_name];
  }
  return (Command::Id)CommandIds::unknown;
}

Command * SCAGCommandReader::createCommand(int id, const DOMDocument *data)
{
  fprintf(stderr,"---- Create command \n");

  switch (id)
  {
    case CommandIds::apply: return new CommandApply(data);
    case CommandIds::updateSmeInfo: return new CommandUpdateSmeInfo(data);
    case CommandIds::addSme: return new CommandAddSme(data);
    case CommandIds::statusSme: return new CommandStatusSme(data);
    case CommandIds::deleteSme: return new CommandDeleteSme(data);
    case CommandIds::traceRoute: return new CommandTraceRoute(data);
    case CommandIds::loadRoutes: return new CommandLoadRoutes(data);
    case CommandIds::regSmsc: return new CommandRegSmsc(data);
    case CommandIds::unregSme: return new CommandUnregSme(data);
    case CommandIds::modifySmsc: return new CommandModifySmsc(data);
    case CommandIds::updateRule: return new CommandUpdateRule(data);
    case CommandIds::removeRule: return new CommandRemoveRule(data);

    default: 
      smsc_log_warn(logger, "Unknown command id \"%i\"", id);
      throw AdminException("Unknown command");
  }
}


}
}
