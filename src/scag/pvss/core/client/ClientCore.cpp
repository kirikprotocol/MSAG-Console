#include "ClientCore.h"
#include "Connector.h"
#include "scag/pvss/packets/Response.h"
#include "scag/pvss/packets/PingRequest.h"

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
connector(0),
lastUsedSeqNum(0),
started(false)
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
    destroyDeadChannels();
}


void ClientCore::startup() throw(PvssException)
{
    if (started) return;
    MutexGuard mg(startMutex);
    if (started) return;

    smsc_log_info(logger,"Client starting...");

    startupIO();
    try {
        // connector = new Connector( new InetSocketAddress(config.getHost(), config.getPort()) );
        connector = new Connector( getConfig(), *this );
        threadPool_.startTask(connector,false);
    } catch (PvssException& exc) {
        smsc_log_error( logger, "Connector start error: %s", exc.what());
        shutdownIO();
        throw exc;
    }

    started = true;

    try
    {
        // MutexGuard mgc(channelMutex);
        const util::msectime_type currentTime = util::currentTimeMillis();
        smsc_log_info( logger, "Creating connections..." );
        for ( int i=0; i< getConfig().getConnectionsCount(); i++)
            createChannel(currentTime);
    }
    catch (PvssException& exc) {
        started = false;
        connector->shutdown();
        clearChannels();
        shutdownIO();
        throw exc;
    }
    
    threadPool_.startTask( this, false );
    smsc_log_info( logger, "Client started" );
}


void ClientCore::shutdown()
{
    if (!started) return;
    MutexGuard mg(startMutex);
    if (!started) return;

    smsc_log_info( logger, "Client is shutting down...");

    started = false;
    connector->shutdown();
    clearChannels();
    shutdownIO();
    stop();
    {
        MutexGuard mgr(channelMutex);
        channelMutex.notify();
    }
    waitUntilReleased();
}


