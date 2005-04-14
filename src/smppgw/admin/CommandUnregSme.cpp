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
  SmscApp->unregisterSmeProxy(systemId);
  return new Response(Response::Ok, "none");
  
}


}
}
}
