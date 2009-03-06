#include "ServerCore.h"
#include "ContextQueue.h"
#include "ServerNewContext.h"
#include "Worker.h"
#include "scag/pvss/api/packets/PingResponse.h"
#include "scag/pvss/api/packets/ErrorResponse.h"

namespace scag2 {
namespace pvss {
namespace core {
namespace server {


ServerCore::ServerCore( ServerConfig& config, Protocol& protocol ) :
Core(config,protocol), Server(),
log_(smsc::logger::Logger::getInstance(taskName())),
started_(false),
syncDispatcher_(0),
asyncDispatcher_(0)
{
}


ServerCore::~ServerCore()
{
    shutdown();
}


bool ServerCore::acceptChannel( PvssSocket* channel )
{
    regset_.create(channel->socket());
    bool accepted = Core::registerChannel(*channel,util::currentTimeMillis());
    if ( accepted ) {
        MutexGuard mg(channelMutex_);
        channels_.push_back(channel->socket());
        managedChannels_.push_back(channel->socket());
    } else {
        regset_.destroy(channel->socket());
    }
    return accepted;
}


void ServerCore::contextProcessed(std::auto_ptr<ServerContext> context) // throw (PvssException)
{
    try {
        sendResponse(context);
    } catch (PvssException& e) {
        context->setState(ServerContext::FAILED);
        reportContext(context);
    }
}


void ServerCore::receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel )
{
    try {
        if ( !packet.get() || !packet->isRequest() || !packet->isValid() )
            throw PvssException(PvssException::BAD_REQUEST,"Received packet isnt valid PVAP request");
    } catch ( PvssException& e ) {
        smsc_log_error(log_,"exception: %s",e.what());
        return;
    }

    uint32_t seqNum = packet->getSeqNum();
    updateChannelActivity(channel);
    Response::StatusType status = Response::OK;
    Request* req = static_cast<Request*>(packet.release());
    smsc_log_debug(log_,"packet received %p: %s", req, req->toString().c_str());
    std::auto_ptr<ServerContext> ctx(new ServerNewContext(req,channel));
    try {
        if (!started_) {
            status = Response::SERVER_SHUTDOWN;
            throw PvssException(PvssException::SERVER_BUSY,"Server is down");
        }

        if ( req->isPing() ) {
            seqNum = uint32_t(-1);
            ctx->setResponse(new PingResponse(ctx->getSeqNum(),Response::OK));
            sendResponse(ctx);
            return;
        }

        ContextQueue* queue = 0;
        if ( syncDispatcher_ ) {
            const int idx = syncDispatcher_->getIndex(*req);
            if ( idx > workers_.Count() ) {
                status = Response::ERROR;
                throw PvssException(PvssException::UNKNOWN,"cannot dispatch");
            }
            Worker* worker = workers_[idx];
            queue = & worker->getQueue();
        } else if ( asyncDispatcher_ ) {
            queue = & dispatcher_->getQueue();
        } else {
            status = Response::ERROR;
            throw PvssException(PvssException::UNKNOWN,"dispatcher is not found");
        }

        status = Response::SERVER_BUSY;
        // ctx->setRespQueue( *respQueue );
        queue->requestReceived(ctx); // may throw server_busy
        // seqNum = uint32_t(-1);

    } catch (PvssException& e) {
        smsc_log_error(log_, "exception: %s",e.what());
        try {
            ctx->setResponse(new ErrorResponse(seqNum,status,e.what()));
            sendResponse(ctx);
        } catch (PvssException& e) {
            smsc_log_error(log_,"exception: %s", e.what());
        }
    }
}


void ServerCore::reportPacket(uint32_t seqNum, PvssSocket& channel, PacketState state)
{
    std::auto_ptr<ServerContext> ctx;
    Response* response = 0;
    {
        ContextRegistry::Ptr ptr(regset_.get(channel.socket()));
        if (!ptr) {
            smsc_log_warn(log_,"packet seqNum=%d on channel %p reported, but registry is not found",
                          seqNum, &channel);
            return;
        }
        ContextRegistry::Ctx i(ptr->get(seqNum));
        if ( !i ) {
            smsc_log_warn(log_, "packet seqNum=%d on channel %p reported as %s, but not found",
                          seqNum, &channel, state == SENT ? "SENT" : (state == FAILED ? "FAILED" : "EXPIRED"));
            return;
        }
        response = i.getContext()->getResponse().get();
        assert(response);
        ctx.reset(static_cast<ServerContext*>(ptr->pop(i)));
        switch (state) {
        case SENT : ctx->setState( ServerContext::SENT ); break;
        case EXPIRED : ctx->setState( ServerContext::FAILED ); break;
        case FAILED : ctx->setState( ServerContext::FAILED ); break;
        default: ctx->setState( ServerContext::FAILED ); break;
        }
    }
    reportContext(ctx);
}


void ServerCore::startup( SyncDispatcher& dispatcher ) throw (PvssException)
{
    if (started_) return;
    MutexGuard mg(startMutex_);
    if (started_) return;

    syncDispatcher_ = &dispatcher;

    startupIO();
    try {
        // create workers
        for (unsigned i = 0; ; ++i) {
            SyncLogic* logic = dispatcher.getSyncLogic(i);
            if (!logic) break;
            Worker* worker = new Worker(*logic,*this);
            workers_.Push(worker);
            threadPool_.startTask(worker,false);
        }
        acceptor_.reset(new Acceptor(getConfig(),*this));
        acceptor_->init();
        threadPool_.startTask(acceptor_.get(),false);
    } catch (PvssException& exc) {
        smsc_log_error(log_,"Acceptor start error: %s",exc.what());
        if (acceptor_.get()) acceptor_->shutdown();
        shutdownIO();
        throw exc;
    }

    started_ = true;
    threadPool_.startTask(this,false);
}


void ServerCore::shutdown()
{
    if (!started_) return;
    MutexGuard mg(startMutex_);
    if (!started_) return;

    smsc_log_info(log_, "Server is shutting down...");

    started_ = false;
    if (acceptor_.get()) acceptor_->shutdown();
    shutdownIO();
    stop();
    {
        smsc_log_info(log_,"sending notify to channel mutex");
        MutexGuard mg(channelMutex_);
        channelMutex_.notify();
    }
    // stop all workers
    for ( int i = 0; i < workers_.Count(); ++i ) {
        workers_[i]->shutdown();
    }
    if ( dispatcher_.get() ) dispatcher_->shutdown();
    waitUntilReleased();
}


int ServerCore::Execute()
{
    const int minTimeToSleep = 10; // 10 msec
    util::msectime_type timeToSleep = getConfig().getProcessTimeout();
    util::msectime_type currentTime = util::currentTimeMillis();
    util::msectime_type nextWakeupTime = currentTime + timeToSleep;

    smsc_log_info(log_,"Server started");
    while (!isStopping)
    {
        // smsc_log_debug(log_,"cycling clientCore");
        currentTime = util::currentTimeMillis();
        int timeToWait = int(nextWakeupTime-currentTime);

        ChannelList currentChannels;
        {
            MutexGuard mg(channelMutex_);
            if ( timeToWait < minTimeToSleep ) timeToWait = minTimeToSleep;
            // smsc_log_debug(log_,"timetowait: %d", int(timeToWait));
            channelMutex_.wait(int(timeToWait));
            if (isStopping) break;
            std::copy(channels_.begin(), channels_.end(),
                      std::back_inserter(currentChannels));
        }
        currentTime = util::currentTimeMillis();
        nextWakeupTime = currentTime + timeToSleep;

        smsc_log_debug(log_,"processing expired contexts for %d channels",unsigned(currentChannels.size()));
        for ( ChannelList::const_iterator j = currentChannels.begin();
              j != currentChannels.end(); ++j ) {
            
            smsc::core::network::Socket* channel = *j;
            ContextRegistry::ProcessingList list;
            {
                ContextRegistry::Ptr ptr = regset_.get(channel);
                if (!ptr) continue;
                util::msectime_type t = ptr->popExpired(list,currentTime,timeToSleep);
                if ( t < nextWakeupTime ) nextWakeupTime = t;
            }
            // fixme: process those items in list
            std::auto_ptr< ServerContext > context;
            for ( ContextRegistry::ProcessingList::iterator i = list.begin();
                  i != list.end();
                  ++i ) {
                // expired
                ServerContext* ctx = static_cast<ServerContext*>(*i);
                if (ctx->getRequest()->isPing()) {
                    smsc_log_warn(log_,"PING failed, timeout");
                    closeChannel(channel);
                    delete ctx;
                } else {
                    ctx->setError("timeout");
                    context.reset(ctx);
                    reportContext(context);
                }
            }
        }
    }
    smsc_log_info( log_, "Server shutdowned" );
    return 0;
}


void ServerCore::sendResponse( std::auto_ptr<ServerContext>& context ) throw (PvssException)
{
    int seqNum = context->getSeqNum();
    smsc::core::network::Socket* socket = context->getSocket();
    const Packet* packet = context->getResponse().get();
    ServerContext* ctx = 0;
    if (!packet) throw PvssException(PvssException::BAD_RESPONSE,"response is null");
    {
        ContextRegistry::Ptr ptr = regset_.get(socket);
        if (!ptr)
            throw PvssException(PvssException::UNKNOWN,
                                "context registry is not found");
        if ( ptr->exists(seqNum) )
            throw PvssException(PvssException::SERVER_BUSY,
                                "seqnum=%d is already in registry", seqNum );
        if ( !packet->isValid() )
            throw PvssException(PvssException::BAD_RESPONSE,"response '%s' is not valid",packet->toString().c_str());
        ctx = context.release();
        ptr->push(ctx);
    }
    try {
        ctx->sendResponse();
    } catch (...) {
        ContextRegistry::Ptr ptr = regset_.get(socket);
        context.reset( static_cast<ServerContext*>(ptr->pop(ptr->get(seqNum))));
        // logic should be notified externally as it may take ctx ownership
        throw;
    }
}


void ServerCore::reportContext( std::auto_ptr<ServerContext> ctx )
{
    if ( !ctx.get() ) {
        smsc_log_warn(log_,"null context reported");
        return;
    }

    Response* response = ctx->getResponse().get();
    if ( !response ) {
        smsc_log_warn(log_,"context with null response reported");
        return;
    }

    uint32_t seqNum = ctx->getSeqNum();

    if ( ctx->getState() == ServerContext::SENT ) {
        if ( response->isPing() ) {
            smsc_log_debug(log_,"PING response sent");
            return;
        } else {
            smsc_log_debug(log_,"Response '%s' sent",response->toString().c_str());
            if ( response->isError() ) return;
        }
    } else {
        // failure
        if ( response->isPing() ) {
            smsc_log_debug(log_,"PING response was not sent" );
            return;
        } else {
            smsc_log_debug(log_,"Response '%s' was not sent",response->toString().c_str());
            if ( response->isError() ) return;
        }
    }

    ContextQueue* queue = ctx->getRespQueue();
    if ( !queue ) {
        smsc_log_warn(log_,"packet seqNum=%d has no resp queue set",seqNum);
    } else {
        queue->reportResponse(ctx);
    }
}


void ServerCore::closeChannel( smsc::core::network::Socket* socket )
{
    ChannelList::iterator i;
    {
        MutexGuard mg(channelMutex_);
        i = std::find( managedChannels_.begin(),
                       managedChannels_.end(),
                       socket );
        if ( i == managedChannels_.end() ) {
            // should we close it?
            return;
        }
        managedChannels_.erase(i);
        i = std::find(channels_.begin(),channels_.end(),socket);
        assert(i != channels_.end());
        deadChannels_.push_back(*i);
        channels_.erase(i);
    }
    PvssSocket* channel = PvssSocket::fromSocket(socket);
    Core::closeChannel(*channel);
    // should we destroy dead channels?
    destroyDeadChannels();
}


void ServerCore::stopCoreLogic()
{
    // FIXME: send signals to all queue, wait until all workers are finished.
    smsc_log_info(log_,"signalling worker thread queues");
    for ( int i = 0; i < workers_.Count(); ++i ) {
        workers_[i]->getQueue().stop();
    }
    if ( dispatcher_.get() ) {
        dispatcher_->getQueue().stop();
    }
    smsc_log_info(log_,"waiting until all workers threads begin stopping");
    while (true) {
        MutexGuard mg(logicMon_);
        int workingCount = 0;
        for ( int i = 0; i < workers_.Count(); ++i ) {
            if ( ! workers_[i]->stopping() ) ++workingCount;
        }
        if ( dispatcher_.get() && !dispatcher_->stopping() ) ++workingCount;
        if ( workingCount == 0 ) break;
        logicMon_.wait(200);
    }

    // all workers and async logic dispatcher are stopping,
    // i.e. they have no pending processing requests and async logic is also stopped.
    // Now we have to wait until all pending outgoing contexts are written to their sockets.
    smsc_log_info(log_,"waiting until registries are empty");
    regset_.waitUntilEmpty();
    smsc_log_info(log_,"registries are empty");
}


void ServerCore::destroyDeadChannels()
{
    ChannelList trulyDead;
    {
        MutexGuard mg(channelMutex_);
        for ( ChannelList::iterator i = deadChannels_.begin();
              i != deadChannels_.end();
              ) {
            PvssSocket* socket = PvssSocket::fromSocket(*i);
            if ( ! socket->isInUse() ) {
                trulyDead.push_back(*i);
                i = deadChannels_.erase(i);
            } else {
                ++i;
            }
        }
    }
    for ( ChannelList::iterator i = trulyDead.begin();
          i != trulyDead.end();
          ++i ) {
        regset_.destroy(*i);
    }
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
