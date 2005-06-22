#ifndef SCAG_TRANSPORT_SMPP_STATE_MACHINE
#define SCAG_TRANSPORT_SMPP_STATE_MACHINE

#include <core/threads/ThreadedTask.hpp>

#include "SmppCommand.h"

namespace scag { namespace transport { namespace smpp 
{
    using smsc::core::threads::ThreadedTask;
    
    class SmppStateMachine;
    struct SmppTransportManager
    {
        /**
         * Method used to get command from SmppManager
         * @return  null or command
         */
        virtual SmppCommand* getCommand() = 0;
        
        /**
         * Method used to send command via SmppProxy
         * @param   command     command to send
         */
        virtual putCommand(SmppCommand* command) = 0;

    protected:

        SmppTransportManager() {};
    };

    class SmppStateMachine : public ThreadedTask
    {
    protected:

        SmppTransportManager&   manager;

    public:

        SmppStateMachine(SmppTransportManager& _manager) 
            : ThreadedTask(), manager(_manager) {};
        virtual ~SmppStateMachine() {};
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_STATE_MACHINE
