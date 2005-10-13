// 
// File:   CommandAddSme.cpp
// Author: loomox
//
// Created on 30 марта 2005
//

#include "SCAGCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "scag/scag.h"
#include "scag/transport/smpp/SmppManagerAdmin.h"

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandAddSme::CreateResponse(scag::Scag * ScagApp)
{
  try {
      if(!ScagApp)
          Exception("Scag undefined");

      smsc_log_info(logger, "CommandAddSme is processing...");
      scag::transport::smpp::SmppManagerAdmin * smppMan = ScagApp->getSmppManagerAdmin();

      if(!smppMan)
          throw Exception("SmppManager undefined");

      smppMan->addSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandAddSme is processed ok");
      return new Response(Response::Ok, "CommandAddSme is processed ok");
  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to add new SME. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "CommandAddSme exception, Unknown exception.");
      return new Response(Response::Error, "Failed to add new SME. Unknown error");
  }
}


}
}
