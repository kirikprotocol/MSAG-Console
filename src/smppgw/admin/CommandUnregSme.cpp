// 
// File:   CommandRegSmsc.cpp
// Author: Vitaly
//
// Created on 06.04.05
//

#include "CommandUnregSme.h"

namespace smsc {
namespace smppgw {
namespace admin {

Response * CommandUnregSme::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{ 
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("CommandUnregSme");
  smsc_log_info(log, "1");
  SmscApp->unregisterSmeProxy(systemId);
  smsc_log_info(log, "2");
  return new Response(Response::Ok, "none");
  
}


}
}
}
