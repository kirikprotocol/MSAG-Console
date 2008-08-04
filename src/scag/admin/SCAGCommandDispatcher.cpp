//
// File:   SCAGCommandDispatcher.cc
// Author: igork
//
// Created on 26 ��� 2004 �., 19:45
//

#include "SCAGCommandDispatcher.h"
#include "CommandIds.h"
#include "core/threads/Thread.hpp"

#include "core/synchronization/Mutex.hpp"
#include "scag/config/ConfigManager.h"

// #include "scag/scag.h"

#include "logger/Logger.h"

scag::Scag *getApp();

namespace scag {
namespace admin {

using namespace smsc::core::synchronization;
using namespace scag;
using scag::config::ConfigManager;
using smsc::logger::Logger;

Logger * SCAGCommandDispatcher::getLogger()
{
    static Logger * logger = Logger::getInstance("CmdDsp");
    return logger;
}

SCAGCommandDispatcher::SCAGCommandDispatcher(Socket * admSocket)
  : CommandDispatcherTempl<SCAGCommandReader, ResponseWriter>(admSocket, "gw.aCD")
{
}

SCAGCommandDispatcher::~SCAGCommandDispatcher()
{
}

Response * SCAGCommandDispatcher::handle(const Command * const command) throw (AdminException)
{
    AdminCommand * adminCommand = 0;
    Command * _command = const_cast<Command *>(command);
    adminCommand = dynamic_cast<AdminCommand *>(_command);

    if (!adminCommand) throw AdminException("Fatal Error: Command is not a 'command' type");

    try {
        Response * result = adminCommand->CreateResponse(getApp());
        //DoActions(adminCommand->GetActions());
        return result;
    } catch (AdminException &e) {
        return new Response(Response::Error, e.what());
    } catch(std::exception& e)
    {
      return new Response(Response::Error, e.what());
    } catch (const char * const e) {
        return new Response(Response::Error, e);
    } catch (...) {
        return new Response(Response::Error, "Unknown exception");
    }
}

/*
void SCAGCommandDispatcher::DoActions(Actions::CommandActions actions)
{
    ConfigManager & cfg = ConfigManager::Instance();

    if (actions.reloadroutes)
        cfg.reloadConfig(scag::config::ROUTE_CFG);
}
*/
}
}
