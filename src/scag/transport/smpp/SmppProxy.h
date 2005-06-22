#ifndef SCAG_TRANSPORT_SMPP_PROXY
#define SCAG_TRANSPORT_SMPP_PROXY

#include "SmppCommand.h"

namespace scag { namespace transport { namespace smpp 
{

    class SmppProxy;
    struct SmppProxyOwner
    {
        /**
         * Method used to notify SmppManager that proxy has received SmppCommand
         * @param   proxy       signalling SmppProxy
         * @param   command     received command
         */
        virtual void reportCommand(const SmppProxy& proxy, SmppCommand* command) = 0; 

        /**
         * Method used to notify SmppManager that proxy has disconnected ???
         * @param   proxy       signalling SmppProxy
         * @param   error       SMPP error code
         */
        virtual void reportError(const SmppProxy& proxy, int error) = 0; 
    
    protected:

        SmppProxyOwner() {};
    };
    
    class SmppProxy
    {
    protected:

        SmppProxyOwner& owner; // used to notify owner when command received or error occured

        // TODO: Add SmppReader & SmppWriter

        virtual bool checkCommand(const SmppCommand* command) = 0;
        SmppProxy(SmppProxyOwner& _owner) : owner(_owner) {};

    public:

        virtual ~SmppProxy() {};
        
        int getBindState();

        void putCommand(const SmppCommand* command);
    };

    class SmeProxy : public SmppProxy
    {
    protected:

        virtual bool checkCommand(const SmppCommand* command);

    public:

        SmeProxy(SmppProxyOwner& _owner) : SmppProxy(_owner) {};
        virtual ~SmeProxy() {};
    };

    class SmscProxy : public SmppProxy
    {
    protected:

        virtual bool checkCommand(const SmppCommand* command);

    public:

        SmscProxy(SmppProxyOwner& _owner) : SmppProxy(_owner) {};
        virtual ~SmscProxy() {};
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_MANAGER

