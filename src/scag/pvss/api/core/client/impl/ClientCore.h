#ifndef _SCAG_PVSS_CORE_CLIENTCORE_H
#define _SCAG_PVSS_CORE_CLIENTCORE_H

#include <list>
#include <memory>
#include "scag/pvss/api/core/client/Client.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "ClientContext.h"
#include "scag/pvss/api/core/Core.h"
#include "scag/pvss/api/core/ContextRegistry.h"
#include "core/buffers/IntHash.hpp"

namespace scag2 {
namespace pvss {
namespace core {
namespace client {

class Connector;

/// interface
class ClientCore : public Core, public Client
{
private:
    static smsc::logger::Logger* logger;

public:
    /**
     * Constructor creates new ClientCore instance with provided configuration.
     *
     * @param config        Client configuration parameters
     * @param protocol      PVAP protocol serializer/deserializer
     */
    /// NOTE: config and proto get owned
    ClientCore( ClientConfig* config, Protocol* protocol );

    virtual ~ClientCore();

    virtual const char* taskName() { return "pvss.ccore"; }


    /**
     * Implementation of InactivityTracker.Listener method.
     * Will be called by InactivityTracker when channel remains inactive during specified timeout.
     * Initiates PING request sending.
     */
    virtual void inactivityTimeoutExpired( smsc::core::network::Socket& channel ) {
        PvssSocket* sock = PvssSocket::fromSocket(&channel);
        sendPingRequest( *sock );
    }

    
    /**
     * Implementation of Client interface method.
     * Starts up PVSS Client (control thread, channels inactivity tracker, readers & writers).
     * Doesn't create connections to PVSS server. Connections will be created on demand.
     *
     * @throws PvssException    Thrown if client fails to start
     */
    virtual void startup() /* throw(PvssException) */ ;


    /**
     * Implementation of Client interface method.
     * Shuts down PVSS Client (control thread, readers & writers).
     * All pending requests will be processed.
     */
    virtual void shutdown();


    /**
     * Method overrides base class method.
     * It removes this channel from list of registered channels,
     * closes channel by super method and starts connect procedure for new channel.
     *
     * @param channel   Channel to close
     */
    virtual void closeChannel( smsc::core::network::Socket& channel );


    virtual bool registerChannel( PvssSocket& channel, util::msectime_type utime ) {
        {
            smsc::core::synchronization::MutexGuard mg(channelMutex_);
            activeChannels_.push_back(&channel);
        }
        return Core::registerChannel(channel,utime);
    }


    virtual bool canProcessRequest( PvssException* exc = 0 );


    /**
     * Implementation of Client interface method.
     * Sends PVAP request and waits PVAP response (sync mode).
     * Method blocks calling thread until response came or error will be occured.
     * However another thread may process diffrent call in same time on this Client.
     *
     * @param request   PVAP Request packet to send to PVSS server
     * @return          PVAP Response packet on success
     * @throws PvssException Thrown when client failed to accept or process request
     */
    std::auto_ptr<Response> processRequestSync(std::auto_ptr<Request>& request) /* throw(PvssException) */ ;


    /**
     * Implementation of Client interface method.
     * Sends PVAP request and registers callback handler to wait PVAP response (async mode).
     * Outgoing request will be enqueued in writers (accepted) and provided handler will be registered for it.
     * These handler will be notified when response came or error be occurred for accepted request.
     *
     * @param request   PVAP Request packet to send to PVSS server
     * @param handler   Callback handler to notify on response or error for this request
     * @throws PvssException Thrown when client rejects request processing (e.g. not inited/connected, writers queue is full)
     */
    void processRequestAsync( std::auto_ptr<Request>& request, Client::ResponseHandler& handler) /* throw(PvssException) */ ;


    /**
     * Implementation of Core abstraction method.
     * Used to handle new response received by Reader or status of sending request.
     *
     * @param packet    Processing packet (request or response)
     * @param channel   Channel on which packet is processing
     * @param state     Packet state in IO processing
     */
    virtual void receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel );


    /**
     * Method notifies core logic that context (owning the packet) has been sent/expired/failed.
     *
     * @param context   Context owning the packet.
     * @param state     Packet state in IO processing { EXPIRED, SENT, FAIL }
     * 
     */
    virtual void reportPacket(uint32_t seqnum, smsc::core::network::Socket& channel, PacketState state);


protected:
    /**
     * Implementation of Runable interface method (Thread class).
     * Controls processing requests queue for processing timeouts.
     */
    virtual int doExecute();

private:

    void clearChannels();

    void destroyDeadChannels();

    virtual ClientConfig& getConfig() const {
        return *static_cast<ClientConfig*>(config);
    }

    int getNextSeqNum() {
        smsc::core::synchronization::MutexGuard mg(seqNumMutex_);
        return (++lastUsedSeqNum_ > 0) ? lastUsedSeqNum_ : (lastUsedSeqNum_ = 1);
    }

    /**
     * Internal method used to create channel and schedule connect attempt time.
     *
     * @param startConnectTime  time to start connect attempt
     * @throws PvssException    thrown when channel creation failed
     */
    void createChannel( util::msectime_type startConnectTime ); // /* throw(PvssException) */ 

    /**
     * Internal method used to obtain next channel for request sending.
     * Channels are selected from channels queue by Round Robin algorithm
     *
     * @return  next channel to send request to
     * @throws PvssException thrown only if no one channel is registered and connected to PVSS server.
     */
    PvssSocket& getNextChannel() /* throw(PvssException) */ ;

    /**
     * Internal method for async request processing.
     * Used from processRequestSync() & processRequestAsync().
     *
     * @param context   Context contains request to process & response handler to notify
     * @throws PvssException  Thrown when client failed to accept request
     * 
     */
    void sendRequest( std::auto_ptr<ClientContext>& context ) /* throw(PvssException) */ ;

    /**
     * Internal method sends PING request into provided channel.
     *
     * @param channel Channel to send PING
     */
    void sendPingRequest( PvssSocket& channel );

    /**
     * Method processes incoming response packet.
     *
     * @param response  response to process
     * @param channel   channel from which response was received
     * @throws PvssException thrown if core is failed to process response
     * 
     * NOTE: Ownership is taken!
     */
    void handleResponse(Response* response,PvssSocket& channel) /* throw(PvssException) */ ;


    void handleResponseWithContext( std::auto_ptr< ClientContext >& context,
                                    Response* response,
                                    smsc::core::network::Socket& channel ) /* throw(PvssException) */ ;


    /*
     * Internal method used to shutdown registered channels
     * and finalize unprocessed requests.
     */
    // void cleanup();

private:
    typedef std::list< PvssSocket* >          ChannelList;

private:
    smsc::core::synchronization::EventMonitor channelMutex_;
    ChannelList                        channels_;        // channels for connector/r/w
    ChannelList                        activeChannels_;  // channels for r/w
    ChannelList                        deadChannels_;    // closed channels (pending for destruction)

    // Requests which has been passed to writers
    ContextRegistrySet                 writingRegset_;

    // Requests waiting for responses (ordered by time)
    ContextRegistrySet                 regset_;
    std::auto_ptr<Connector>           connector_;

    smsc::core::synchronization::Mutex seqNumMutex_;
    int lastUsedSeqNum_; // PVSS client uses incrementable seqNums for all channels

    smsc::core::synchronization::Mutex startMutex_;
    bool                               started_;

};

} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_CLIENTCORE_H */
