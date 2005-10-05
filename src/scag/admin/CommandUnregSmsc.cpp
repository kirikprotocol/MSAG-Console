// 
// File:   CommandRegSmsc.cpp
// Author: Vitaly
//
// Created on 06.04.05
//

#include "CommandUnregSmsc.h"

namespace scag {
namespace admin {

Response * CommandUnregSmsc::CreateResponse(scag::Scag * SmscApp)
{ 
  smsc_log_info(logger, "CommandUnregSme is processing...");
  try {
      SmscApp->getSmppManagerAdmin()->updateSmppEntity(getSmppEntityInfo());
  }catch(Exception& e){
      smsc_log_info(logger, "CommandUnregSme exception, %s", e.what());
  }catch(...){
      smsc_log_info(logger, "CommandUnregSme exception, Unknown exception.");
  }
  smsc_log_info(logger, "CommandUnregSme is processed");
  return new Response(Response::Ok, "none");
  
}


}
}
