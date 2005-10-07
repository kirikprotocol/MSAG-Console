// 
// File:   CommandModifySmsc.cpp
// Author: Vitaly
//
// Created on 05.04.05
//

#include "CommandModifySmsc.h"
#include "util/xml/utilFunctions.h"
#include "util/Exception.hpp"


#include <exception.h>

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandModifySmsc::CreateResponse(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandModifySmsc is processing");
  try {

      if(!SmscApp->modifySmsc(smppEntityInfo))
         throw Exception("Parameters modify exception.");

      smsc_log_info(logger, "CommandModifySmsc is processed ok");
      return new Response(Response::Ok, "none");
  }catch(Exception& e){
     char msg[1024];
     sprintf(msg, "Failed to modify smsc. Details: %s", e.what());
     smsc_log_error(logger, msg);
     return new Response(Response::Error, msg);
  }catch(...){
      smsc_log_error(logger, "Failed to modify smsc. Unknown error.");
      return new Response(Response::Error, "Failed to modify smsc. Unknown error.");
  }
}


}
}
