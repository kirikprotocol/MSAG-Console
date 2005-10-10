// 
// File:   CommandRegSmsc.cpp
// Author: Vitaly
//
// Created on 06.04.05
//

#include "CommandUnregSmsc.h"

namespace scag {
namespace admin {

Response * CommandUnregSmsc::CreateResponse(scag::Scag * ScagApp)
{ 
  smsc_log_info(logger, "CommandUnregSme is processing...");
  try {
      if(!ScagApp)
          Exception("Scag undefined");

      ScagApp->getSmppManagerAdmin()->updateSmppEntity(getSmppEntityInfo());

      smsc_log_info(logger, "CommandUnregSme is processed");
      return new Response(Response::Ok, "none");

  }catch(Exception& e){
      char msg[1024];
      sprintf(msg, "Failed to unregister smsc. Details: %s", e.what());
      smsc_log_error(logger, msg);
      return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to unregister smsc. Unknown error");
      return new Response(Response::Error, "Failed to unregister smsc. Unknown error");
  }
  smsc_log_error(logger, "Failed to unregister smsc. Unknown error");
  return new Response(Response::Error, "Failed to unregister smsc. Unknown error");
  
}


}
}
