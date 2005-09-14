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

// Its during for a test only
//#include "scag/scag.h"

#include "scag/gwsme.hpp"
#include "util/Exception.hpp"

#include <exception.h>

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandRegSmsc::CreateResponse(scag::Scag * SmscApp)
{
  smsc::logger::Logger *log = smsc::logger::Logger::getInstance("CommandRegSmsc");
  try {      

      smsc::sme::SmeConfig gwcfg;
      gwcfg.host = smscConfig.host;
      gwcfg.port = smscConfig.port;
      gwcfg.sid = smscConfig.sid;
      gwcfg.password = smscConfig.password;
      gwcfg.smppTimeOut = smscConfig.smppTimeOut;

      //if(!SmscApp->regSmsc(gwcfg, altHost, altPort, systemId, uid))
      //   throw Exception("Duplicate gwsmeid %d!",(int)uid);
      
      return new Response(Response::Ok, "none");
  }catch(exception& e){
     __trace2__("CommandRegSmsc::CreateResponse exception:%s",e.what());
     smsc_log_warn(log, "CommandRegSmsc::CreateResponse exception:%s",e.what());
     return new Response(Response::Error, "CommandRegSmsc::CreateRespnse exception");
  }catch(...){
      __trace__("CommandRegSmsc::CreateResponse exception:unknown");
      smsc_log_warn(log, "CommandRegSmsc::CreateResponse exception:unknown");
      return new Response(Response::Error, "CommandRegSmsc::CreateRespnse exception");
  }
}


}
}
