//
// File:   CommandUpdateSmeInfo.cc
// Author: igork
//
// Created on 20 Август 2004 г., 15:42
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

}
}
}

