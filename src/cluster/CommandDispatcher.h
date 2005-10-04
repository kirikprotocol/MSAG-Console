#ifndef __SMSC_CLUSTER_COMMAND_DISPATCHER__
#define __SMSC_CLUSTER_COMMAND_DISPATCHER__

#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/Event.hpp"
#include "core/synchronization/EventMonitor.hpp"

#include "core/threads/Thread.hpp"

#include "core/buffers/IntHash.hpp"
#include "core/buffers/Array.hpp"

#include "Interconnect.h"
#include "logger/Logger.h"

namespace smsc { namespace cluster 
{
    using namespace smsc::core::synchronization;
    using namespace smsc::core::buffers;
    
    using smsc::core::threads::Thread;
    
    class CommandDispatcher : public Thread
    {
    private:
        smsc::logger::Logger    *logger;
        EventMonitor        commandsMonitor;
        Array<Command *>    commands;

        Mutex                               listenersLock;
        Mutex                               stopLock;
        IntHash< Array<CommandListener *> > listeners;
        bool isStoped();

        bool isStopping;
        
    public:

        CommandDispatcher();
        ~CommandDispatcher();

        void addCommand(Command* command);
        void addListener(CommandType type, CommandListener* listener);
        
        void Start();
        void Stop();

        virtual int Execute();
    };

}}

#endif // __SMSC_CLUSTER_COMMAND_DISPATCHER__

