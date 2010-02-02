//
// File:   SCAGCommandReader.cc
// Author: igork
//
// Created on 27 Май 2004 г., 16:49
//

#include "scag/admin/CommandIds.h"
#include "SCAGCommandReader2.h"
#include "SCAGAdminCommand2.h"
#include "SMSCAdminCommands2.h"
#include "SCAGRuleCommands2.h"

namespace scag2 {
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
  commandlist["updateSme"] = CommandIds::updateSme;
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
  commandlist["setLogCategories"] = CommandIds::setLogCategories;
  commandlist["storeLogCategories"] = CommandIds::storeLogConfig;

  commandlist["listSme"] = CommandIds::listSme;
  commandlist["listSmsc"] = CommandIds::listSmsc;
  commandlist["addMetaEntity"] = CommandIds::addMetaEntity;
  commandlist["updateMetaEntity"] = CommandIds::updateMetaEntity;
  commandlist["deleteMetaEntity"] = CommandIds::deleteMetaEntity;
  commandlist["addMetaEndpoint"] = CommandIds::addMetaEndpoint;
  commandlist["removeMetaEndpoint"] = CommandIds::removeMetaEndpoint;

    commandlist["replaceCounterActions"] = CommandIds::replaceCounterActions;
    commandlist["replaceCounterTemplate"] = CommandIds::replaceCounterTemplate;
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


    case CommandIds::updateSme: return new CommandUpdateSmeInfo(data);
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
    case CommandIds::setLogCategories: return new CommandSetLogCategories(data);
    case CommandIds::storeLogConfig: return new CommandStoreLogConfig(data);

    case CommandIds::listSme: return new CommandListSme(data);
    case CommandIds::listSmsc: return new CommandListSmsc(data);

    case CommandIds::addMetaEntity: return new CommandAddMetaEntity(data);
    case CommandIds::updateMetaEntity: return new CommandUpdateMetaEntity(data);
    case CommandIds::deleteMetaEntity:return new CommandDeleteMetaEntity(data);
    case CommandIds::addMetaEndpoint:return new CommandAddMetaEndpoint(data);
    case CommandIds::removeMetaEndpoint:return new CommandRemoveMetaEndpoint(data);

  case CommandIds::replaceCounterActions: return new CommandReplaceCounter(CommandIds::replaceCounterActions,data);
  case CommandIds::replaceCounterTemplate: return new CommandReplaceCounter(CommandIds::replaceCounterTemplate,data);

    default:
      smsc_log_warn(logger, "Unknown command id \"%i\"", id);
      throw AdminException("Unknown command");
  }
}


}
}
