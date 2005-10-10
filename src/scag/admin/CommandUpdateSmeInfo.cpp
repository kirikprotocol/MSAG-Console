//
// File:   CommandUpdateSmeInfo.cc
// Author: igork
//
// Created on 20 á×ÇÕÓÔ 2004 Ç., 15:42
//

#include "CommandUpdateSmeInfo.h"
#include "CommandIds.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

namespace scag {
namespace admin {

//
// Constructor
///
CommandUpdateSmeInfo::CommandUpdateSmeInfo(const xercesc::DOMDocument * const document)
  : Abstract_CommandSmeInfo((Command::Id)CommandIds::updateSmeInfo, document)
{
  smsc_log_info(logger, "UpdateSmeInfo command");
}

Response * CommandUpdateSmeInfo::CreateResponse(scag::Scag * ScagApp)
{
  try {
      smsc_log_info(logger, "CommandUpdateSmeInfo is processing...");

      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!ScagApp)
          Exception("Scag undefined");

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->updateSmppEntity(getSmppEntityInfo());  

      smsc_log_info(logger, "CommandUpdateSmeInfo processed ok.");
      return new Response(Response::Ok, "CommandUpdateSmeInfo processed ok.");
  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to update sme. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to update sme. Unknown error.");
      return new Response(Response::Error, "Failed to update sme. Unknown error.");
  }
  return new Response(Response::Error, "Failed to update sme. Unknown error.");
}

}
}

