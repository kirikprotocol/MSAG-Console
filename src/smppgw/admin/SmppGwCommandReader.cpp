// 
// File:   SmppGwCommandReader.cc
// Author: igork
//
// Created on 27 Май 2004 г., 16:49
//

#include "SmppGwCommandReader.h"
#include "CommandIds.h"
#include "CommandApply.h"
#include "CommandUpdateSmeInfo.h"
#include "CommandAddSme.h"
#include "CommandDeleteSme.h"

namespace smsc {
namespace smppgw {
namespace admin {

SmppGwCommandReader::SmppGwCommandReader(Socket * admSocket)
  : CommandReader(admSocket)
{
}

SmppGwCommandReader::~SmppGwCommandReader()
{
}

int SmppGwCommandReader::getCommandIdByName(const char * const command_name)
{
  if (strcmp("apply", command_name) == 0)
    return (Command::Id)CommandIds::apply;
  if (strcmp("updateSmeInfo", command_name) == 0)
    return (Command::Id)CommandIds::updateSmeInfo;
  if (strcmp("addSme", command_name) == 0)
    return (Command::Id)CommandIds::addSme;
  if (strcmp("deleteSme", command_name) == 0)
    return (Command::Id)CommandIds::deleteSme;
  
  return (Command::Id)CommandIds::unknown;
}

Command * SmppGwCommandReader::createCommand(int id, const DOMDocument *data)
{
  switch (id)
  {
    case CommandIds::apply: return new CommandApply(data);
    case CommandIds::updateSmeInfo: return new CommandUpdateSmeInfo(data);
    case CommandIds::addSme: return new CommandAddSme(data);
    case CommandIds::deleteSme: return new CommandDeleteSme(data);
    default: 
      smsc_log_warn(logger, "Unknown command id \"%i\"", id);
      throw AdminException("Unknown command");
  }
}


}
}
}
