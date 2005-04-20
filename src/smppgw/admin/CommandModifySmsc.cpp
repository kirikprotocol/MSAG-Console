// 
// File:   CommandModifySmsc.cpp
// Author: Vitaly
//
// Created on 05.04.05
//

#include "CommandModifySmsc.h"
#include "util/xml/utilFunctions.h"
#include "util/Exception.hpp"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;

Response * CommandModifySmsc::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("CommandModifySmsc");
  try {
      if(!SmscApp->modifySmsc(smscConfig, altHost, altPort, systemId, uid))
         throw Exception("Parameters modify exception.");
      
      return new Response(Response::Ok, "none");
  }catch(exception& e){
     __trace2__("CommandModifySmsc::CreateResponse exception:%s",e.what());
     smsc_log_warn(log, "CommandModifySmsc::CreateResponse exception:%s",e.what());
     return new Response(Response::Error, "CommandModifySmsc::CreateRespnse exception");
  }catch(...){
      __trace__("CommandModifySmsc::CreateResponse exception:unknown");
      smsc_log_warn(log, "CommandModifySmsc::CreateResponse exception:unknown");
      return new Response(Response::Error, "CommandModifySmsc::CreateRespnse exception");
  }
}


}
}
}
