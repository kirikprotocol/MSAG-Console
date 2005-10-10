// 
// File:   CommandRegSmsc.cpp
// Author: Vitaly
//
// Created on 05.04.05
//

#include "CommandRegSmsc.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "core/threads/ThreadPool.hpp"

#include "util/Exception.hpp"
#include "scag/transport/smpp/SmppManagerAdmin.h"

#include <exception.h>

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandRegSmsc::CreateResponse(scag::Scag * ScagApp)
{
  smsc_log_info(logger, "CommandModifySmsc is processing...");
  try {

      if(!ScagApp)
          Exception("Scag undefined");

      if(!ScagApp->regSmsc(smppEntityInfo))
         throw Exception("Duplicate gwsmeid");

      smsc_log_info(logger, "CommandModifySmsc is processed ok");
      return new Response(Response::Ok, "none");
  }catch(exception& e){
      char msg[1024];
      sprintf(msg, "Failed to register smsc. Details: %s", e.what());

     smsc_log_warn(logger, msg);
     return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_warn(logger, "Failed to register smsc. Unknown error.");
      return new Response(Response::Error, "Failed to register smsc. Unknown error.");
  }
}


}
}                                                       
