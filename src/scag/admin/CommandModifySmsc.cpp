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
  }catch(exception& e){
     __trace2__("CommandModifySmsc::CreateResponse exception:%s",e.what());
     smsc_log_info(logger, "CommandModifySmsc::CreateResponse exception:%s",e.what());
     return new Response(Response::Error, "CommandModifySmsc::CreateRespnse exception");
  }catch(...){
      __trace__("CommandModifySmsc::CreateResponse exception:unknown");
      smsc_log_info(logger, "CommandModifySmsc::CreateResponse exception:unknown");
      return new Response(Response::Error, "CommandModifySmsc::CreateRespnse exception");
  }
}


}
}
