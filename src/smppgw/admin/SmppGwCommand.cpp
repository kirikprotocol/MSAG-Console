// 
// File:   SmppGwCommand.cpp
// Author: loomox
//
// Created on 24 march 2005 
//

#include "SmppGwCommand.h"
#include "CommandIds.h"
#include "util/xml/utilFunctions.h"
#include "smeman/smeproxy.h"
#include <util/config/route/RouteConfig.h>

#include <router/route_types.h>
#include <sms/sms.h>
#include <sms/sms_const.h>



namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::util::xml;
using namespace smsc::sms;
using namespace smsc::util::config;
using namespace smsc::util::config::route;


SmppGwCommand::SmppGwCommand(Command::Id id) :Command(id)
{
}

SmppGwCommand::~SmppGwCommand()
{
}

Response * SmppGwCommand::CreateResponse(smsc::smppgw::Smsc * SmscApp)
{
  return new Response(Response::Ok, "none");
}

Actions::CommandActions SmppGwCommand::GetActions()
{
  Actions::CommandActions result;
  return result;
}

}
}
}

