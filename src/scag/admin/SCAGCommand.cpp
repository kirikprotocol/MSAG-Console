// 
// File:   SCAGCommand.cpp
// Author: loomox
//
// Created on 24 march 2005 
//

#include "SCAGCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "smeman/smeproxy.h"
#include <util/config/route/RouteConfig.h>

#include <router/route_types.h>
#include <sms/sms.h>
#include <sms/sms_const.h>



namespace scag {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::util::config::route;


SCAGCommand::SCAGCommand(Command::Id id) :Command(id),
    logger(smsc::logger::Logger::getInstance("admin.command"))
{
}

SCAGCommand::~SCAGCommand()
{
}

Response * SCAGCommand::CreateResponse(scag::Scag * ScagApp)
{
  return new Response(Response::Ok, "none");
}

Actions::CommandActions SCAGCommand::GetActions()
{
  Actions::CommandActions result;
  return result;
}

}
}

