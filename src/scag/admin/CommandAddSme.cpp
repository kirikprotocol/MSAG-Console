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
  //SmscApp->getSmeAdmin()->addSme(getSmeInfo());
  return new Response(Response::Ok, "none");
}


}
}
