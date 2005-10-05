// 
// File:   CommandAddSme.cpp
// Author: loomox
//
// Created on 30 марта 2005
//

#include "CommandAddSme.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "scag/scag.h"

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandAddSme::CreateResponse(scag::Scag * SmscApp)
{
  try {
      smsc_log_info(logger, "CommandAddSme is processing...");
      SmscApp->getSmppManagerAdmin()->addSmppEntity(getSmppEntityInfo());
  }catch(Exception& e){
      smsc_log_info(logger, "CommandAddSme exception, %s", e.what());
  }catch(...){
      smsc_log_info(logger, "CommandAddSme exception, Unknown exception.");
  }
  smsc_log_info(logger, "CommandAddSme is processed ok");
  return new Response(Response::Ok, "none");
}


}
}
