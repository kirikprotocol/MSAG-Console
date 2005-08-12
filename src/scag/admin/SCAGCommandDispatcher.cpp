//
// File:   SCAGCommandDispatcher.cc
// Author: igork
//
// Created on 26 Май 2004 г., 19:45
//

#include "SCAGCommandDispatcher.h"
#include "CommandIds.h"
#include "core/threads/Thread.hpp"
#include "scag/smsc.hpp"
#include "core/synchronization/Mutex.hpp"

namespace scag {
namespace admin {

using namespace smsc::core::synchronization;
using namespace scag;

class GwRunner : public smsc::core::threads::Thread
{
  public:
    GwRunner(const SmscConfigs& cfgs)
    : _app(new scag::Smsc), running(false), configs(cfgs)
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

    scag::Smsc* const getApp()
    {
      MutexGuard guard(mutex);
      return _app;
    }

protected:
  smsc::scag::Smsc* _app;
  Mutex mutex;
  bool running;
  const SmscConfigs& configs;
};


GwRunner * runner = 0;
Mutex runnerMutex;
const SmscConfigs* SCAGCommandDispatcher::configs = 0;

void SCAGCommandDispatcher::startGw()
{
  MutexGuard guard(runnerMutex);
  if (runner == 0) {
    runner = new GwRunner(*configs);
    runner->Start();
  }
}

void SCAGCommandDispatcher::stopGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Stop();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
}

void SCAGCommandDispatcher::abortGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Abort();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
}

void SCAGCommandDispatcher::dumpGw()
{
  MutexGuard guard(runnerMutex);
  if (runner != 0 && runner->isRunning()) {
    runner->Dump();
    runner->WaitFor();
    delete runner;
    runner = 0;
  }
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
  try
  {
    SCAGCommand * scagcommand;
    scagcommand = (SCAGCommand *)command;
    

    Response * result = scagcommand->CreateResponse(runner->getApp());
    DoActions(scagcommand->GetActions());
    return result;
  } catch (AdminException &e) {
    return new Response(Response::Error, e.what());
  } catch (const char * const e) {
    return new Response(Response::Error, e);
  } catch (...) {
    return new Response(Response::Error, "Unknown exception");
  }
}

void SCAGCommandDispatcher::shutdown()
{
  stopGw();
}

void SCAGCommandDispatcher::DoActions(Actions::CommandActions actions)
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
