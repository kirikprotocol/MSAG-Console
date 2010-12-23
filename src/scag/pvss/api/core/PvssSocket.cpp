#include "PvssSocket.h"
#include "scag/util/io/HexDump.h"
#include "scag/pvss/api/core/Core.h"

namespace {
smsc::core::synchronization::Mutex logMtx;
}

namespace scag2 {
namespace pvss {
namespace core {

using namespace smsc::core::synchronization;

smsc::logger::Logger* PvssSocket::log_ = 0;

PvssSocket::PvssSocket( Core& core ) :
core_(core),
sock_( new smsc::core::network::Socket ),
host_( core.getConfig().getHost() ),
port_( core.getConfig().getPort() ),
connectionTmo_(5),
rdBuflen_(0),
writer_(0),
reader_(0),
connectTime_(0),
lastActivity_(0)
{
    init();
}


PvssSocket::PvssSocket( Core& core, smsc::core::network::Socket* socket ) :
core_(core),
sock_(socket),
rdBuflen_(0),
writer_(0),
reader_(0),
connectTime_(0),
lastActivity_(util::currentTimeMillis())
{    
    init();
}


PvssSocket::~PvssSocket()
{
    smsc_log_debug(log_,"dtor: socket %p",this);
}


void PvssSocket::send( const Packet* packet, bool isRequest, bool force ) /* throw (PvssException) */ 
{
    MutexGuard mg(writerMutex_);
    if ( ! writer_ )
        throw PvssException(PvssException::NOT_CONNECTED, "writer is not connected");
    if ( writer_->stopping() )
        throw PvssException(PvssException::NOT_CONNECTED, "writer is not started");
    if ( packet == 0 )
        throw PvssException(PvssException::IO_ERROR, "packet to send is null");
    if ( ! packet->isValid() )
        throw PvssException(PvssException::IO_ERROR, "packet %s is bad formed", packet->toString().c_str() );

    // fast check for queue limit (w/o locking)
    const unsigned pendingCount = pendingContexts_.Count();
    if ( !force && pendingCount > unsigned(writer_->getConfig().getChannelQueueSizeLimit()) )
        throw PvssException(PvssException::CLIENT_BUSY, "Queue size limit exceeded for channel: %p",this);

    std::auto_ptr<WriteContext> writeContext( new WriteContext(packet->getSeqNum(),
                                                               force ? 0 : util::currentTimeMillis()) );

    // packet serialization
    writer_->serialize(*packet,writeContext->buffer);
    if ( packet->hasTiming() ) {
        packet->timingMark("afterSerlz");
        char buf[40];
        sprintf(buf," (pendingQsz=%u)", pendingCount );
        packet->timingComment(buf);
    }
    int packetSize = writeContext->buffer.GetPos();
    if ( log_->isDebugEnabled() ) {
        smsc_log_debug(log_,"writing %s: %s", isRequest ? "request" : "response",
                       packet->toString().c_str());
        util::HexDump hd;
        util::HexDump::string_type dump;
        hd.hexdump( dump, writeContext->buffer.get(), packetSize );
        smsc_log_debug( log_, "write length=%d data=%s", packetSize-4, hd.c_str(dump) );
    }

    // should we check the queue limit again?
    {
        MutexGuard mg(pendingContextMon_);
        if ( !force && pendingContexts_.Count() > writer_->getConfig().getChannelQueueSizeLimit() )
            throw PvssException(PvssException::CLIENT_BUSY, "Queue size limit exceeded for channel: %p",this);
        pendingContexts_.Push(writeContext.release());
        pendingContextMon_.notify();
    }

    // notify the writer
    writer_->wakeup();
}


bool PvssSocket::wantToSend( util::msectime_type currentTime )
{
    if ( !isConnected() ) return false;
    if ( wrContext_.get() && wrBuffer_.GetPos() < wrBuffer_.getSize() ) { return true; }
    bool result = false;
    WriteContext* ctx;
    std::list< WriteContext* > expired;
    {
        MutexGuard mg(pendingContextMon_);
        // extract the first item from the queue
        while ( true ) {
            if ( ! pendingContexts_.Pop(ctx) ) break;
            if ( ! ctx ) continue;
            if ( ctx->creationTime == 0 || ctx->creationTime + core_.getConfig().getIOTimeout() >= currentTime ) {
                result = true;
                wrContext_.reset(ctx);
                break;
            }
            expired.push_back(ctx);
        }
    }
    if ( result ) {
        wrBuffer_.setExtBuf( const_cast<char*>(ctx->buffer.get()), ctx->buffer.GetPos() );
    }

    // report expired requests
    while ( ! expired.empty() ) {
        ctx = expired.front();
        expired.pop_front();
        core_.reportPacket(ctx->seqNum,*socket(),Core::EXPIRED);
        delete ctx;
    }
    return result;
}


void PvssSocket::sendData()
{
    assert(wrContext_.get());
    if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) {
        int res = wrBuffer_.getSize() - wrBuffer_.GetPos();
        smsc_log_debug(log_,"writing %d/%d bytes",res,wrBuffer_.getSize());
        res = sock_->Write( wrBuffer_.get()+wrBuffer_.GetPos(), res );
        if (res<=0) {
            smsc_log_warn(log_,"sendData: write failed: %d", res);
            core_.reportPacket(wrContext_->seqNum, *socket(), Core::FAILED);
            wrContext_.reset(0);
            core_.handleError(PvssException(PvssException::IO_ERROR,"io error (write failed:%d)",res),*this);
            return;
        }

        lastActivity_ = util::currentTimeMillis();
        wrBuffer_.SetPos( wrBuffer_.GetPos() + res );

        if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) return;
    }
    core_.reportPacket(wrContext_->seqNum, *socket(), Core::SENT);
    wrContext_.reset(0);
}


/// ???
void PvssSocket::processInput()
{
    if ( rdBuffer_.GetPos() < 4 ) {
        // reading length
        int res;
        res = sock_->Read(rdBuffer_.get()+rdBuffer_.GetPos(),4-rdBuffer_.GetPos());
        if (res <= 0) {
            core_.handleError(PvssException(PvssException::IO_ERROR,"error reading packet length res=%d, conn=%d",
                                            res,sock_->isConnected() ? 1 : 0), *this);
            return;
        }
        lastActivity_ = util::currentTimeMillis();
        rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
        if ( rdBuffer_.GetPos() >= 4 ) {
            rdBuflen_ = ntohl(*reinterpret_cast<const uint32_t*>(rdBuffer_.get()))+4;
            if ( rdBuflen_ > 70000 ) {
                core_.handleError(PvssException(PvssException::IO_ERROR, "too large packet: %d",rdBuflen_), *this);
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
        core_.handleError(PvssException(PvssException::IO_ERROR,"error reading packet data"), *this);
        return;
    }
    lastActivity_ = util::currentTimeMillis();
    rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
    if ( rdBuffer_.GetPos() < rdBuflen_ ) return;
    Protocol::Buffer readbuf(rdBuffer_.get()+4,rdBuflen_-4);
    readbuf.SetPos(rdBuflen_-4);
    if ( log_->isDebugEnabled() ) {
        util::HexDump hd;
        util::HexDump::string_type dump;
        hd.hexdump(dump,readbuf.get(),readbuf.GetPos());
        smsc_log_debug(log_,"read length=%d data=%s",readbuf.GetPos(),hd.c_str(dump));
    }
    std::auto_ptr<Packet> packet;
    try {
        packet.reset( core_.getProtocol()->deserialize(readbuf) );
    } catch (PvssException& e) {
        core_.handleError(e,*this);
        return;
    }
    rdBuffer_.SetPos(0);
    rdBuflen_ = 0;
    core_.receivePacket(packet,*this);
}


void PvssSocket::connect() /* throw (exceptions::IOException) */ 
{
    if ( isConnected() ) return;
    {
        MutexGuard mg(pendingContextMon_);
        if ( isConnected() ) return;
    }
    smsc_log_info(log_,"Connecting channel %p to host=%s:%d tmo=%d", this,
                  host_.c_str(), int(port_), connectionTmo_ );
    if ( sock_->Init(host_.c_str(),port_,connectionTmo_) == -1 ) {
        smsc_log_warn(log_,"cannot init socket %p on channel %p", socket(), this );
        throw exceptions::IOException("cannot init socket %p on channel %p", socket(), this);
    }
    if ( sock_->Connect() == -1 ) {
        smsc_log_info(log_,"connect failed");
        throw exceptions::IOException("Connect() failed for socket %p on channel %p", socket(), this);
    }
}


void PvssSocket::disconnect()
{
    sock_->Close();
}

void PvssSocket::registerWriter( PacketWriter* writer )
{
    MutexGuard mg(writerMutex_);
    smsc_log_debug(log_,"attaching writer %p to channel %p",writer,this);
    assert( bool(writer) != bool(writer_) );
    writer_ = writer;
}
void PvssSocket::registerReader( PacketReader* reader )
{
    smsc_log_debug(log_,"attaching reader %p to channel %p",reader,this);
    assert( bool(reader) != bool(reader_) );
    reader_ = reader;
}

bool PvssSocket::isInUse() const
{
    return writer_ || reader_;
}

void PvssSocket::init()
{
    if ( ! log_ ) {
        MutexGuard mg(logMtx);
        if ( ! log_ ) log_ = smsc::logger::Logger::getInstance("pvss.sock");
    }
    sock_->setData(0,this);
    if ( host_.empty() && sock_->isConnected() ) {
        char buf[60];
        sock_->GetPeer(buf);
        char* colon = strchr(buf,':');
        if ( colon ) {
            host_ = std::string(buf,colon-buf);
            port_ = atoi(colon+1);
        }
    }
    smsc_log_debug(log_,"ctor: socket %p (%s:%d)",this,host_.c_str(),int(port_)&0xffff);
}

} // namespace core
} // namespace pvss
} // namespace scag2
