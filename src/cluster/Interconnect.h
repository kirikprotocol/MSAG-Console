#ifndef __SMSC_CLUSTER_INTERCONNECT__
#define __SMSC_CLUSTER_INTERCONNECT__

#include "Commands.h"

namespace smsc { namespace cluster 
{
    /**
     * Listener interface to handle ohter-side commands by internal subsystems.
     * Listeners should be registered via Interconnect interface.
     */
    class CommandListener
    {
    protected:
        
        CommandListener() {};
    
    public:
        
        handle(const Command& command) = 0;
    };

    /**
     * Interconnect interface allows:
     * 1) send commands to other side from internal subsystems.
     * 2) register listeners to handle ohter-side commands
     * 3) activate ohter-side commands dispatching process
     */
    class Interconnect
    {
    protected:
        
        Interconnect() {};
        
    public:

        virtual void sendCommand(const Command& command) = 0;
        virtual void addListener(CommandType type, CommandListener* listener) = 0;
        virtual void activate() = 0;
    };

}}

#endif // __SMSC_CLUSTER_INTERCONNECT__
