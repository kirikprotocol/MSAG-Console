// 
// File:   CommandAddSme.cpp
// Author: loomox
//
// Created on 30 марта 2005
//

#include "CommandAddSme.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;

Response * CommandAddSme::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  SmscApp->getSmeAdmin()->addSme(getSmeInfo());
  return new Response(Response::Ok, "none");
}


}
}
}
