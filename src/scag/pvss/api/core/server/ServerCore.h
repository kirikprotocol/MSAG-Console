#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCORE_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCORE_H

#include <memory>
#include "scag/pvss/api/core/Core.h"
#include "scag/pvss/api/core/ContextRegistry.h"
#include "scag/pvss/api/core/Statistics.h"
#include "Server.h"
#include "core/buffers/Array.hpp"
#include "ServerConfig.h"
#include "Acceptor.h"
#include "AsyncDispatcherThread.h"
#include "Dispatcher.h"

namespace scag2 {
namespace pvss {

class Protocol;

namespace core {
namespace server {

class Worker;

/// interface
class ServerCore : public Core, public Server
{
public:
    /**
     * Constructor creates new ServerCore instance with provided configuration.
     *
     * @param config        Server configuration parameters
     * @param protocol      PVAP protocol serializer/deserializer
     */
    ServerCore( ServerConfig* config, Protocol* protocol );

    virtual ~ServerCore();

    virtual const char* taskName() { return "pvss.srv"; }

    /**
     * Implementation of InactivityTracker.Listener method.<p/>
     * Will be called by InactivityTracker when channel remains inactive during specified timeout.<p/>
     * Method implementation initiates channel closing.
     */
    virtual void inactivityTimeoutExpired(smsc::core::network::Socket& channel) {
        smsc_log_warn(log_,"Inactivity timeout expired for channel %p", &channel);
        closeChannel(channel);
    }


    void startSubTask( smsc::core::threads::ThreadedTask* task, bool delOnCompletion ) {
        threadPool_.startTask(task,delOnCompletion);
    }


    /// try to accept a channel for IO. Ownership is passed.
    virtual bool acceptChannel( PvssSocket* channel );


    /// accept an old transport channel
    void acceptOldChannel( smsc::core::network::Socket* socket );

    /*
     * Used to send response or error for processed request.
     * Invoked from worker thread.
     * Puts response to Writer queue for channel on wich original request was received.
     *
     * @param context           ServerContext instance containing processed request with response or error set.
     * @throws PvssException    Thrown if provided context is invalid or server failes to sent it.
     */
    void contextProcessed(std::auto_ptr<ServerContext> context); // /* throw(PvssException) */ ;


    /**
     * Implementation of Core abstraction method.
     *
     * @param packet    Processing packet (request or response)
     * @param channel   Channel on which packet is processing
     * @param state     Packet state in IO processing
     */
    virtual void receivePacket(std::auto_ptr<Packet> packet, PvssSocket& channel);


    /**
     * Implementation of Core abstraction method.
     */
    virtual void receiveOldPacket( std::auto_ptr< ServerContext > context );


    /**
     * Implementation of Core abstraction method.
     * Invoked from writer to notify core api of a packet state change (sent,failed,expired).
     *
     * @param seqnum    seqnum of processed packetProcessing packet (request or response)
     * @param channel   Channel on which packet is processing
     * @param state     Packet state in IO processing
     */
    virtual void reportPacket(uint32_t seqnum, smsc::core::network::Socket& channel, PacketState state);


    void init() /* throw PvssException */;


    /**
     * Implementation of Server interface method.
     * Starts up PVSS server (acceptor & control thread, readers & writers).
     *
     * @throws PvssException    Thrown if server fails to start
     */
    /*public synchronized */
    virtual void startup( SyncDispatcher& dispatcher ) /* throw(PvssException) */ ;
    virtual void startup( AsyncDispatcher& dispatcher ) /* throw (PvssException) */ ;

    /*
    {
        if (started) return;

        startupIO();
        try {
            acceptor = new Acceptor(new InetSocketAddress(config.getHost(), config.getPort()));
        } catch (PvssException exc) {
            log_.error("Acceptor start error: " + exc.getMessage());
            shutdownIO();
            throw exc;
        }

        started = true;
        this.start();
    }
     */

    /**
     * Implementation of Server interface method.
     * Shuts down PVSS server in background (non blocking).
     *
     * Shutdown sequence:
     * 1) Stop acceptor (close server socket).
     * 2) Signal readers to stop receive new requests and exit.
     * 3) Signal writers to send or cancel the rest of data and exit.
     */
    virtual void shutdown();
    /*
    {
        started = false;
        acceptor.shutdown();
        shutdownIO();
        synchronized (processingContexts) {
            processingContexts.notifyAll();
        }
        synchronized (contextsMonitor) {
            contextsMonitor.notifyAll();
        }
    }
     */

    /*
    private void timeoutContext(ServerContext context, boolean pending)
    {
        String contextType = (pending) ? "Pending":"Processing";
        final int seqNum = context.getSeqNum();
        final SocketChannel channel = context.getChannel();
        try {
            sendResponse(new ErrorResponse(seqNum, Response.StatusType.REQUEST_TIMEOUT,
                                           contextType + " request timeout"), channel);
        } catch (PvssException send_error_exc) {
            log_.warn(contextType + " request timeout send failed. Details: " + send_error_exc.getMessage());
        }
    }

    private void cancelContext(ServerContext context, boolean pending)
    {
        String contextType = (pending) ? "Pending":"Processing";
        final int seqNum = context.getSeqNum();
        final SocketChannel channel = context.getChannel();
        try {
            sendResponse(new ErrorResponse(seqNum, Response.StatusType.SERVER_SHUTDOWN,
                                           contextType + " request cancelled"), channel);
        } catch (PvssException send_error_exc) {
            log_.warn(contextType + " request cancelled send failed. Details: " + send_error_exc.getMessage());
        }
    }
     */

