//
// File:   SmppGwCommandDispatcher.cc
// Author: igork
//
// Created on 26 ��� 2004 �., 19:45
//

#include "SmppGwCommandDispatcher.h"
#include "CommandIds.h"
#include "core/threads/Thread.hpp"
#include "smppgw/smsc.hpp"
#include "core/synchronization/Mutex.hpp"

namespace smsc {
namespace smppgw {
namespace admin {

using namespace smsc::core::synchronization;
using namespace smsc::smppgw;

class GwRunner : public smsc::core::threads::Thread
{
  public:
    GwRunner(const SmscConfigs& cfgs)
    : _app(new smsc::smppgw::Smsc), running(false), configs(cfgs)
    {}

    virtual ~GwRunner() {
      if (_app != 0)
        delete _app;
      _app = 0;
    }

    virtual int Execute() {
      try {
        if (_app != 0) {
          {
            MutexGuard guard(mutex);
            running = true;
          }
          _app->init(configs);
          _app->run();
        }
        else
          fprintf(stderr, "SMSPP GW runner not initialized");
      } catch(std::exception& e) {
        fprintf(stderr, "top level exception: %s\n", e.what());
        return (-1);
      } catch(...) {
        fprintf(stderr, "FATAL EXCEPTION!\n");
        return (-0);
      }
      _app->shutdown();
      {
        MutexGuard guard(mutex);
        running = false;
      }
      fprintf(stderr, "SMSPP GW finished\n");
      return 0;
    }

    void Stop()
    {
      if (_app != 0)
        _app->stop();
    }
    void Abort()
    {
      if (_app != 0)
        _app->abortSmsc();
    }
    void Dump()
    {
      if (_app != 0)
        _app->dumpSmsc();
    }

    bool isRunning()
    {
      MutexGuard guard(mutex);
      return running;
    }

    smsc::smppgw::Smsc* const getApp()
    {
      MutexGuard guard(mutex);
      return _app;
    }

protected:
  smsc::smppgw::Smsc* _app;
  Mutex mutex;
  bool running;
  const SmscConfigs& configs;
};


GwRunner * runner = 0;
Mutex runnerMutex;
const SmscConfigs* SmppGwCommandDispatcher::configs = 0;

void SmppGwCommandDispatcher::startGw()
{
  MutexGuard guard(runnerMutex);
  if (runner == 0) {
    runner = new GwRunner(*configs);
    runner->Start();
  }
}

void SmppGwCommandDispatcher::stopGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Stop();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
}

void SmppGwCommandDispatcher::abortGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Abort();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
}

void SmppGwCommandDispatcher::dumpGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Dump();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
}

SmppGwCommandDispatcher::SmppGwCommandDispatcher(Socket * admSocket)
  : CommandDispatcherTempl<SmppGwCommandReader, ResponseWriter>(admSocket, "gw.aCD")
{
}

SmppGwCommandDispatcher::~SmppGwCommandDispatcher()
{
}

Response * SmppGwCommandDispatcher::handle(const Command * const command) throw (AdminException)
{
  try
  {
    SmppGwCommand * smppgwcommand;
    smppgwcommand = (SmppGwCommand *)command;

    Response * result = smppgwcommand->CreateResponse(runner->getApp());
    DoActions(smppgwcommand->GetActions());
    return result;
  } catch (AdminException &e) {
    return new Response(Response::Error, e.what());
  } catch (const char * const e) {
    return new Response(Response::Error, e);
  } catch (...) {
    return new Response(Response::Error, "Unknown exception");
  }
}

void SmppGwCommandDispatcher::shutdown()
{
  stopGw();
}

void SmppGwCommandDispatcher::DoActions(Actions::CommandActions actions)
{
    if (actions.reloadconfig) {
      configs->routesconfig->reload();
      configs->smemanconfig->reload();
      runner->getApp()->reloadRoutes(*configs);
    }

    if (actions.restart) {
      stopGw();
      startGw();
    }
}

}
}
}
