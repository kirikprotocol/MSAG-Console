#ifndef __SMSC_CLUSTER_INTERCONNECT__
#define __SMSC_CLUSTER_INTERCONNECT__

#include "Commands.h"

namespace smsc { namespace cluster
{

    typedef enum {
        SINGLE = 0,
        MASTER = 1,
        SLAVE  = 2
    } Role;

    typedef void ChangeRoleHandler(Role, void*);

    /**
     * Listener interface to handle ohter-side commands by internal subsystems.
     * Listeners should be registered via Interconnect interface.
     */
    class CommandListener
    {
    protected:

        CommandListener() {};

    public:

        virtual void handle(const Command& command) = 0;
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
        static Interconnect* instance;


    public:

        static Interconnect* getInstance(){
            return instance;
        };


        //virtual void sendCommand(const Command& command) = 0;
        virtual void sendCommand(Command* command) = 0;
        virtual void addListener(CommandType type, CommandListener* listener) = 0;
        virtual void activate() = 0;
        virtual Role getRole() = 0;
        virtual void changeRole(Role role_) = 0;
        virtual void  addChangeRoleHandler(ChangeRoleHandler*, void*) = 0;
    };

}}

#endif // __SMSC_CLUSTER_INTERCONNECT__
