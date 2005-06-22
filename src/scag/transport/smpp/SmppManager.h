#ifndef SCAG_TRANSPORT_SMPP_MANAGER
#define SCAG_TRANSPORT_SMPP_MANAGER

#include <core/threads/ThreadPool.hpp>

#include "SmppProxy.h"
#include "SmppStateMachine.h"

namespace scag { namespace transport { namespace smpp 
{
    using smsc::core::threads::ThreadPool;
    
    class SmppManager : public SmppProxyOwner, public SmppTransportManager
    {
    private:

        ThreadPool              stateMachines;
        IntHash<SmppEntity*>    entities;

    public:

        SmppManager(ConfigView* config);
        ~SmppManager();

        /**
         * Method used from SmppProxies to notify that command received
         * Interface SmppProxyOwner implementation
         */
        virtual void reportCommand(const SmppProxy& proxy, SmppCommand* command);
        
        /**
         * Method used from SmppProxies to report error
         * Interface SmppProxyOwner implementation
         */
        virtual void reportError(const SmppProxy& proxy, int error);

        
        /**
         * Method used from StateMachines to obtain next command
         * Interface SmppTransportManager implementation
         */
        virtual SmppCommand* getCommand();

        /**
         * Method used to send command to SmppProxy
         * Interface SmppTransportManager implementation
         */
        virtual putCommand(SmppCommand* command);
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_MANAGER

