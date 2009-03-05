#include "ClientCore.h"
#include "Connector.h"
#include "scag/pvss/api/packets/Response.h"
#include "scag/pvss/api/packets/PingRequest.h"

using namespace smsc::core::synchronization;

namespace {

using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::client;

class Handler : public Client::ResponseHandler
{
public:
    Handler( std::auto_ptr<Request>& originalRequest ) : request_(originalRequest) {}

    std::auto_ptr<PvssException>& getError() {
        return error_;
    }
    std::auto_ptr<Request>& getRequest() {
        return request_;
    }
    std::auto_ptr<Response>& getResponse() {
        return response_;
    }
    virtual void handleResponse( std::auto_ptr<Request> request, std::auto_ptr<Response> response ) {
        MutexGuard mg(mon_);
        mon_.notifyAll();
        request_ = request;
        response_ = response;
    }
    virtual void handleError(const PvssException& e, std::auto_ptr<Request> request) {
        MutexGuard mg(mon_);
        mon_.notifyAll();
        request_ = request;
        error_.reset( new PvssException(e) );
    }

    void wait() {
        MutexGuard mg(mon_);
        mon_.wait();
    }

private:
    smsc::core::synchronization::EventMonitor mon_;
    std::auto_ptr<Request>&      request_;     // note the reference!
    std::auto_ptr<Response>      response_;
    std::auto_ptr<PvssException> error_;
};

}