void ClientCore::closeChannel( PvssSocket& channel )
{
    Core::closeChannel( channel );
    connector->unregisterChannel(channel);
    {
        // move the channel to a dead channel queue
        MutexGuard mgc(channelMutex);
        ChannelList::iterator i = std::find(activeChannels.begin(),
                                            activeChannels.end(),
                                            &channel);
        if (i != activeChannels.end()) activeChannels.erase(i);
        i = std::find(channels.begin(), channels.end(), &channel);
        if (i != channels.end()) {
            deadChannels.push_back(*i);
            channels.erase(i);
        }
    }
    destroyDeadChannels();

    if (started) {
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
        MutexGuard mgr(processingRequestMon);
        ProcessingRequestMap* map = processingRequestMaps.Get(&channel);
        ProcessingRequestList::iterator* i = map->GetPtr(seqNum);
        if ( !i ) {
            smsc_log_warn(logger, "packet seqNum=%d on channel %p reported as %s, but not found",
                          seqNum, &channel, state == SENT ? "SENT" : (state == FAILED ? "FAILED" : "EXPIRED"));
            return;
        }
        request = (**i)->getRequest().get();
        if ( state == SENT ) {
            if (request->isPing())
                smsc_log_debug(logger,"PING request sent");
            else
                smsc_log_debug(logger,"Request '%s' sent",request->toString().c_str());
            // do nothing, waiting for response
            return;
        } else {
            ctx.reset(**i);
            map->Delete(seqNum);
            processingRequestLists.Get(&channel)->erase(*i);
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
    while (started)
    {
        smsc_log_debug(logger,"cycling clientCore");
        MutexGuard mgr(processingRequestMon);
        currentTime = util::currentTimeMillis();
        int timeToWait = int(nextWakeupTime-currentTime);
        if ( timeToWait > 0 ) {
            if ( timeToWait < minTimeToSleep ) timeToWait = minTimeToSleep;
            processingRequestMon.wait(timeToWait);
        }
        nextWakeupTime = currentTime + timeToSleep;
        MutexGuard mgc(channelMutex);
        for ( ChannelList::const_iterator j = activeChannels.begin();
              j != activeChannels.end(); ++j ) {
            ProcessingRequestList* list = processingRequestLists.Get(*j);
            ProcessingRequestMap* map = processingRequestMaps.Get(*j);
            ProcessingRequestList::iterator i = list->begin();
            for ( ;
                  i != list->end();
                  ) {
                util::msectime_type expireTime = (*i)->getCreationTime() + timeToSleep;
                if ( currentTime >= expireTime ) {
                    int seqNum = (*i)->getSeqNum();
                    map->Delete(seqNum);
                    i = list->erase(i);
                    // FIXME: report expiration
                } else {
                    if ( expireTime < nextWakeupTime ) nextWakeupTime = expireTime;
                    break;
                }
            }
        }
        if (!started) break;
    }
    smsc_log_info( logger, "Client shutdowned" );
    return 0;
}


void ClientCore::clearChannels()
{
    // NOTE: connector must be stopped already!
    assert(connector->released());
    MutexGuard mg(channelMutex);
    for ( ChannelList::iterator i = channels.begin(); i != channels.end(); ++i ) {
        PvssSocket& channel = **i;
        Core::closeChannel(channel);
        deadChannels.push_back(*i);
    }
    channels.clear();
    activeChannels.clear();
}


void ClientCore::destroyDeadChannels()
{
    ChannelList trulyDead;
    {
        MutexGuard mg(channelMutex);
        for ( ChannelList::iterator i = deadChannels.begin();
              i != deadChannels.end();
              ) {
            if ( ! (*i)->isInUse() ) {
                trulyDead.push_back(*i);
                i = deadChannels.erase(i);
            } else {
                ++i;
            }
        }
    }
    for ( ChannelList::iterator i = trulyDead.begin();
          i != trulyDead.end();
          ++i ) {
        PvssSocket* channel = *i;
        ProcessingRequestList** plist = 0;
        {
            MutexGuard mg(processingRequestMon);
            ProcessingRequestMap** map = processingRequestMaps.GetPtr(channel);
            if (map) {
                processingRequestMaps.Delete(channel);
                delete *map;
            }
            plist = processingRequestLists.GetPtr(channel);
            if (plist) processingRequestLists.Delete(channel);
        }
        if (!plist) {
            smsc_log_warn(logger,"request list for channel %p is not found",channel);
            continue;
        }
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
    }
}


void ClientCore::createChannel( util::msectime_type startConnectTime )
{
    if (!started) return;
    PvssSocket* channel(new PvssSocket(getConfig().getHost(),
                                       getConfig().getPort(),
                                       getConfig().getConnectTimeout()/1000));
    smsc_log_info(logger,"creating a channel %p on %s:%d tmo=%d",channel,
                  getConfig().getHost().c_str(), getConfig().getPort(),getConfig().getConnectTimeout()/1000);
    {
        MutexGuard mg(processingRequestMon);
        if ( ! processingRequestMaps.Exists(channel) ) {
            processingRequestMaps.Insert(channel,new ProcessingRequestMap);
            processingRequestLists.Insert(channel,new ProcessingRequestList);
        }
    }
    {
        MutexGuard mg(channelMutex);
        channels.push_back(channel);
    }
    connector->connectChannel(*channel,startConnectTime);
}


PvssSocket& ClientCore::getNextChannel() throw (PvssException)
{
    MutexGuard mg(channelMutex);
    if ( activeChannels.empty() )
        throw PvssException(PvssException::NOT_CONNECTED,"No one channel is connected");
    PvssSocket* channel = activeChannels.front();
    activeChannels.pop_front();
    activeChannels.push_back(channel);
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
        MutexGuard mgr(processingRequestMon);
        ProcessingRequestMap* map = processingRequestMaps.Get(&channel);
        if ( map->Exist(seqNum) )
            throw PvssException(PvssException::CLIENT_BUSY,
                                "Possible seqNum overrrun! seqNum=%d is still in use", seqNum);
        ProcessingRequestList* list = processingRequestLists.Get(&channel);
        if ( ! packet->isValid() )
            throw PvssException(PvssException::BAD_REQUEST, "Request is bad formed");
        bool wasEmpty = list->empty();
        list->push_back(context.release());
        ProcessingRequestList::iterator i = list->end();
        --i;
        map->Insert(seqNum,i);
        if (wasEmpty) processingRequestMon.notify();
    }
    try {
        channel.send(packet,true,false);
    } catch (...) {
        // restore context
        MutexGuard mgr(processingRequestMon);
        ProcessingRequestMap* map = processingRequestMaps.Get(&channel);
        ProcessingRequestList::iterator* i = map->GetPtr(seqNum);
        assert(i);
        ProcessingRequestList* list = processingRequestLists.Get(&channel);
        context.reset(**i);
        list->erase(*i);
        map->Delete(seqNum);
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
        MutexGuard mgr(processingRequestMon);
        ProcessingRequestMap* map = processingRequestMaps.Get(&channel);
        ProcessingRequestList::iterator* i = map->GetPtr(seqNum);
        if ( i ) {
            ProcessingRequestList* list = processingRequestLists.Get(&channel);
            context.reset(**i);
            list->erase(*i);
            map->Delete(seqNum);
        }
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