    /*
    {
        final long minTimeout = 10; // min sleep time = 10msec
        final long processTimeout = config.getProcessTimeout();
        long nextWakeupTime;

        while (started)
        {
            final long currentTime = System.currentTimeMillis();

            synchronized (contextsMonitor) // check pending requests queue
            {
                ServerContext context = pendingContexts.peekFirst();
                if (context != null) {
                    long nextTime = context.getCreationTime() + processTimeout;
                    if (currentTime >= nextTime) { // pending context has expired
                        pendingContexts.removeFirst();
                        timeoutContext(context, true);
                        continue; // expire all pending packets first
                    }
                    else nextWakeupTime = nextTime;
                }
                else nextWakeupTime = currentTime + processTimeout;
            }

            synchronized (processingContexts) // no pending requests for expire now => check processing contexts
            {
                ServerContext context = processingContexts.peekFirst();
                if (context != null) {
                    final long nextTime = context.getCreationTime() + processTimeout;
                    if (currentTime >= nextTime) {
                        processingContexts.removeFirst();
                        ServerContext removed = removeProcessingContext(context.getChannel(), context.getSeqNum(), true);
                        if (removed != null) timeoutContext(removed, false); // processing context has expired
                        continue; // expire the rest of contexts
                    }
                    else if (nextTime < nextWakeupTime) nextWakeupTime = nextTime;
                }

                long timeToSleep = nextWakeupTime - currentTime;
                try { processingContexts.wait((timeToSleep > minTimeout) ? timeToSleep : minTimeout); }
                catch (InterruptedException e) { e.printStackTrace(); }
            }
        }
        cleanup();
    }
     */

    /*
    private void cleanup()
    {
        synchronized (contextsMonitor) { // cancel all pending requests
            for (ServerContext context : pendingContexts) cancelContext(context, true);
            pendingContexts.clear();
        }

        synchronized (processingContexts) { // cancel not PROCESSED contexts
            for (ServerContext context : processingContexts) {
                ServerContext removed = removeProcessingContext(context.getChannel(), context.getSeqNum(), true);
                if (removed != null) cancelContext(removed, false); // cancel processing context
            }
            processingContexts.clear();
        }
    }
     */

    virtual ServerConfig& getConfig() const { return *static_cast<ServerConfig*>(config);}


    /// notification sent when worker thread is stopped
    void workerThreadIsStopped() {
        MutexGuard mg(logicMon_);
        logicMon_.notify();
    }

    /// count caught exceptions
    void countExceptions( PvssException::Type et, const char* where );

private:
    /**
     * Implementation of Runable interface method (Thread class).
     * Controls pending & processing queues (request processing timeouts checking).
     */
    virtual int doExecute();


    /// invoked when a new packet is received either from new or old transport.
    void receiveContext( std::auto_ptr< ServerContext > context );

    void sendResponse(std::auto_ptr<ServerContext>& ctx) /* throw(PvssException) */ ;

    /// report context
    void reportContext( std::auto_ptr<ServerContext> ctx );

    /// close channel if managed locally or leave it.
    void closeChannel( smsc::core::network::Socket* socket );

    virtual void closeChannel( smsc::core::network::Socket& channel )
    {
        closeChannel(&channel);
    }

    virtual void stopCoreLogic();

    void destroyDeadChannels();

    /// NOTE: statMutex_ must be locked
    util::msectime_type checkStatistics();

private:
    typedef std::list< smsc::core::network::Socket* > ChannelList;

    class ExceptionCount
    {
        typedef smsc::core::buffers::IntHash< smsc::core::buffers::Hash< unsigned > > Count;
    public:
#ifdef INTHASH_USAGE_CHECKING
        ExceptionCount() : count_(SMSCFILELINE) {}
#endif
        std::string toString() const;
        void add( const ExceptionCount& c );
        void count( PvssException::Type et, const char* where );
    private:
        Count  count_;
    };

protected:
    smsc::logger::Logger*                       log_;
    smsc::logger::Logger*                       loge_;

private:
    smsc::core::synchronization::Mutex          startMutex_;
    bool                                        started_;
    SyncDispatcher*                             syncDispatcher_; // not owned

    std::auto_ptr<Acceptor>                     acceptor_;   // owned

    smsc::core::synchronization::EventMonitor   logicMon_;   // monitor on working threads
    std::auto_ptr<AsyncDispatcherThread>        dispatcher_; // owned (created only if asyncdispatcher)
    smsc::core::buffers::Array<Worker*>         workers_;    // owned (created only if syncdispatcher)

    smsc::core::synchronization::EventMonitor   channelMutex_;
    ChannelList                                 channels_;        // channels for r/w
    ChannelList                                 managedChannels_; // channels managed locally
    ChannelList                                 deadChannels_;

    smsc::core::synchronization::EventMonitor   waiter_;
    ContextRegistrySet                          regset_;

    smsc::core::synchronization::EventMonitor   statMutex_;
    Statistics                                  total_;
    Statistics                                  last_;
    bool                                        hasNewStats_;
    std::auto_ptr<ExceptionCount>               exceptions_;
    ExceptionCount                              totalExceptions_;  // counted in main thread
};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCORE_H */
