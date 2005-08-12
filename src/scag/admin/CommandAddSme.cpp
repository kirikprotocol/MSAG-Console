// 
// File:   CommandAddSme.cpp
// Author: loomox
//
// Created on 30 ����� 2005
//

#include "CommandAddSme.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"

namespace scag {
namespace admin {

using namespace smsc::util::xml;

Response * CommandAddSme::CreateResponse(scag::Smsc * SmscApp)
{
  SmscApp->getSmeAdmin()->addSme(getSmeInfo());
  return new Response(Response::Ok, "none");
}


}
}
