
#ifndef SMSC_MCI_SME_MCI_MODULE
#define SMSC_MCI_SME_MCI_MODULE

#include <string>

#include <logger/Logger.h>
#include <util/Exception.hpp>

#include <core/threads/Thread.hpp>
#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

#include "misscall/callproc.hpp"

#define MCI_MODULE_TEST YES
//#undef  MCI_MODULE_TEST

extern "C" void clearSignalMask(void);

namespace smsc { namespace mcisme 
{
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
        Mutex   attachLock, exitLock;
        bool    bAttached, bNeedExit;
        
        inline void setNeedExit() {
            MutexGuard guard(exitLock);
            bNeedExit = true;
        }
        inline bool isNeedExit() {
            MutexGuard guard(exitLock);
            return bNeedExit;
        }

    public:

        MCIModule() : Thread(), logger(Logger::getInstance("smsc.mcisme.MCIModule")), 
            listener(0), bAttached(false), bNeedExit(false)
        {
            smsc_log_info(logger, "Starting MCI Module...");
            
        #ifndef MCI_MODULE_TEST    
            module = MissedCallProcessor::instance();
            if (!module) throw Exception("Failed to instantiate MCI Module processor.");
        #endif
            Thread::Start();

            smsc_log_info(logger, "MCI Module started.");
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
                    smsc_log_info(logger, "MCI Module listener attached.");
                }
        #endif
                listener = _listener; bAttached = true;
            }
        }
        void Detach()
        {
            MutexGuard guard(attachLock);

            if (bAttached && listener)
            {
        #ifndef MCI_MODULE_TEST                
                module->removeMissedCallListener();
                smsc_log_info(logger, "MCI Module listener detached.");
        #endif
                listener = 0; bAttached = false;
            }
        }

        #ifdef MCI_MODULE_TEST
        void test()
        {
            MissedCallEvent event; char abonent[128];
            Event sleepEvent;
            for (int i=0; i<10 && !isNeedExit(); i++)
            {
                for (int j=0; j<10 && !isNeedExit(); j++)
                {
                    event.time = time(NULL)+i;
                    sprintf(abonent, "+790290%05d", i);       event.to   = abonent;
                    sprintf(abonent, "+790290%05d", 10000-j); event.from = abonent;
                    {
                        MutexGuard guard(attachLock);
                        if (bAttached && listener) listener->missed(event);
                    }
                    sleepEvent.Wait(500);
                }
            }
        }
        #endif
        
        virtual int Execute()
        {
            clearSignalMask();

            while (!isNeedExit())
            {
                try
                {   
        #ifndef MCI_MODULE_TEST
                    if (module) module->run();
        #else                    
                    test();
        #endif
                } 
                catch (std::exception& exc) {
                    smsc_log_error(logger, "MCI Module failure. Reason: %s", exc.what());
                    exitedEvent.Wait(100);
                }
                catch (...) {
                    smsc_log_error(logger, "MCI Module failure: Cause is unknown");
                    exitedEvent.Wait(100);
                }
            }
            exitedEvent.Signal();
            return 0;
        }
    };
}}

#endif //SMSC_MCI_SME_MCI_MODULE

