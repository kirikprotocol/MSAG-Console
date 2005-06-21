#ifndef SCAG_TRANSPORT_SMPP_CONNECTOR
#define SCAG_TRANSPORT_SMPP_CONNECTOR

#include "SmppProxy.h"

namespace scag { namespace transport { namespace smpp 
{
    
    class SmscConnector : public Thread, public SmppProxyOwner
    {
    private:

        std::string     host;
        int             port;

        SmppProxyOwner& owner;

    public:
    
        SmscConnector(const std::string& _host, int _port, SmppProxyOwner& _owner) 
            : Thread(), SmppProxyOwner(), owner(_owner), host(_host), port(_port) {};
        ~SmscConnector();

        /**
         * Used internally to reconnect proxy
         */
        virtual void reportError(const SmppProxy &proxy);
        
        virtual void reportCommand(const SmppProxy &proxy) {
            owner.reportCommand(proxy);
        };
    };

}}}

#endif // SCAG_TRANSPORT_SMPP_CONNECTOR

