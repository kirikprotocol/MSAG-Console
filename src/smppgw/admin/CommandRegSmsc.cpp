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
#include "smppgw/smsc.hpp"
#include "smppgw/gwsme.hpp"
#include "util/Exception.hpp"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;

Response * CommandRegSmsc::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  smsc::logger::Logger *log=smsc::logger::Logger::getInstance("smppgw.admin");
  try {
      SmeManager* smeman =  (SmeManager*)(SmscApp->getSmeAdmin());
      GatewaySme *gwsme=new GatewaySme(smscConfig, smeman, altHost.c_str(), altPort);
      gwsme->setId(systemId.c_str(), smeman->lookup(systemId.c_str()));
      if(SmscApp->getGwSme(uid))
      {
          throw Exception("Duplicate gwsmeid %d!",(int)uid);
      }
      SmscApp->setGwSme(uid, gwsme);
      gwsme->setPrefix(uid);
      smeman->registerInternallSmeProxy(systemId.c_str(),gwsme);
      SmscApp->startTpTask(gwsme);
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
}
