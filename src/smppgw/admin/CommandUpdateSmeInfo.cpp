//
// File:   CommandUpdateSmeInfo.cc
// Author: igork
//
// Created on 20 á×ÇÕÓÔ 2004 Ç., 15:42
//

#include "CommandUpdateSmeInfo.h"
#include "CommandIds.h"

namespace smsc {
namespace smppgw {
namespace admin {

//
// Constructor
///
CommandUpdateSmeInfo::CommandUpdateSmeInfo(const xercesc::DOMDocument * const document)
  : Abstract_CommandSmeInfo((Command::Id)CommandIds::updateSmeInfo, document)
{
  smsc_log_debug(logger, "UpdateSmeInfo command");
}

Response * CommandUpdateSmeInfo::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  SmscApp->getSmeAdmin()->updateSmeInfo(getSmeInfo().systemId, getSmeInfo());
  return new Response(Response::Ok, "none");
}


}
}
}

