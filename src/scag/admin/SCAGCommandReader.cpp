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
    //applySmppRoutes|loadSmppTraceRoutes|traceSmppRoute|
  commandlist["applyConfig"] = CommandIds::applyConfig;

  commandlist["applySmppRoutes"] = CommandIds::applySmppRoutes;
  commandlist["loadSmppTraceRoutes"] = CommandIds::loadSmppTraceRoutes;
  commandlist["traceSmppRoute"] = CommandIds::traceSmppRoute;

  commandlist["addSme"] = CommandIds::addSme;
  commandlist["statusSme"] = CommandIds::statusSme;
  commandlist["deleteSme"] = CommandIds::deleteSme;
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

  commandlist["applyHttpRoutes"] = CommandIds::applyHttpRoutes;
  commandlist["loadHttpTraceRoutes"] = CommandIds::loadHttpTraceRoutes;
  commandlist["traceHttpRoute"] = CommandIds::traceHttpRoute;
  commandlist["getLogCategories"] = CommandIds::getLogCategories;
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
  if (!data) 
  {
      smsc_log_warn(logger, "Command document is invalid");
      throw AdminException("Command document is invalid");
  }

  switch (id)
  {
    case CommandIds::applyConfig: return new CommandApplyConfig(data);

    case CommandIds::applySmppRoutes: return new CommandApplySmppRoutes(data);
    case CommandIds::traceSmppRoute: return new CommandTraceSmppRoute(data);
    case CommandIds::loadSmppTraceRoutes: return new CommandLoadSmppTraceRoutes(data);


    case CommandIds::updateSmeInfo: return new CommandUpdateSmeInfo(data);
    case CommandIds::addSme: return new CommandAddSme(data);
    case CommandIds::statusSme: return new CommandStatusSme(data);
    case CommandIds::deleteSme: return new CommandDeleteSme(data);
    case CommandIds::addSmsc: return new CommandAddSmsc(data);
    case CommandIds::deleteSmsc: return new CommandDeleteSmsc(data);
    case CommandIds::updateSmsc: return new CommandUpdateSmsc(data);
    case CommandIds::updateRule: return new CommandUpdateRule(data);
    case CommandIds::removeRule: return new CommandRemoveRule(data);
    case CommandIds::addRule: return new CommandAddRule(data);

    case CommandIds::reloadOperators: return new CommandReloadOperators(data);
    case CommandIds::reloadServices: return new CommandReloadServices(data);
    case CommandIds::reloadTariffMatrix: return new CommandReloadTariffMatrix(data);
    case CommandIds::applyHttpRoutes: return new CommandApplyHttpRoutes(data);
    case CommandIds::loadHttpTraceRoutes: return new CommandLoadHttpTraceRoutes(data);
    case CommandIds::traceHttpRoute: return new CommandTraceHttpRoute(data);
    case CommandIds::getLogCategories: return new CommandGetLogCategories(data);

    default: 
      smsc_log_warn(logger, "Unknown command id \"%i\"", id);
      throw AdminException("Unknown command");
  }
}


}
}
