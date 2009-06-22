#include "Socket.h"

#include <list>
#include "SocketWriter.h"
#include "Core.h"
#include "Config.h"
#include "scag/bill/ewallet/Exception.h"

using namespace smsc::core::synchronization;

namespace {
smsc::core::synchronization::Mutex logMtx;
}

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

smsc::logger::Logger* Socket::log_ = 0;

Socket* Socket::fromSocket( smsc::core::network::Socket& sock )
{
    return reinterpret_cast< Socket* >( sock.getData(0) );
}


Socket::Socket( Core& core, util::msectime_type ac ) :
core_(core),
sock_( new smsc::core::network::Socket ),
refCount_(0),
queue_(new smsc::core::buffers::CyclicQueue< WriteContext* >),
rdBuflen_(0),
writer_(0),
lastActivity_(ac)
{
    init();
    // sock_->setData(0,this);
}


/// send packet to a sending queue and notify writer.
/// NOTE: invoked from core.
/// NOTE: if exception is thrown, context fields are not touched.
void Socket::send( std::auto_ptr< Context >& context, bool sendRequest )
{
    const Packet* packet;
    const char* what;
    if ( sendRequest ) {
        packet = context->getRequest().get();
        what = "request";
    } else {
        packet = context->getResponse().get();
        what = "response";
    }
    if ( !packet ) {
        throw Exception( sendRequest ? Status::BAD_REQUEST : Status::BAD_RESPONSE,
                         "%s is null", what );
    } else if ( !packet->isValid() ) {
        throw Exception( sendRequest ? Status::BAD_REQUEST : Status::BAD_RESPONSE,
                         "%s is bad formed", what );
    }

    if ( ! isConnected() ) {
        throw Exception( "socket is not connected", Status::NOT_CONNECTED );
    }
    MutexGuard mg(writerMutex_);
    if ( ! writer_ || !queue_ ) {
        throw Exception( "writer is not attached", Status::NOT_CONNECTED );
    }
    if ( writer_->stopping() ) {
        throw Exception( "writer is not started", Status::NOT_CONNECTED );
    }
    // fast check for queue limit (w/o locking)
    if ( sendRequest && size_t(queue_->Count()) > core_.getConfig().getSocketQueueSizeLimit() ) {
        throw Exception( "queue size limit exceeded", Status::CLIENT_BUSY );
    }

    std::auto_ptr< WriteContext > writeContext
        ( new WriteContext( sendRequest ? util::currentTimeMillis() : 0 ) );

    // serialization
    core_.getStreamer().serialize( *packet, writeContext->buffer );

    { // pushing to the queue, checking its size again
        MutexGuard mgq(queueMutex_);
        if ( sendRequest && size_t(queue_->Count()) > core_.getConfig().getSocketQueueSizeLimit() ) {
            throw Exception("queue size limit exceeded", Status::CLIENT_BUSY);
        }
        writeContext->setContext( context.release() );
        queue_->Push( writeContext.release() );
        // queueMon_.notify();
    }
    writer_->wakeup();
}


/// check that socket wants to send some data.
/// invoked from writer.
/// NOTE: not thread-safe
bool Socket::wantToSend( util::msectime_type currentTime )
{
    if ( !isConnected() ) return false;
    if ( wrContext_.get() && wrBuffer_.GetPos() < wrBuffer_.getSize() ) { return true; }
    bool result = false;
    WriteContext* ctx;
    std::list< WriteContext* > expired;
    {
        MutexGuard mg(queueMutex_);
        while( true ) {
            if ( !queue_ ) break;
            if ( !queue_->Pop(ctx) ) break;
            if ( !ctx ) continue;
            if ( ctx->creationTime == 0 ||
                 ctx->creationTime + core_.getConfig().getIOTimeout() >= currentTime ) {
                result = true;
                wrContext_.reset(ctx);
                break;
            }
            expired.push_back(ctx);
        }
    }

    if ( result ) {
        // report sending
        core_.reportPacket(*this,ctx->getSeqNum(),ctx->popContext(),Context::SENDING);
        wrBuffer_.setExtBuf( const_cast<char*>(ctx->buffer.get()), ctx->buffer.GetPos() );
    }

    // report expired
    while ( !expired.empty() ) {
        ctx = expired.front();
        expired.pop_front();
        core_.reportPacket(*this,ctx->getSeqNum(),ctx->popContext(),Context::EXPIRED);
        delete ctx;
    }
    return result;
}


/// send data and notify core when a packet is sent.
void Socket::sendData()
{
    assert( wrContext_.get() );
    if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) {
        int res = wrBuffer_.getSize() - wrBuffer_.GetPos();
        smsc_log_debug(log_,"writing %d/%d bytes",res,wrBuffer_.getSize());
        res = sock_->Write( wrBuffer_.get()+wrBuffer_.GetPos(), res );
        if (res<=0) {
            smsc_log_warn(log_,"sendData: write failed: %d", res);
            reportPacket(Context::FAILED);
            return;
        }

        // lastActivity_ = util::currentTimeMillis();
        wrBuffer_.SetPos( wrBuffer_.GetPos() + res );

        if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) return;
    }
    reportPacket(Context::SENT);
}


