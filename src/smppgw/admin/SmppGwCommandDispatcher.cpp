// 
// File:   SmppGwCommandDispatcher.cc
// Author: igork
//
// Created on 26 Май 2004 г., 19:45
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
    : _app(new smsc::smppgw::Smsc), running(false), smscConfigs(cfgs)
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
          _app->init(smscConfigs);
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
      
protected:
  smsc::smppgw::Smsc* _app;
  Mutex mutex;
  bool running;
  const SmscConfigs& smscConfigs;
};


GwRunner * runner = 0;
Mutex runnerMutex;
const SmscConfigs* SmppGwCommandDispatcher::smscConfigs = 0;

void SmppGwCommandDispatcher::startGw()
{
  MutexGuard guard(runnerMutex);
  if (runner == 0) {
    runner = new GwRunner(*smscConfigs);
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
  : CommandDispatcherTempl(admSocket, "gw.aCD")
{
}

SmppGwCommandDispatcher::~SmppGwCommandDispatcher()
{
}

Response * SmppGwCommandDispatcher::handle(const Command * const command) throw (AdminException)
{
  try
  {
    switch (command->getId())
    {
    case CommandIds::apply:
      return apply((CommandApply*)command);
    default:
      return new Response(Response::Error, "Unknown command");
    }
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

Response * SmppGwCommandDispatcher::apply(CommandApply*)
{
  try
  {
    stopGw();
    startGw();
    return new Response(Response::Ok, "none");
  } catch (AdminException &e) {
    return new Response(Response::Error, e.what());
  } catch (const char * const e) {
    return new Response(Response::Error, e);
  } catch (...) {
    return new Response(Response::Error, "Unknown exception");
  }
}

}
}
}
