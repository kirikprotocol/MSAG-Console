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

Response * CommandRegSmsc::CreateResponse(scag::Scag * SmscApp)
{
  smsc_log_info(logger, "CommandModifySmsc is processing...");
  try {      

      if(!SmscApp->regSmsc(smppEntityInfo))
         throw Exception("Duplicate gwsmeid");

      smsc_log_info(logger, "CommandModifySmsc is processed ok");
      return new Response(Response::Ok, "none");
  }catch(exception& e){
     __trace2__("CommandRegSmsc::CreateResponse exception:%s",e.what());
     smsc_log_warn(logger, "CommandRegSmsc::CreateResponse exception:%s",e.what());
     return new Response(Response::Error, "CommandRegSmsc::CreateRespnse exception");
  }catch(...){
      __trace__("CommandRegSmsc::CreateResponse exception:unknown");
      smsc_log_warn(logger, "CommandRegSmsc::CreateResponse exception:unknown");
      return new Response(Response::Error, "CommandRegSmsc::CreateRespnse exception");
  }
}


}
}                                                       
