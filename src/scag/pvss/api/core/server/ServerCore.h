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
    ServerCore( ServerConfig& config, Protocol& protocol );

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

    /**
     * Implementation of Server interface method.</p>
     * Blocks until next request will be received & processed or state will be changed for processing request.
     * Method exits only when PVSS server is completely shutted down.
     * Dequeues next context from states or requests queue.
     *
     * @param logic     Sync server logic interface
     */
    /*
    virtual void waitEvents(SyncLogic logic);
    {
        ServerContext context;
        while ((context = getEvent()) != null)
        {
            switch (context.getState())
            {
            case NEW:
                Response response;
                try { response = logic.process(context.getRequest()); } catch (Exception exc) {
                    response = new ErrorResponse(context.getSeqNum(), Response.StatusType.ERROR, exc.getMessage());
                }
                try { context.setResponse(response); } catch (PvssException exc) {
                    context.setState(ServerContext.State.FAILED);
                    logic.responseFail(response);
                }
                break;
            case SENT:   logic.responseSent(context.getResponse()); break;
            case FAILED: logic.responseFail(context.getResponse()); break;
            default:
                log_.error("Unknown sync context state=" + context.getState());
                break;
            }
        }
    }
     */

    /*
     * Implementation of Server interface method.</p>
     * Blocks until next request will be received or state will be changed for processing request.
     * Dequeues next context from states or requests queue.
     * Method exits only when PVSS server is completely shutted down.
     *
     * @param logic     Async server logic interface
     */
    // virtual void waitEvents(AsyncLogic& logic);
    /*
    virtual void waitEvents(AsyncLogic& logic)
    {
        ServerContext context;
        while ((context = getEvent()) != null)
        {
            switch (context.getState())
            {
            case NEW:    logic.requestReceived(context); break;
            case SENT:   logic.responseSent(context); break;
            case FAILED: logic.responseFail(context); break;
            default:
                log_.error("Unknown async context state=" + context.getState());
                break;
            }
        }
    }
     */

    // private final Object contextsMonitor = new Object();

    /*
     * Method returns next context when it new or it's state was changed.<p/>
     * Uses pendingStates & pendingContexts queues. Blocks while no contexts available.<p/>
     * After shutdown method returns all from pendingStates queue & ignors new context,
     * for the rest of pendingContexts queue fail-responses will be generated.
     *
     * @return  next context (new or when it states has changed),
     *          null when PVSS server is shutting down & no any changed states rest.
     */
    /*
    private ServerContext getEvent()
    {
        synchronized (contextsMonitor)
        {
            while (started || !pendingStates.isEmpty() || !pendingContexts.isEmpty() || hasProcessingContexts())
            {
                if (!pendingStates.isEmpty()) // process all states first
                    return pendingStates.pollFirst();

                if (!pendingContexts.isEmpty()) { // process new requests
                    ServerContext context = pendingContexts.pollFirst();
                    registerProcessingContext(context);
                    return context;
                }

                try { contextsMonitor.wait(started ? config.getProcessTimeout() : 10); }
                catch (InterruptedException e) { log_.warn("Wait on contexts monitor interrupted", e); }
            }
        }
        return null; // break only when no one context rest
    }
     */

    /*
     * Used to send response or error for processed request.
     * Invoked from worker thread.
     * Puts response to Writer queue for channel on wich original request was received.
     *
     * @param context           ServerContext instance containing processed request with response or error set.
     * @throws PvssException    Thrown if provided context is invalid or server failes to sent it.
     */
    void contextProcessed(std::auto_ptr<ServerContext> context); // /* throw(PvssException) */ ;

    /*
    {
        PvssSocket& channel = context.getChannel();
        checkProcessingContext(channel, context.getSeqNum()); // check registration only, don't remove!
        sendResponse(context.getResponse(), channel);
    }
     */


    /*
    private void registerProcessingContext(ServerContext context)
    {
        synchronized (processingContexts)
        {
            processingContexts.addLast(context);
            final SocketChannel channel = context.getChannel();
            synchronized (processingContextsMap) {
                HashMap<Integer, ServerContext> map = processingContextsMap.get(channel);
                if (map == null) {
                    map = new HashMap<Integer, ServerContext>();
                    processingContextsMap.put(channel, map);
                }
                map.put(context.getSeqNum(), context);
            }
            processingContexts.notify();
        }
    }
    private ServerContext removeProcessingContext(SocketChannel channel, int seqNum, boolean checkState)
    {
        ServerContext context = null;
        synchronized (processingContextsMap) {
            HashMap<Integer, ServerContext> map = processingContextsMap.get(channel);
            if (map != null)
            {
                if (checkState) {
                    context = map.get(seqNum);
                    if (context.getState() == ServerContext.State.PROCESSED)
                        return null; // do not remove, wait response report from Writers
                }
                context = map.remove(seqNum);
                if (map.isEmpty()) processingContextsMap.remove(channel);
            }
        }
        return context;
    }
    private void checkProcessingContext(SocketChannel channel, int seqNum) throws PvssException
    {
        synchronized (processingContextsMap) {
            HashMap<Integer, ServerContext> map = processingContextsMap.get(channel);
            if (map == null || !map.containsKey(seqNum))
                throw new PvssException(PvssException.Type.UNEXPECTED_RESPONSE,
                                        "Request expired, error response was already sent");
        }
    }
    private boolean hasProcessingContexts()
    {
        synchronized (processingContextsMap) {
            return !processingContextsMap.isEmpty();
        }
    }
     */

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


    /*
    {
        try
        {
            if (state == Core.PacketState.RECEIVED) // Response received
            {
                if (packet == null || !(packet instanceof Request) || !packet.isValid())
                    throw new PvssException(PvssException.Type.BAD_REQUEST,
                                            "Received packet isn't valid PVAP request");
                handleRequest((Request)packet, channel);
            }
            else // EXPIRED | FAILED | SENT Response notification
            {
                if (packet == null || !(packet instanceof Response) || !packet.isValid())
                    throw new PvssException(PvssException.Type.BAD_RESPONSE,
                                            "Handled packet isn't valid PVAP response");
                reportResponse((Response)packet, channel, state);
            }
        }
        catch (PvssException exc) {
            log_.error(exc.getMessage(), exc); // Do not close channel
        }
    }
     */



    /**
     * Implementation of Core abstraction method.
     * Invoked from writer to notify core api of a packet state change (sent,failed,expired).
     *
     * @param seqnum    seqnum of processed packetProcessing packet (request or response)
     * @param channel   Channel on which packet is processing
     * @param state     Packet state in IO processing
     */
    virtual void reportPacket(uint32_t seqnum, smsc::core::network::Socket& channel, PacketState state);



    /*
    private void handleRequest(Request request, SocketChannel channel) throws PvssException
    {
        updateChannelActivity(channel); // Update last activity time on channel

        if (!started) {
            sendResponse(new ErrorResponse(request.getSeqNum(),
                                           Response.StatusType.SERVER_SHUTDOWN, null), channel);
            return;
        }

        if (request instanceof PingRequest) {
            PingResponse ping = new PingResponse(request.getSeqNum());
            ping.setStatus(Response.StatusType.OK);
            sendResponse(ping, channel);
        } 
        else {
            boolean needErrorResponse = false;
            final int pendingContextSize;
            synchronized (contextsMonitor) {
                pendingContextSize = pendingContexts.size();
                if (pendingContextSize < ((ServerConfig)config).getQueueSizeLimit()) {
                    pendingContexts.addLast(new ServerContext(request, channel, this));
                    contextsMonitor.notifyAll();
                } else needErrorResponse = true;
            }
            if (needErrorResponse) {
                log_.warn("Pending requests queue overflow, queue size=" + pendingContextSize);
                sendResponse(new ErrorResponse(request.getSeqNum(),
                                               Response.StatusType.SERVER_BUSY, "Try later"), channel);
            }
        }
    }
     */

    /*
    private void reportResponse(Response response, SocketChannel channel, PacketState state)
    {
        if (response instanceof PingResponse) log_.warn("PING response state=" + state);
        else if (response instanceof ErrorResponse) log_.warn("ERROR response state=" + state);
        else
        {
            if (log_.isDebugEnabled()) log_.debug("Response '" + response + "' state=" + state);
            ServerContext context = removeProcessingContext(channel, response.getSeqNum(), false);
            if (context != null) {
                synchronized (contextsMonitor) {
                    context.setState((state == PacketState.SENT) ?
                                     ServerContext.State.SENT : ServerContext.State.FAILED);
                    pendingStates.addLast(context);
                    contextsMonitor.notifyAll();
                }
            } // else context not found => already expired
        }
    }

     */


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
    void checkStatistics();

private:
    typedef std::list< smsc::core::network::Socket* > ChannelList;

    class ExceptionCount
    {
        typedef smsc::core::buffers::IntHash< smsc::core::buffers::Hash< unsigned > > Count;
    public:
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
