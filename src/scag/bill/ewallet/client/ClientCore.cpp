#include "ClientCore.h"
#include "ClientContext.h"
#include "scag/bill/ewallet/Exception.h"
#include "scag/bill/ewallet/RequestVisitor.h"
#include "scag/bill/ewallet/Open.h"
#include "scag/bill/ewallet/Commit.h"
#include "scag/bill/ewallet/Rollback.h"
#include "scag/util/RelockMutexGuard.h"
// #include "scag/bill/ewallet/Ping.h"

using smsc::core::synchronization::MutexGuard;

namespace {

using namespace scag2::bill::ewallet;

class SeqNumCollector : public RequestVisitor
{
public:
    SeqNumCollector() {
        seqNums_.reserve(40);
    }
    virtual bool visitOpen( Open& o ) {
        seqNums_.push_back(o.getSeqNum());
        return true;
    }
    virtual bool visitCommit( Commit& o ) {
        seqNums_.push_back(o.getSeqNum());
        return true;
    }
    virtual bool visitRollback( Rollback& o ) {
        seqNums_.push_back(o.getSeqNum());
        return true;
    }
    virtual bool visitCheck( Check& o ) {
        return false;
    }

    void reportVisited( smsc::logger::Logger* logger )
    {
        if ( seqNums_.empty() ) return;
        std::string res;
        res.reserve(seqNums_.size()*10);
        char buf[20];
        for ( std::vector<uint32_t>::const_iterator i = seqNums_.begin();
              i != seqNums_.end();
              ++i ) {
            sprintf(buf," %u",*i);
            res.append(buf);
        }
        smsc_log_debug(logger,"contexts to wait for %u:%s",
                       unsigned(seqNums_.size()),res.c_str());
    }

private:
    std::vector<uint32_t> seqNums_;
};

}


