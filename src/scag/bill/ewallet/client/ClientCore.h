#ifndef SCAG_BILL_EWALLET_CLIENT_CLIENTCORE_H
#define SCAG_BILL_EWALLET_CLIENT_CLIENTCORE_H

#include "scag/bill/ewallet/Client.h"
#include "scag/bill/ewallet/proto/Core.h"
#include "ClientContext.h"
#include "Connector.h"
#include "Loopback.h"
#include "scag/bill/ewallet/proto/ContextRegistry.h"

namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

/// an implementation
class ClientCore : public Client, public proto::Core
{
private:
public:
    ClientCore( proto::Config* config, Streamer* streamer );

    virtual ~ClientCore();

    virtual const char* taskName() { return "ewall.core"; }

    virtual void startup();
    virtual void shutdown();
    virtual bool canProcessRequest( Exception* exc = 0 );

    /// process request asynchronously.
    /// handler must live until the request is processed (or failed).
    virtual void processRequest( std::auto_ptr<Request> request, ResponseHandler& handler );

    virtual void receivePacket( proto::SocketBase& socket, std::auto_ptr<Packet> packet );
    /// report context state change.
    /// NOTE: \param context may be NULL, in which case context should be searched in internal registry.
    virtual void reportPacket( proto::SocketBase& socket,
                               uint32_t seqNum, 
                               proto::Context* context,
                               proto::Context::ContextState state );
    virtual void handleError( proto::SocketBase& socket, const Exception& exc );

    virtual bool registerSocket( proto::Socket& socket );

protected:
    virtual int doExecute();
    // virtual ClientContext* mergeResponse( proto::Socket& socket,
    // std::auto_ptr<Response> response );

    virtual void stopCoreLogic() {}

    virtual void inactivityTimeout( proto::Socket& socket );

private:
    void sendRequest( std::auto_ptr< proto::Context >& context );
    // void processContext( proto::Socket& socket,
    // std::auto_ptr< ClientContext > context );

    // create a socket, send it to connector
    void createSocket( util::msectime_type oldac = 0 ); // throw

    uint32_t getNextSeqNum();
    proto::SocketBase& getNextSocket( const Request* request ); // throw

    void closeAllSockets();
    void destroyDeadSockets();
    virtual void closeSocket( proto::Socket& socket );

private:
    typedef proto::ContextRegistrySet< ClientContext >  RegistrySet;
    typedef std::list< proto::Socket* >                 SocketList;

private:
    smsc::core::synchronization::Mutex      startMutex_;
    bool                                    started_;

    smsc::core::synchronization::Mutex      seqMutex_;
    uint32_t                                seqNum_;
                                           
    smsc::core::synchronization::EventMonitor socketMon_;
    SocketList                              sockets_;
    SocketList                              deadSockets_;
    SocketList                              activeSockets_;

    std::auto_ptr< Connector >              connector_;

    RegistrySet                             regSet_;
    std::auto_ptr< Loopback >               loopback_;
};

} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2

#endif /* !SCAG_BILL_EWALLET_CLIENT_H */
