// 
// File:   CommandRegSmsc.cpp
// Author: Vitaly
//
// Created on 06.04.05
//

#include "CommandUnregSme.h"

namespace scag {
namespace admin {

Response * CommandUnregSme::CreateResponse(scag::Scag * SmscApp)
{ 
  //SmscApp->unregisterSmeProxy(systemId);
  return new Response(Response::Ok, "none");
  
}


}
}