namespace scag2 {
namespace bill {
namespace ewallet {
namespace client {

ClientCore::ClientCore( proto::Config* config, Streamer* streamer ) :
Core(config,streamer),
started_(false),
seqNum_(0)
{
}


ClientCore::~ClientCore()
{
    shutdown();
}


void ClientCore::startup()
{
    if (started_) return;
    MutexGuard mg(startMutex_);
    if (started_) return;

    if ( !getConfig().isEnabled() ) {
        smsc_log_warn(log_,"ewallet is disabled from config");
        return;
    }

    smsc_log_info(log_,"Starting Ewallet Client %s", getConfig().toString().c_str() );

    startupIO();
    try {
        connector_.reset( new Connector(*this) );
        threadPool_.startTask( connector_.get(), false );
    } catch ( Exception& exc ) {
        smsc_log_error(log_,"connector start error: %s", exc.what() );
        shutdownIO( false );
        throw exc;
    }

    started_ = true;
    try {
        smsc_log_info( log_, "creating connections..." );
        if ( getConfig().isLoopback() ) {
            // use a loopback connection
            loopback_.reset( new Loopback(*this) );
            regSet_.create(loopback_.get());
            threadPool_.startTask( loopback_->getProcessor(), false );
        } else {
            // use real network connections
            for ( size_t i = 0; i < getConfig().getConnectionsCount(); ++i ) {
                createSocket();
            }
        }
    } catch ( Exception& exc ) {
        started_ = false;
        connector_->shutdown();
        closeAllSockets();
        if ( loopback_.get() ) loopback_->close();
        shutdownIO( false );
        throw exc;
    }

    threadPool_.startTask(this,false);
    smsc_log_info(log_,"Ewallet client started");
}


void ClientCore::shutdown()
{
    if (!started_) return;
    MutexGuard mg(startMutex_);
    if (!started_) return;

    smsc_log_info(log_,"Ewallet client is shutting down...");
    started_ = false;
    connector_->shutdown();
    closeAllSockets();
    if ( loopback_.get() ) loopback_->close();
    shutdownIO( false );
    stop();
    {
        MutexGuard mgr(socketMon_);
        socketMon_.notify();
    }
    waitUntilReleased();
    smsc_log_info(log_,"client thread is stopped, destroying all dead channels");
    destroyDeadSockets();
}


bool ClientCore::canProcessRequest( Exception* exc )
{
    // FIXME
    return false;
}


void ClientCore::processRequest( std::auto_ptr< Request > request, ResponseHandler& handler )
{
    std::auto_ptr< proto::Context > context( new ClientContext(request.release(),&handler) );
    try {
        sendRequest(context);
    } catch ( Exception& e ) {
        handler.handleError(context->getRequest(),e);
        // throw;
    } catch ( std::exception& e ) {
        handler.handleError(context->getRequest(),Exception(e.what(),Status::UNKNOWN));
        // throw;
    } catch (...) {
        handler.handleError(context->getRequest(),Exception("unknown exception",Status::UNKNOWN));
        // throw;
    }
}


void ClientCore::receivePacket( proto::SocketBase& socket, std::auto_ptr<Packet> packet )
{
    std::auto_ptr< ClientContext > context;
    uint32_t seqNum;
    try {
        if ( packet.get() == 0 ) {
            throw Exception( "null packet received", Status::BAD_RESPONSE );
        } else if ( packet->isRequest() ) {
            throw Exception( "not a response received", Status::BAD_RESPONSE );
        }

        socket.updateActivity();

        if ( !packet->isValid() ) {
            throw Exception( "invalid response received", Status::BAD_RESPONSE );
        }

        seqNum = packet->getSeqNum();

        // look for a request in registry
        std::auto_ptr<Response> response(static_cast<Response*>(packet.release()));

        {
            RegistrySet::Ptr ptr = regSet_.get(&socket);
            if ( !ptr ) {
                throw Exception( Status::IO_ERROR, "registry for socket %p is not found", &socket );
            }
            RegistrySet::Ctx ctx(ptr->get(seqNum));
            if ( ! ctx ) {
                throw Exception( Status::TIMEOUT, "seqnum=%u is not found in registry for socket %p, timeouted?", seqNum, &socket );
            }
            if ( ctx->getState() == proto::Context::SENDING ) {
                // we are too fast
                ctx->getResponse().reset(response.release());
                ctx->setState( proto::Context::RECEIVED );
                return;
            }
            if ( ctx->getState() != proto::Context::SENT ) {
                // wrong state
                smsc_log_error(log_,"wrong state: %u in receivePacket", ctx->getState());
                abort();
            }
            context.reset(ptr->pop(ctx));
        }
        context->getResponse().reset(response.release());

    } catch ( Exception& exc ) {
        smsc_log_error(log_, "exception: %s", exc.what() );
        return;
    }

    reportPacket(socket,seqNum,context.release(),proto::Context::DONE);
}


void ClientCore::reportPacket( proto::SocketBase& socket,
                               uint32_t           seqNum,
                               proto::Context* context,
                               proto::Context::ContextState    state )
{
    std::auto_ptr<ClientContext> clientContext(static_cast<ClientContext*>(context));

    smsc_log_debug(log_,"reporting seq=%u ctx=%p on socket=%p state=%u", seqNum, context, &socket, unsigned(state) );

    switch ( state ) {
    case proto::Context::SENDING : {
        if ( !context ) {
            smsc_log_error(log_,"sending should have non-null context");
            return;
        }
        RegistrySet::Ptr ptr(regSet_.get(&socket));
        if ( !ptr ) {
            smsc_log_error(log_,"registry for socket %p is not found", &socket);
            state = proto::Context::FAILED;
            break; // return to core
        }
        if ( ptr->exists(seqNum) ) {
            smsc_log_warn(log_,"seqnum=%u already exists in registry %p", seqNum, &socket );
            return;
        }
        clientContext->setState( proto::Context::SENDING );
        ptr->push(clientContext.release());
        return;
    }

    case proto::Context::SENT : {
        if ( context ) {
            smsc_log_error(log_,"'sent' should not have context");
            abort();
        }
        RegistrySet::Ptr ptr(regSet_.get(&socket));
        if ( !ptr ) {
            smsc_log_error(log_,"registry for socket %p is not found", &socket);
            return;
        }
        RegistrySet::Ctx ctx(ptr->get(seqNum));
        if ( ! ctx ) {
            smsc_log_error(log_,"seqnum=%u already deleted from registry %p", seqNum, &socket );
            return;
        }
        ClientContext* foundContext = ctx.getContext();
        if ( foundContext->getState() == proto::Context::RECEIVED ) {
            // extract context and process it
            clientContext.reset( ptr->pop(ctx) );
            state = proto::Context::DONE;
            break;
        } else if ( foundContext->getState() != proto::Context::SENDING ) {
            smsc_log_error(log_,"seqnum=%u context found in registry %p has wrong state %u", seqNum, &socket, foundContext->getState() );
            abort();
        }

        foundContext->setState( proto::Context::SENT );
        return;
    }
    case proto::Context::DONE : {
        if ( !context ) {
            smsc_log_error(log_,"done should have non-null context");
            return;
        }
        break;
    }
    case proto::Context::EXPIRED : {
        if ( !context ) {
            smsc_log_error(log_,"expired should have non-null context");
            return;
        }
        break;
    }
    case proto::Context::FAILED : {
        if ( !context ) {
            // context is not passed, it must be in registry
            RegistrySet::Ptr ptr(regSet_.get(&socket));
            if ( !ptr ) {
                smsc_log_error(log_,"registry for socket %p is not found", &socket);
                return;
            }
            RegistrySet::Ctx ctx(ptr->get(seqNum));
            if ( ! ctx ) {
                smsc_log_error(log_,"seqnum=%u already deleted from registry %p", seqNum, &socket );
                return;
            }
            clientContext.reset(ptr->pop(ctx));
        }
        break;
    }
    default : {
        smsc_log_error(log_,"state %u should not be passed to reportPacket", state);
        abort();
        return;
    }
    }
        
    // now we have a context with one of the states: DONE, FAILED, EXPIRED
    smsc_log_debug(log_,"setting state %u to context %p", unsigned(state), clientContext.get());
    clientContext->setState( state );
}


void ClientCore::handleError( proto::SocketBase& socket, const Exception& exc )
{
    smsc_log_error(log_,"exception on socket %p: %s", &socket, exc.what() );
    proto::Socket* realSocket = socket.castToSocket();
    if (realSocket) closeSocket(*realSocket);
    else socket.close();
}


bool ClientCore::registerSocket( proto::Socket& socket )
{
    if ( Core::registerSocket(socket) ) {
        MutexGuard mg(socketMon_);
        activeSockets_.push_back(&socket);
        return true;
    }
    return false;
}


int ClientCore::doExecute()
{
    const int minTimeToSleep = 10; // 10 msec

    util::msectime_type timeToSleep = getConfig().getProcessTimeout();
    util::msectime_type currentTime = util::currentTimeMillis();
    util::msectime_type nextWakeupTime = currentTime + timeToSleep;

    smsc_log_info(log_,"Client started");
    while (!isStopping)
    {
        // smsc_log_debug(log_,"cycling %s", taskName());
        currentTime = util::currentTimeMillis();
        int timeToWait = int(nextWakeupTime-currentTime);

        SocketList currentSockets;
        {
            MutexGuard mgc(socketMon_);
            if ( timeToWait > 0 ) {
                if ( timeToWait < minTimeToSleep ) timeToWait = minTimeToSleep;
                socketMon_.wait(timeToWait);
                if (isStopping) break;
            }
            std::copy( activeSockets_.begin(), activeSockets_.end(),
                       std::back_inserter(currentSockets));
        }
        currentTime = util::currentTimeMillis();
        nextWakeupTime = currentTime + timeToSleep;

        for ( SocketList::const_iterator j = currentSockets.begin();
              j != currentSockets.end(); ++j ) {
            
            proto::Socket* socket = *j;
            RegistrySet::ContextList list;
            // SeqNumCollector seqNumCollector;
            {
                RegistrySet::Ptr ptr = regSet_.get(socket);
                if (!ptr) continue;
                util::msectime_type t = ptr->popExpired(list,currentTime,timeToSleep);
                if ( t < nextWakeupTime ) nextWakeupTime = t;

                // it is strange, but we are going to process the contexts until we die.
                // To do that, we move contexts to the non-expirable part of the registry
                // and send check commands.
                // NOT ANYMORE!
                /*
                for ( RegistrySet::ContextList::iterator i = list.begin();
                      i != list.end();
                      ) {
                    ClientContext* ctx = *i;
                    if ( ctx->getHandler() && ctx->getRequest().get() ) {
                        if ( ctx->getRequest()->visit(seqNumCollector) ) {
                            // leave the context in non-expirable part of the registry
                            i = list.erase(i);
                            ctx->makeNonExpirable();
                            ptr->push(ctx);
                            continue;
                        }
                    }
                    ++i;
                }
                 */
            }
            
            // seqNumCollector.reportVisited(log_);

            // process those items in list
            for ( RegistrySet::ContextList::iterator i = list.begin();
                  i != list.end();
                  ++i ) {
                // expired
                ClientContext* ctx = *i;
                if ( ! ctx->getRequest().get() ) {
                    assert(ctx->getResponse().get());
                    smsc_log_warn( log_,"Context w/o request found, resp:%s, created: %d ms ago",
                                   ctx->getResponse()->toString().c_str(),
                                   int(currentTime - ctx->getCreationTime()) );
                } else {
                    /*
                    if ( Request::isPing(*ctx->getRequest().get()) ) {
                        smsc_log_warn(log_,"PING failed, timeout");
                        closeSocket(*socket);
                    } else {
                     */
                    ctx->setError(Exception("timeout",Status::TIMEOUT));
                }
                delete ctx;
            }
        }
    }
    smsc_log_info( log_, "ClientCore::Execute finished" );
    return 0;
}


void ClientCore::inactivityTimeout( proto::Socket& socket )
{
    /*
    smsc_log_debug(log_,"sending Ping");
    try {
        std::auto_ptr<proto::Context> context(new ClientContext(new Ping,0));
        socket.send( context, true );
    } catch ( Exception& e ) {
        smsc_log_warn(log_,"PING send failed: %s", e.what() );
    }
     */
}


void ClientCore::sendRequest( std::auto_ptr< proto::Context >& context )
{
    try {
        if ( stopping() )
            throw Exception( "client deactivated", Status::NOT_CONNECTED );
        uint32_t seqNum = getNextSeqNum();
        context->setSeqNum(seqNum);

        // FIXME: temporary sent to a loopback queue
        // if ( loopback_.get() ) loopback_->send( context, true );

        smsc_log_debug(log_,"setting seqnum %u to context",seqNum);
        proto::SocketBase& socket = getNextSocket( context->getRequest().get() );
        smsc_log_debug(log_,"socket received: %p",&socket);
        socket.send( context, true );
    } catch ( Exception& e ) {
        smsc_log_warn(log_,"exc in sendRequest(req=%p): %s", context->getRequest().get(), e.what());
        throw;
    }
}


void ClientCore::createSocket( util::msectime_type oldac ) // throw
{
    if ( !started_ ) return;
    proto::Socket* socket(new proto::Socket(*this,oldac));
    smsc_log_info(log_,"creating a socket %p on %s:%d tmo=%d", socket,
                  getConfig().getHost().c_str(), getConfig().getPort(),
                  getConfig().getConnectTimeout()/1000 );
    regSet_.create(socket);
    {
        MutexGuard mg(socketMon_);
        sockets_.push_back(socket);
    }
    socket->attach(taskName());
    connector_->connectSocket(*socket);
}


uint32_t ClientCore::getNextSeqNum()
{
    MutexGuard mg(seqMutex_);
    uint32_t result = ++seqNum_;
    if ( !result ) result = ++seqNum_;
    return result;
}


proto::SocketBase& ClientCore::getNextSocket( const Request* request )
{
    if ( loopback_.get() ) return *loopback_.get();

    util::RelockMutexGuard mg(socketMon_);
    if ( sockets_.empty() ) {
        mg.Unlock();
        throw Exception("no socket available", Status::NOT_CONNECTED );
    } else {
        for ( SocketList::iterator i = sockets_.begin(); i != sockets_.end();
              ++i ) {
            if ( (*i)->isConnected() ) {
                proto::Socket* socket = *i;
                sockets_.erase(i);
                sockets_.push_back(socket);
                return *socket;
            }
        }
        mg.Unlock();
        throw Exception("sockets are not connected", Status::NOT_CONNECTED );
    }
}


void ClientCore::closeAllSockets()
{
    assert(connector_->released());
    MutexGuard mg(socketMon_);
    for ( SocketList::iterator i = sockets_.begin(); i != sockets_.end(); ++i ) {
        proto::Socket* socket = *i;
        Core::closeSocket(*socket);
        smsc_log_debug(log_,"pushing socket %p to dead", socket);
        deadSockets_.push_back(socket);
        regSet_.destroy(socket);
    }
    sockets_.clear();
}


void ClientCore::closeSocket( proto::Socket& socket )
{
    Core::closeSocket(socket);
    connector_->unregisterSocket(socket);
    bool found = false;
    util::msectime_type oldac;
    {
        MutexGuard mgc(socketMon_);
        SocketList::iterator i = std::find( sockets_.begin(),
                                            sockets_.end(),
                                            &socket );
        if ( i != sockets_.end() ) {
            found = true;
            oldac = socket.getLastActivity();
            sockets_.erase(i);
            deadSockets_.push_back(&socket);
        }
    }
    if ( found ) {
        smsc_log_debug(log_,"pushing socket %p to dead", &socket );
        regSet_.destroy( &socket );
        destroyDeadSockets();
        if ( started_ ) {
            smsc_log_debug(log_,"recreating socket... with time=%llu",
                           static_cast<unsigned long long>(oldac));
            try {
                createSocket(oldac);
            } catch ( Exception& exc ) {
                smsc_log_warn(log_,"socket recreation %p, details: %s", exc.what() );
            }
        }
    }
}


void ClientCore::destroyDeadSockets()
{
    SocketList trulyDead;
    {
        MutexGuard mg(socketMon_);
        for ( SocketList::iterator i = deadSockets_.begin();
              i != deadSockets_.end();
              ) {
            if ( (*i)->attachCount() <= 1 ) {
                trulyDead.push_back(*i);
                i = deadSockets_.erase(i);
            } else {
                ++i;
            }
        }
    }
    for ( SocketList::iterator i = trulyDead.begin();
          i != trulyDead.end();
          ++i ) {
        proto::Socket* socket = *i;

        /*
         * moved to ContextRegistry
        // destroying all contexts on closed channel
        Exception exc = Exception("channel is closed", Status::IO_ERROR);
        RegistrySet::ContextList pl;
        do {
            RegistrySet::Ptr ptr = regSet_.get(socket);
            if (!ptr) break;
            ptr->popAll(pl);
        } while (false);

        while ( !pl.empty() ) {
            ClientContext* ctx = static_cast<ClientContext*>(pl.front());
            pl.pop_front();
            ctx->setError(exc);
            delete ctx;
        }

        regSet_.destroy(socket);
         */
        socket->detach(taskName()); // it will destroy the socket
    }
}


} // namespace client
} // namespace ewallet
} // namespace bill
} // namespace scag2
