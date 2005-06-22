#ifndef SCAG_TRANSPORT_SMPP_MANAGER
#define SCAG_TRANSPORT_SMPP_MANAGER

#include "SmppProxy.h"
#include "SmppStateMachine.h"

namespace scag { namespace transport { namespace smpp 
{
    
    class SmppManager : public SmppProxyOwner, public SmppTransportManager
    {
    private:


    public:

        SmppManager(ConfigView* config);
        ~SmppManager();

        /**
         * Method used from SmppProxies to notify that command received
         */
        virtual void reportCommand(const SmppProxy& proxy, SmppCommand* command);
        
        /**
         * Method used from SmppProxies to report error
         */
        virtual void reportError(const SmppProxy& proxy, int error);

        /**
         * Method used from StateMachines to obtain next command
         */
        SmppCommand* getCommand();
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_MANAGER

