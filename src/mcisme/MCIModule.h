
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
        
        Logger*             logger;
        MissedCallListener* listener;
    
        Mutex   startLock;
        Event   exitedEvent;
        bool    bStarted, bNeedExit;
        
        inline MissedCallProcessor* instantiateModule() {
        #ifndef MCI_MODULE_TEST
            MissedCallProcessor *mcp = MissedCallProcessor::instance();
            if (!mcp) smsc_log_error(logger, "Failed to instantiate MCI Module processor.");
            return mcp;
        #else
            return 0;
        #endif 
        }
    
    public:

        MCIModule(MissedCallListener* _listener) 
            : logger(Logger::getInstance("smsc.mcisme.MCIModule")),
                listener(_listener), bStarted(false), bNeedExit(false) {};
        virtual ~MCIModule() { Stop(); };
        

        inline bool isStarted()
        {
            MutexGuard guard(startLock);
            return bStarted;
        }
        void Start()
        {
            MutexGuard guard(startLock);
            
            if (!bStarted) {
                smsc_log_info(logger, "Starting ...");
                bNeedExit = false; 
        #ifndef MCI_MODULE_TEST
                MissedCallProcessor *mcp = instantiateModule();
                if (mcp && listener) {
                    mcp->addMissedCallListener(listener);
                }
        #endif
                Thread::Start(); bStarted = true;
                smsc_log_info(logger, "Started.");
            }

        }
        void Stop()
        {
            MutexGuard  guard(startLock);

            if (bStarted) {
                smsc_log_info(logger, "Stopping ...");
                bNeedExit = true; 
        #ifndef MCI_MODULE_TEST
                MissedCallProcessor *mcp = instantiateModule();
                if (mcp) { 
                    mcp->stop(); mcp->removeMissedCallListener();
                }
        #endif
                exitedEvent.Wait(); bStarted = false;
                smsc_log_info(logger, "Stoped.");
            }
        }
        
        #ifdef MCI_MODULE_TEST
        void test()
        {
            MissedCallEvent event; char abonent[128];
            Event sleepEvent;
            for (int i=0; i<10 && !bNeedExit; i++)
            {
                for (int j=0; j<10 && !bNeedExit; j++)
                {
                    event.time = time(NULL)+i;
                    sprintf(abonent, "+790290%05d", i);       event.to   = abonent;
                    sprintf(abonent, "+790290%05d", 10000-j); event.from = abonent;
                    listener->missed(event);
                    sleepEvent.Wait(500);
                }
            }
        }
        #endif
        
        virtual int Execute()
        {
            while (!bNeedExit)
            {
                try
                {   
        #ifndef MCI_MODULE_TEST                    
                    MissedCallProcessor *mcp = instantiateModule();
                    if (!mcp) break;
                    mcp->run();
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