/// invoked from reader when there in some data on input.
/// NOTE: not thread-safe.
void Socket::processInput()
{
    if ( rdBuffer_.GetPos() < 4 ) {
        // reading length
        int res;
        res = sock_->Read(rdBuffer_.get()+rdBuffer_.GetPos(),4-rdBuffer_.GetPos());
        if (res <= 0) {
            core_.handleError(*this,Exception("error reading packet length", Status::IO_ERROR));
            return;
        }
        // lastActivity_ = util::currentTimeMillis();
        rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
        if ( rdBuffer_.GetPos() >= 4 ) {
            rdBuflen_ = ntohl(*reinterpret_cast<const uint32_t*>(rdBuffer_.get()))+4;
            if ( rdBuflen_ > 70000 ) {
                core_.handleError(*this,Exception(Status::IO_ERROR, "too large packet: %d",rdBuflen_));
                return;
            }
            smsc_log_debug(log_,"read packet size:%d",rdBuflen_-4);
            rdBuffer_.reserve(rdBuflen_);
        }
        return;
    }
    // reading data
    int res;
    res = sock_->Read(rdBuffer_.get()+rdBuffer_.GetPos(),rdBuflen_-rdBuffer_.GetPos());
    if ( res <= 0 ) {
        core_.handleError(*this,Exception("error reading packet data", Status::IO_ERROR));
        return;
    }
    // lastActivity_ = util::currentTimeMillis();
    rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
    if ( rdBuffer_.GetPos() < rdBuflen_ ) return;
    Streamer::Buffer readbuf(rdBuffer_.get()+4,rdBuflen_-4);
    readbuf.SetPos(rdBuflen_-4);
    if ( log_->isDebugEnabled() ) {
        util::HexDump hd;
        util::HexDump::string_type dump;
        hd.hexdump(dump,readbuf.get(),readbuf.GetPos());
        smsc_log_debug(log_,"read length=%d data=%s",readbuf.GetPos(),hd.c_str(dump));
    }
    std::auto_ptr<Packet> packet;
    try {
        packet.reset( core_.getStreamer().deserialize(readbuf) );
    } catch (Exception& e) {
        core_.handleError(*this,e);
        return;
    }
    rdBuffer_.SetPos(0);
    rdBuflen_ = 0;
    // first of all, it may be a race condition if server reply too fast,
    // i.e. the context containing a written request is not passed to core yet.
    // So we have to check it here.
    do {
        if ( packet->isRequest() ) break;
        MutexGuard mg(queueMutex_);
        if ( ! wrContext_.get() ) break;
        Context* ctx = wrContext_->getContext();
        if ( ! ctx ) break;
        uint32_t seqNum = ctx->getSeqNum();
        if ( seqNum != packet->getSeqNum() || !ctx->getRequest().get() ) break;
        // response match that of request
        if ( ctx->getResponse().get() ) {
            // response is already set
            smsc_log_warn(log_,"context seqNum=%u already has a response set, new one is skipped",unsigned(seqNum));
        } else {
            ctx->getResponse().reset( static_cast<Response*>(packet.release()));
        }
        // do not pass packet to core, it will be passed in writer
        return;

    } while ( false );
    core_.receivePacket(*this,packet);
}


/// attach/detach a socket: refcounting
void Socket::attach( const char* who )
{
    smsc_log_debug(log_,"attaching socket %p to %s", this, who);
    MutexGuard mg(refMutex_);
    ++refCount_;
}


void Socket::detach( const char* who )
{
    smsc_log_debug(log_,"detaching socket %p from %s", this, who);
    bool destroy;
    {
        MutexGuard mg(refMutex_);
        if (refCount_ > 0) {
            --refCount_;
        } else {
            smsc_log_warn(log_,"logics failure: refcount=0 before detach of %p", this);
        }
        destroy = ( refCount_ == 0 );
    }
    if ( destroy ) {
        delete this;
    }
}


unsigned Socket::attachCount() {
    MutexGuard mg(refMutex_);
    return refCount_;
}


void Socket::setWriter( SocketWriter* writer )
{
    SocketWriter* prevWriter;
    {
        MutexGuard mg(writerMutex_);
        if ( writer == writer_ ) {
            return;
        }
        if ( writer == 0 || writer_ == 0 ) {
            writer_ = writer;
            return;
        }
        prevWriter = writer_;
    }
    smsc_log_warn(log_,"valid writer cannot be changed: %p -> %p", prevWriter, writer );
}


void Socket::connect()
{
    if ( isConnected() ) return;
    const std::string host = core_.getConfig().getHost();
    const short port = core_.getConfig().getPort();
    updateActivity();
    smsc_log_info(log_,"Connecting channel %p to host=%s:%d", this, host.c_str(), port );
    if ( sock_->Init(host.c_str(),port,5) == -1 ) {
        smsc_log_info(log_,"cannot init socket");
        throw Exception(Status::NOT_CONNECTED,"cannot init socket on channel %p",this);
    }
    if ( sock_->Connect() == -1 ) {
        smsc_log_info(log_,"connect failed");
        throw Exception(Status::NOT_CONNECTED,"cannot connect socket on channel %p", this);
    }
}


Socket::~Socket()
{
    // checking precondition!
    assert( refCount_ == 0 );
    assert( wrContext_.get() == 0 );
    assert( writer_ == 0 );
    assert( ! isConnected() );
    smsc::core::buffers::CyclicQueue< WriteContext* >* prevQueue = 0;
    {
        MutexGuard mg(queueMutex_);
        std::swap(queue_,prevQueue);
    }
    // cleanup
    WriteContext* ctx;
    while ( prevQueue->Pop(ctx) ) {
        core_.reportPacket(*this, ctx->getSeqNum(), ctx->popContext(), Context::FAILED);
        delete ctx;
    }
    delete prevQueue;
}


void Socket::init()
{
    if ( !log_ ) {
        MutexGuard mg(::logMtx);
        if (!log_) log_ = smsc::logger::Logger::getInstance("ewall.sock");
    }
    sock_->setData(0,this);
}


void Socket::reportPacket( int state )
{
    core_.reportPacket(*this,wrContext_->getSeqNum(),wrContext_->popContext(),Context::ContextState(state));
    wrContext_.reset(0);
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
