#include "Socket.h"

#include <list>
#include "SocketWriter.h"
#include "Core.h"
#include "Config.h"
#include "scag/bill/ewallet/Exception.h"

using namespace smsc::core::synchronization;

namespace scag2 {
namespace bill {
namespace ewallet {
namespace proto {

Socket* Socket::fromSocket( smsc::core::network::Socket& sock )
{
    return reinterpret_cast< Socket* >( sock.getData(0) );
}


Socket::Socket( Core& core, util::msectime_type ac ) :
SocketBase(core),
sock_( new smsc::core::network::Socket ),
wrBufferSize_(0),
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
        MutexGuard mgq(queueMon_);
        if ( sendRequest && size_t(queue_->Count()) > core_.getConfig().getSocketQueueSizeLimit() ) {
            throw Exception("queue size limit exceeded", Status::CLIENT_BUSY);
        }
        writeContext->setContext( context.release() );
        queue_->Push( writeContext.release() );
        // actually, this is not necessary, as writer has its own monitor
        queueMon_.notify();
    }
    writer_->wakeup();
}


/// check that socket wants to send some data.
/// invoked from writer.
/// NOTE: not thread-safe
bool Socket::wantToSend( util::msectime_type currentTime )
{
    if ( !isConnected() ) return false;
    if ( wrBuffer_.GetPos() < wrBufferSize_ ) { return true; }
    bool result = false;
    WriteContext* ctx;
    std::list< WriteContext* > expired;
    {
        MutexGuard mg(queueMon_);
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
        Streamer::Buffer& buffer = ctx->buffer;
        const size_t buflen = buffer.GetPos();
        wrBuffer_.setExtBuf(const_cast<char*>(buffer.get()), buflen);
        wrBufferSize_ = buflen;
        if (log_->isDebugEnabled()) {
            util::HexDump hd;
            util::HexDump::string_type dump;
            dump.reserve(hd.hexdumpsize(buflen)+hd.strdumpsize(buflen)+10);
            hd.hexdump(dump,buffer.get(),buflen);
            hd.strdump(dump,buffer.get(),buflen);
            smsc_log_debug(log_,"packet to send: %s",hd.c_str(dump));
        }
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
    // assert( wrContext_.get() );
    if ( wrBuffer_.GetPos() < wrBufferSize_ ) {
        int res = wrBufferSize_ - wrBuffer_.GetPos();
        smsc_log_debug(log_,"writing %d/%d bytes",res,wrBufferSize_);
        res = sock_->Write( wrBuffer_.get()+wrBuffer_.GetPos(), res );
        if (res<=0) {
            smsc_log_warn(log_,"sendData: write failed: %d", res);
            reportPacket(Context::FAILED);
            return;
        }

        // lastActivity_ = util::currentTimeMillis();
        wrBuffer_.SetPos( wrBuffer_.GetPos() + res );

        if ( wrBuffer_.GetPos() < wrBufferSize_ ) return;
    }
    reportPacket(Context::SENT);
}


/// invoked from reader when there is some data on input.
/// NOTE: not thread-safe.
void Socket::processInput()
{
    if ( rdBuffer_.GetPos() < 4 ) {
        // reading length
        int res;
        res = sock_->Read(rdBuffer_.get()+rdBuffer_.GetPos(),4-rdBuffer_.GetPos());
        if (res < 0) {
            core_.handleError(*this,Exception("error reading packet length", Status::IO_ERROR));
            return;
        } else if (res == 0) {
            smsc_log_debug(log_,"peer has closed connection");
            core_.closeSocket(*this);
            return;
        }
        // lastActivity_ = util::currentTimeMillis();
        rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
        if ( rdBuffer_.GetPos() >= 4 ) {
            rdBuflen_ = ntohl(*reinterpret_cast<const uint32_t*>(rdBuffer_.get()));
            if ( rdBuflen_ > 70000 ) {
                core_.handleError(*this,Exception(Status::IO_ERROR, "too large packet: %d",rdBuflen_));
                return;
            }
            smsc_log_debug(log_,"read packet size:%d",rdBuflen_);
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
    rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
    smsc_log_debug(log_,"reading %u data bytes, buflen=%u",res,unsigned(rdBuffer_.GetPos()));
    // lastActivity_ = util::currentTimeMillis();
    if ( rdBuffer_.GetPos() < rdBuflen_ ) return;
    // Streamer::Buffer readbuf(rdBuffer_.get()+4,rdBuflen_-4);
    // readbuf.SetPos(rdBuflen_-4);
    if ( log_->isDebugEnabled() ) {
        util::HexDump hd;
        util::HexDump::string_type dump;
        dump.reserve(hd.hexdumpsize(rdBuflen_)+hd.strdumpsize(rdBuflen_)+10);
        hd.hexdump(dump,rdBuffer_.get(),rdBuflen_);
        hd.strdump(dump,rdBuffer_.get(),rdBuflen_);
        smsc_log_debug(log_,"read len=%u data=%s",unsigned(rdBuflen_),hd.c_str(dump));
    }

    std::auto_ptr<Packet> packet;
    try {
        packet.reset( core_.getStreamer().deserialize(rdBuffer_) );
        smsc_log_debug(log_,"packet received: %s", packet->toString().c_str());
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
        Context* ctx;
        {
            MutexGuard mg(queueMon_);
            if ( ! wrContext_.get() ) break;
            ctx = wrContext_->getContext();
        }
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
    smsc_log_info(log_,"socket %p dtor",this);
    MutexGuard mg(queueMon_);
    assert( refCount_ == 0 );
    assert( wrContext_.get() == 0 );
    assert( writer_ == 0 );
    assert( ! isConnected() );
}


void Socket::init()
{
    sock_->setData(0,this);
}


void Socket::reportPacket( int state )
{
    MutexGuard mg(queueMon_);
    core_.reportPacket(*this,wrContext_->getSeqNum(),wrContext_->popContext(),Context::ContextState(state));
    wrContext_.reset(0);
}

} // namespace proto
} // namespace ewallet
} // namespace bill
} // namespace scag2
