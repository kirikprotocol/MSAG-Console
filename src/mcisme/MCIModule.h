
#ifndef SMSC_MCI_SME_MCI_MODULE
#define SMSC_MCI_SME_MCI_MODULE

#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <string>

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include <system/smscsignalhandlers.h>

#include <mcisme/callproc.hpp>

//#define MCI_MODULE_TEST YES
#undef  MCI_MODULE_TEST

namespace smsc {
namespace mcisme {

using std::string;
using std::exception;
    
using smsc::logger::Logger;
using smsc::util::Exception;

using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;
using smsc::core::synchronization::Mutex;

using namespace smsc::misscall;

class MCIModule : public Thread
{
private:
        
  Logger*                 logger;
  MissedCallListener*     listener;
    
#ifndef MCI_MODULE_TEST        
  MissedCallProcessor*    module;
#endif
        
  Event   exitedEvent;
  Mutex   attachLock, exitLock, runningLock;
  bool    bAttached, bNeedExit, bRunning;
        
  inline void setNeedExit() {
    MutexGuard guard(exitLock);
    bNeedExit = true;
  }
  inline bool isNeedExit() {
    MutexGuard guard(exitLock);
    return bNeedExit;
  }
  void setRunning(bool running) {
    MutexGuard guard(runningLock);
    bRunning = running;
  }

public:

  MCIModule(Hash<Circuits>& circuits, std::vector<Rule>& rules, 
            const ReleaseSettings& releaseSettings, const char* redirectionAddress,
            const char* callingMask, const char* calledMask, const char* countryCode,
            const IsupUserCfg& isup)
    : Thread(), logger(Logger::getInstance("smsc.mcisme.MCIModule")), 
      listener(0), bAttached(false), bNeedExit(false), bRunning(false)
  {
    smsc_log_info(logger, "Initing MCI Module...");
#ifndef MCI_MODULE_TEST    
    module = MissedCallProcessor::instance();
    if (!module) throw Exception("Failed to instantiate MCI Module processor.");
#ifdef EIN_HD
    module->configure(isup);
#endif

    module->setReleaseSettings(releaseSettings);
    module->setRedirectionAddress(redirectionAddress);
    module->setCircuits(circuits);
    module->setRules(rules);
    module->setCountryCode(countryCode);
    if (!setCallingMask(callingMask))
      throw Exception("Failed to compile calling mask '%s'.", callingMask ? callingMask:"");
    if (!setCalledMask(calledMask)) 
      throw Exception("Failed to compile called mask '%s'.", calledMask ? calledMask:"");
#endif
    smsc_log_info(logger, "MCI Module inited.");
  };
  virtual ~MCIModule()
  { 
    smsc_log_info(logger, "Stopping MCI Module...");
            
    Detach(); setNeedExit();
#ifndef MCI_MODULE_TEST
    if (module) module->stop();
#endif
    exitedEvent.Wait();
            
    smsc_log_info(logger, "MCI Module stopped.");
  };
        
  void Attach(MissedCallListener* _listener)
  {
    MutexGuard guard(attachLock);
            
    if (!bAttached && _listener)
    {
#ifndef MCI_MODULE_TEST                
      if (module) {
        module->removeMissedCallListener();
        module->addMissedCallListener(_listener);
      }
#endif
      listener = _listener; bAttached = true;
      smsc_log_info(logger, "MCI Module listener attached.");
    }
  }
  void Detach()
  {
    MutexGuard guard(attachLock);

    if (bAttached && listener)
    {
#ifndef MCI_MODULE_TEST                
      module->removeMissedCallListener();
#endif
      listener = 0; bAttached = false;
      smsc_log_info(logger, "MCI Module listener detached.");
    }
  }

#ifdef MCI_MODULE_TEST
  void test()
  {
    const int maxAbonents = 900000;
    srandom((unsigned int)time(NULL)/*(unsigned int)0xdeadbeaf*/);

    MissedCallEvent event; char abonent[128]; Event sleepEvent;
    for (int i=0; i<maxAbonents && !isNeedExit(); i++)
    {
      int number = (int)(random()%maxAbonents);
      int caller = (int)(maxAbonents - random()%maxAbonents);
      event.time = time(NULL)+((int)random()%3600);
      sprintf(abonent, "+70123%06d", number); event.to   = abonent;
      sprintf(abonent, "+70123%06d", caller); event.from = abonent;
      if (i%10 == 0)
        event.from = ""; // unknown
      event.cause = ABSENT;
      {
        MutexGuard guard(attachLock);
        if (bAttached && listener) 
        {
          listener->missed(event);
          listener->missed(event);
          listener->missed(event);
          caller = (int)(maxAbonents - random()%maxAbonents);
          sprintf(abonent, "+79029%06d", caller); event.from = abonent;
          listener->missed(event);
          caller = (int)(maxAbonents - random()%maxAbonents);
          sprintf(abonent, "+79029%06d", caller); event.from = abonent;
          listener->missed(event);
          listener->missed(event);
          listener->missed(event);
          caller = (int)(maxAbonents - random()%maxAbonents);
          sprintf(abonent, "+79029%06d", caller); event.from = abonent;
          listener->missed(event);
          listener->missed(event);
          listener->missed(event);
          listener->missed(event);
          listener->missed(event);
        }
      }
      sleepEvent.Wait(20);
    }
  }
#endif
        
  bool isRunning() {
    MutexGuard guard(runningLock);
    return bRunning;
  }
  virtual int Execute()
  {
    while (!isNeedExit())
    {
      setRunning(true);
      try
      {   
#ifndef MCI_MODULE_TEST
        int result = 0;
        if (module) result = module->run();
        setRunning(false);
        smsc_log_warn(logger, "MCI Module exited. Exit code=%d, restarting module", result);
#else                    
        test(); setRunning(false);
#endif
      } 
      catch (std::exception& exc) {
        smsc_log_error(logger, "MCI Module failure. Reason: %s", exc.what());
        setRunning(false); exitedEvent.Wait(100);
      }
      catch (...) {
        smsc_log_error(logger, "MCI Module failure: Cause is unknown");
        setRunning(false); exitedEvent.Wait(100);
      }
    }
    exitedEvent.Signal();
    return 0;
  }
};
}}

#endif //SMSC_MCI_SME_MCI_MODULE