namespace scag2 {
namespace pvss {
namespace core {
namespace client {


smsc::logger::Logger* ClientCore::logger = 0;

ClientCore::ClientCore( ClientConfig& config, Protocol& proto ) :
Core(config,proto),
connector_(0),
lastUsedSeqNum_(0),
started_(false)
{
    if ( ! logger ) {
        static Mutex logMutex;
        MutexGuard mg(logMutex);
        if ( ! logger ) logger = smsc::logger::Logger::getInstance("pvss.core");
    }
}


ClientCore::~ClientCore()
{
    shutdown();
}


void ClientCore::startup() throw(PvssException)
{
    if (started_) return;
    MutexGuard mg(startMutex_);
    if (started_) return;

    smsc_log_info(logger,"Client starting...");

    startupIO();
    try {
        // connector = new Connector( new InetSocketAddress(config.getHost(), config.getPort()) );
        connector_.reset(new Connector( getConfig(), *this ));
        threadPool_.startTask(connector_.get(),false);
    } catch (PvssException& exc) {
        smsc_log_error( logger, "Connector start error: %s", exc.what());
        shutdownIO();
        throw exc;
    }

    started_ = true;

    try
    {
        // MutexGuard mgc(channelMutex);
        const util::msectime_type currentTime = util::currentTimeMillis();
        smsc_log_info( logger, "Creating connections..." );
        for ( int i=0; i< getConfig().getConnectionsCount(); i++)
            createChannel(currentTime);
    }
    catch (PvssException& exc) {
        started_ = false;
        connector_->shutdown();
        clearChannels();
        shutdownIO();
        throw exc;
    }
    
    threadPool_.startTask( this, false );
    smsc_log_info( logger, "Client started" );
}


void ClientCore::shutdown()
{
    if (!started_) return;
    MutexGuard mg(startMutex_);
    if (!started_) return;

    smsc_log_info( logger, "Client is shutting down...");

    started_ = false;
    connector_->shutdown();
    clearChannels();
    shutdownIO();
    stop();
    {
        MutexGuard mgr(channelMutex_);
        channelMutex_.notify();
    }
    waitUntilReleased();
    destroyDeadChannels();
}


void ClientCore::closeChannel( PvssSocket& channel )
{
    Core::closeChannel( channel );
    connector_->unregisterChannel(channel);
    {
        // move the channel to a dead channel queue
        MutexGuard mgc(channelMutex_);
        ChannelList::iterator i = std::find(activeChannels_.begin(),
                                            activeChannels_.end(),
                                            &channel);
        if (i != activeChannels_.end()) activeChannels_.erase(i);
        i = std::find(channels_.begin(), channels_.end(), &channel);
        if (i != channels_.end()) {
            deadChannels_.push_back(*i);
            channels_.erase(i);
        }
    }
    destroyDeadChannels();

    if (started_) {
        smsc_log_debug(logger,"Recreating channel...");
        try {
            createChannel( util::currentTimeMillis() + getConfig().getConnectTimeout());
        }
        catch (PvssException& create_exc) {
            smsc_log_warn(logger,"Channel recreation failed. Details: %s", create_exc.what());
        }
    }
}


std::auto_ptr<Response> ClientCore::processRequestSync( std::auto_ptr<Request>& request ) throw(PvssException)
{
    Handler handler(request);
    std::auto_ptr<ClientContext> context( new ClientContext(request.release(),&handler) );
    try {
        sendRequest(context);
        handler.wait();
    } catch ( ... ) {
        // exception until queuing, restore original request.
        request = context->getRequest();
        throw;
    }
    // otherwise the original request is already restored
    if ( handler.getError().get() != 0) throw *(handler.getError().get());
    return handler.getResponse();
}


void ClientCore::processRequestAsync( std::auto_ptr<Request>& request, Client::ResponseHandler& handler) throw(PvssException)
{
    std::auto_ptr<ClientContext> context( new ClientContext(request.release(),&handler) );
    try {
        sendRequest(context);
    } catch (...) {
        request = context->getRequest();
        throw;
    }
}


void ClientCore::receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel )
{
    try
    {
        if (packet.get() == 0 || packet->isRequest() || !packet->isValid()) {
            throw PvssException( PvssException::BAD_RESPONSE,
                                 "Received packet isn't valid PVAP response" );
        }
        handleResponse( static_cast<Response*>(packet.release()), channel );
    }
    catch (PvssException& exc) {
        smsc_log_error( logger, "handle exception: %s", exc.what()); // Do not close channel
    }
}


void ClientCore::reportPacket( uint32_t seqNum, PvssSocket& channel, PacketState state )
{
    std::auto_ptr<ClientContext> ctx;
    Request* request = 0;
    {
        ContextRegistry::Ptr ptr(regset_.get(channel.socket()));
        if ( !ptr ) {
            smsc_log_warn(logger,"packet seqNum=%d on channel %p reported, but registry is not found",
                          seqNum, &channel);
            return;
        }
        /*
        MutexGuard mgr(processingRequestMon);
        ProcessingRequestMap* map = processingRequestMaps.Get(&channel);
        ProcessingRequestList::iterator* i = map->GetPtr(seqNum);
         */
        ContextRegistry::Ctx i(ptr->get(seqNum));
        if ( !i ) {
            smsc_log_warn(logger, "packet seqNum=%d on channel %p reported as %s, but not found",
                          seqNum, &channel, state == SENT ? "SENT" : (state == FAILED ? "FAILED" : "EXPIRED"));
            return;
        }
        request = i.getContext()->getRequest().get();
        if ( state == SENT ) {
            if (request->isPing())
                smsc_log_debug(logger,"PING request sent");
            else
                smsc_log_debug(logger,"Request '%s' sent",request->toString().c_str());
            // do nothing, waiting for response
            return;
        } else {
            ctx.reset(static_cast<ClientContext*>(ptr->pop(i)));
        }
    }
    try {
        if ( state == EXPIRED )
            throw PvssException(PvssException::REQUEST_TIMEOUT,"timeout");
        if ( state == FAILED )
            throw PvssException(PvssException::IO_ERROR,"failed to send request");
    } catch (PvssException& exc) {
        if (request->isPing()) {
            smsc_log_warn(logger,"PING failed, details: %s", exc.what());
            closeChannel(channel);
        } else {
            ctx->setError(exc);
        }
    }
}


int ClientCore::Execute()
{
    const int minTimeToSleep = 10; // 10 msec

    util::msectime_type timeToSleep = getConfig().getProcessTimeout();
    util::msectime_type currentTime = util::currentTimeMillis();
    util::msectime_type nextWakeupTime = currentTime + timeToSleep;

    smsc_log_info(logger,"Client started");
    while (started_)
    {
        // smsc_log_debug(logger,"cycling clientCore");
        currentTime = util::currentTimeMillis();
        int timeToWait = int(nextWakeupTime-currentTime);
        if ( timeToWait > 0 ) {
            if ( timeToWait < minTimeToSleep ) timeToWait = minTimeToSleep;
            regset_.wait(timeToWait);
        }
        nextWakeupTime = currentTime + timeToSleep;

        ChannelList currentChannels;
        {
            MutexGuard mgc(channelMutex_);
            std::copy(activeChannels_.begin(), activeChannels_.end(),
                      std::back_inserter(currentChannels));
        }
        for ( ChannelList::const_iterator j = currentChannels.begin();
              j != currentChannels.end(); ++j ) {
            
            PvssSocket* channel = *j;
            ContextRegistry::ProcessingList list;
            {
                ContextRegistry::Ptr ptr = regset_.get(channel->socket());
                if (!ptr) continue;
                util::msectime_type t = ptr->popExpired(list,currentTime,timeToSleep);
                if ( t < nextWakeupTime ) nextWakeupTime = t;
            }
            // fixme: process those items in list
            for ( ContextRegistry::ProcessingList::iterator i = list.begin();
                  i != list.end();
                  ++i ) {
                // expired
                ClientContext* ctx = static_cast<ClientContext*>(*i);
                if (ctx->getRequest()->isPing()) {
                    smsc_log_warn(logger,"PING failed, timeout");
                    closeChannel(*channel);
                } else {
                    ctx->setError(PvssException(PvssException::REQUEST_TIMEOUT,"timeout"));
                }
                delete ctx;
            }
        }
    }
    smsc_log_info( logger, "Client shutdowned" );
    return 0;
}


void ClientCore::clearChannels()
{
    // NOTE: connector must be stopped already!
    assert(connector_->released());
    MutexGuard mg(channelMutex_);
    for ( ChannelList::iterator i = channels_.begin(); i != channels_.end(); ++i ) {
        PvssSocket& channel = **i;
        Core::closeChannel(channel);
        deadChannels_.push_back(*i);
    }
    channels_.clear();
    activeChannels_.clear();
}


void ClientCore::destroyDeadChannels()
{
    ChannelList trulyDead;
    {
        MutexGuard mg(channelMutex_);
        for ( ChannelList::iterator i = deadChannels_.begin();
              i != deadChannels_.end();
              ) {
            if ( ! (*i)->isInUse() ) {
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
        PvssSocket* channel = *i;
        regset_.destroy(channel->socket());
        /*
         * FIXME: think on how to convey requests on closed channel
        for ( ProcessingRequestList::iterator j = (*plist)->begin();
              j != (*plist)->end();
              ++j ) {
            ClientContext* ctx = *j;
            Request* req = ctx->getRequest().get();
            if ( req && req->isPing() ) {
                smsc_log_debug(logger,"PING request on closed channel");
            } else {
                ctx->setError(PvssException(PvssException::IO_ERROR,"channel was closed"));
            }
            delete ctx;
        }
        delete *plist;
         */
    }
}


void ClientCore::createChannel( util::msectime_type startConnectTime )
{
    if (!started_) return;
    PvssSocket* channel(new PvssSocket(getConfig().getHost(),
                                       getConfig().getPort(),
                                       getConfig().getConnectTimeout()/1000));
    smsc_log_info(logger,"creating a channel %p on %s:%d tmo=%d",channel,
                  getConfig().getHost().c_str(), getConfig().getPort(),getConfig().getConnectTimeout()/1000);
    regset_.create(channel->socket());
    {
        MutexGuard mg(channelMutex_);
        channels_.push_back(channel);
    }
    connector_->connectChannel(*channel,startConnectTime);
}


PvssSocket& ClientCore::getNextChannel() throw (PvssException)
{
    MutexGuard mg(channelMutex_);
    if ( activeChannels_.empty() )
        throw PvssException(PvssException::NOT_CONNECTED,"No one channel is connected");
    PvssSocket* channel = activeChannels_.front();
    activeChannels_.pop_front();
    activeChannels_.push_back(channel);
    return *channel;
}


void ClientCore::sendRequest(std::auto_ptr<ClientContext>& context) throw(PvssException)
{
    if (isStopping)
        throw PvssException(PvssException::NOT_CONNECTED, "Client deactivated");

    int seqNum = getNextSeqNum();
    context->setSeqNum(seqNum);
    PvssSocket& channel = getNextChannel();
    const Packet* packet = context->getRequest().get();
    {
        ContextRegistry::Ptr ptr = regset_.get(channel.socket());
        if (!ptr) {
            throw PvssException(PvssException::UNKNOWN,
                                "context registry is not found");
        }
        if ( ptr->exists(seqNum) )
            throw PvssException(PvssException::CLIENT_BUSY,
                                "Possible seqNum overrrun! seqNum=%d is still in use", seqNum);
        if ( ! packet->isValid() )
            throw PvssException(PvssException::BAD_REQUEST, "Request is bad formed");
        ptr->push( context.release() );
    }
    try {
        channel.send(packet,true,false);
    } catch (...) {
        // restore context
        ContextRegistry::Ptr ptr = regset_.get(channel.socket());
        context.reset( static_cast<ClientContext*>(ptr->pop(ptr->get(seqNum))));
        throw;
    }
}


void ClientCore::sendPingRequest( PvssSocket& channel )
{
    smsc_log_debug(logger,"Sending PING");
    try {
        std::auto_ptr<ClientContext> ptr(new ClientContext(new PingRequest(),0));
        sendRequest(ptr);
    } catch (PvssException& exc) {
        smsc_log_warn( logger, "PING send failed: %s", exc.what());
    }
}


void ClientCore::handleResponse(Response* response,PvssSocket& channel) throw(PvssException)
{
    std::auto_ptr<Response> resp(response);
    updateChannelActivity(channel); // Update last activity time on channel (for PINGs generation)
    
    std::auto_ptr<ClientContext> context;
    int seqNum = response->getSeqNum();
    {
        ContextRegistry::Ptr ptr = regset_.get(channel.socket());
        if ( !ptr )
            throw PvssException(PvssException::UNEXPECTED_RESPONSE,"No context registry found");
        context.reset(static_cast<ClientContext*>(ptr->pop(ptr->get(seqNum))));
    }
    if (! context.get())
        throw PvssException( PvssException::UNEXPECTED_RESPONSE, "No context for seqNum=%d", seqNum);

    Request* request = context->getRequest().get();
    try {
        assert(request);
        if (!response->correspondsRequest(*request))
            throw PvssException(PvssException::UNEXPECTED_RESPONSE,
                                "Response '%s' doesn't corresponds request '%s', seqNum=%d",
                                response->toString().c_str(), request->toString().c_str(), seqNum);
    } catch (PvssException& exc) {
        if (request->isPing())
            smsc_log_warn( logger, "Unexpected response on PING. Details: %s", exc.what());
        else context->setError(exc);
        return;
    }

    if (request->isPing()) {
        if (response->getStatus() != Response::OK) {
            smsc_log_warn( logger, "PING Failed on channel %p", &channel );
            closeChannel(channel);
        } else {
            smsc_log_debug( logger, "PING Ok");
        }
    }
    else context->setResponse(resp); // set response for client
}


void ClientCore::cleanup()
{
    clearChannels();
}


} // namespace client
} // namespace core
} // namespace pvss
} // namespace scag2
