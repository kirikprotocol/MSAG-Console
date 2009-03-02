#ifndef _SCAG_PVSS_CORE_SERVER_SERVERCORE_H
#define _SCAG_PVSS_CORE_SERVER_SERVERCORE_H

namespace scag2 {
namespace pvss {
namespace core {
namespace server {

/// interface
class ServerCore : public Core, public Server
{
protected:
    smsc::logger::Logger* logger;

public:
    /**
     * Constructor creates new ServerCore instance with provided configuration.
     *
     * @param config        Server configuration parameters
     * @param protocol      PVAP protocol serializer/deserializer
     */
    ServerCore( ServerConfig& config, Protocol& protocol );

    virtual ~ServerCore();

    virtual const char* taskName() { return "pvss.score"; }

    /**
     * Implementation of InactivityTracker.Listener method.<p/>
     * Will be called by InactivityTracker when channel remains inactive during specified timeout.<p/>
     * Method implementation initiates channel closing.
     */
    virtual void inactivityTimeoutExpired(PvssSocket& channel) {
        smsc_log_warn(logger,"Inactivity timeout expired for channel %p", &channel);
        closeChannel(channel);
    }

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
                logger.error("Unknown sync context state=" + context.getState());
                break;
            }
        }
    }
     */

    /**
     * Implementation of Server interface method.</p>
     * Blocks until next request will be received or state will be changed for processing request.
     * Dequeues next context from states or requests queue.
     * Method exits only when PVSS server is completely shutted down.
     *
     * @param logic     Async server logic interface
     */
    virtual void waitEvents(AsyncLogic& logic);
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
                logger.error("Unknown async context state=" + context.getState());
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
                catch (InterruptedException e) { logger.warn("Wait on contexts monitor interrupted", e); }
            }
        }
        return null; // break only when no one context rest
    }
     */

    /*
     * Used to send response or error for processed request.
     * Puts response to Writer queue for channel on wich original request was received.
     *
     * @param context           ServerContext instance containing processed request with response or error set.
     * @throws PvssException    Thrown if provided context is invalid or server failes to sent it.
     */
    /*
    protected void contextProcessed(ServerContext context) throws PvssException
    {
        final SocketChannel channel = context.getChannel();
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
            logger.error(exc.getMessage(), exc); // Do not close channel
        }
    }
     */



    /**
     * Implementation of Core abstraction method.
     *
     * @param seqnum    seqnum of processed packetProcessing packet (request or response)
     * @param channel   Channel on which packet is processing
     * @param state     Packet state in IO processing
     */
    virtual void reportPacket(uint32_t seqnum, PvssSocket& channel, PacketState state);



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
                logger.warn("Pending requests queue overflow, queue size=" + pendingContextSize);
                sendResponse(new ErrorResponse(request.getSeqNum(),
                                               Response.StatusType.SERVER_BUSY, "Try later"), channel);
            }
        }
    }
     */

    /*
    private void reportResponse(Response response, SocketChannel channel, PacketState state)
    {
        if (response instanceof PingResponse) logger.warn("PING response state=" + state);
        else if (response instanceof ErrorResponse) logger.warn("ERROR response state=" + state);
        else
        {
            if (logger.isDebugEnabled()) logger.debug("Response '" + response + "' state=" + state);
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

    private void sendResponse(Response response, SocketChannel channel) throws PvssException
    {
        getWriter(channel).send(response, channel, true); // don't check queue limits
    }
     */

    /**
     * Implementation of Server interface method.
     * Starts up PVSS server (acceptor & control thread, readers & writers).
     *
     * @throws PvssException    Thrown if server fails to start
     */
    /*public synchronized */
    virtual void startup() throw(PvssException);
    /*
    {
        if (started) return;

        startupIO();
        try {
            acceptor = new Acceptor(new InetSocketAddress(config.getHost(), config.getPort()));
        } catch (PvssException exc) {
            logger.error("Acceptor start error: " + exc.getMessage());
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
            logger.warn(contextType + " request timeout send failed. Details: " + send_error_exc.getMessage());
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
            logger.warn(contextType + " request cancelled send failed. Details: " + send_error_exc.getMessage());
        }
    }
     */

    /**
     * Implementation of Runable interface method (Thread class).
     * Controls pending & processing queues (request processing timeouts checking).
     */
    virtual int Execute();
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

};

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_SERVER_SERVERCORE_H */
