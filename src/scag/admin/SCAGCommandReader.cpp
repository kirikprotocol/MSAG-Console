// 
// File:   SCAGCommandReader.cc
// Author: igork
//
// Created on 27 ��� 2004 �., 16:49
//

#include "SCAGCommandReader.h"
#include "CommandIds.h"

#include "SCAGAdminCommand.h"
#include "SMSCAdminCommands.h"
#include "SCAGRuleCommands.h"

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
  commandlist["addSmsc"] = CommandIds::addSmsc;
  commandlist["deleteSmsc"] = CommandIds::deleteSmsc;
  commandlist["updateSmsc"] = CommandIds::updateSmsc;

  commandlist["updateRule"] = CommandIds::updateRule;
  commandlist["removeRule"] = CommandIds::removeRule;
  commandlist["addRule"] = CommandIds::addRule;

  commandlist["reloadOperators"] = CommandIds::reloadOperators;
  commandlist["reloadServices"] = CommandIds::reloadServices;
  commandlist["reloadTariffMatrix"] = CommandIds::reloadTariffMatrix;

  commandlist["reloadHttpRoutes"] = CommandIds::reloadHttpRoutes;
  commandlist["reloadHttpTraceRoutes"] = CommandIds::reloadHttpTraceRoutes;
  commandlist["traceHttpRoute"] = CommandIds::traceHttpRoute;
}

/*
    addSme
    statusSme
    apply
    deleteSme
    loadRoutes
    traceRoute
    updateSmeInfo
    addSmsc
    deleteSmsc
    updateSmsc
    addRule
    updateRule
    removeRule
    reloadOperators
    reloadServices
    reloadTariffMatrix
    
*/

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
  if (!data) 
  {
      smsc_log_warn(logger, "Command document is invalid");
      throw AdminException("Command document is invalid");
  }

  switch (id)
  {
    case CommandIds::apply: return new CommandApply(data);
    case CommandIds::updateSmeInfo: return new CommandUpdateSmeInfo(data);
    case CommandIds::addSme: return new CommandAddSme(data);
    case CommandIds::statusSme: return new CommandStatusSme(data);
    case CommandIds::deleteSme: return new CommandDeleteSme(data);
    case CommandIds::traceRoute: return new CommandTraceRoute(data);
    case CommandIds::loadRoutes: return new CommandLoadRoutes(data);
    case CommandIds::addSmsc: return new CommandAddSmsc(data);
    case CommandIds::deleteSmsc: return new CommandDeleteSmsc(data);
    case CommandIds::updateSmsc: return new CommandUpdateSmsc(data);
    case CommandIds::updateRule: return new CommandUpdateRule(data);
    case CommandIds::removeRule: return new CommandRemoveRule(data);
    case CommandIds::addRule: return new CommandAddRule(data);

    case CommandIds::reloadOperators: return new CommandReloadOperators(data);
    case CommandIds::reloadServices: return new CommandReloadServices(data);
    case CommandIds::reloadTariffMatrix: return new CommandReloadTariffMatrix(data);
    case CommandIds::reloadHttpRoutes: return new CommandReloadHttpRoutes(data);
    case CommandIds::reloadHttpTraceRoutes: return new CommandReloadHttpTraceRoutes(data);
    case CommandIds::traceHttpRoute: return new CommandTraceHttpRoute(data);

    default: 
      smsc_log_warn(logger, "Unknown command id \"%i\"", id);
      throw AdminException("Unknown command");
  }
}


}
}
