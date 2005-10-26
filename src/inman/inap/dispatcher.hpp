#ident "$Id$"

#ifndef __SMSC_INMAN_INAP_DISPATCHER__
#define __SMSC_INMAN_INAP_DISPATCHER__

#include "logger/Logger.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/Event.hpp"

using smsc::logger::Logger;
using smsc::core::threads::Thread;
using smsc::core::synchronization::Event;

namespace smsc  {
namespace inman {
namespace inap  {

class Dispatcher : public Thread
{
    public:
        Dispatcher();
        virtual ~Dispatcher();

        virtual void Run();
        virtual void Stop();
        virtual int  Execute();


    protected:
        Event           started;
        Event           stopped;
        volatile bool   running;
        Logger*         logger;
};

} //inap
} //inman
} //smsc

#endif /* __SMSC_INMAN_INAP_DISPATCHER__ */

