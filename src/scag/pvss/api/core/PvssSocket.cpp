#include "PvssSocket.h"
#include "scag/util/HexDump.h"
#include "scag/pvss/api/core/Core.h"

namespace {
smsc::core::synchronization::Mutex logMtx;
}

namespace scag2 {
namespace pvss {
namespace core {

using namespace smsc::core::synchronization;

smsc::logger::Logger* PvssSocket::log_ = 0;

PvssSocket::PvssSocket( const std::string& host,
                        short port,
                        int connectionTmo ) :
sock_( new smsc::core::network::Socket ),
host_(host), port_(port), connectionTmo_(connectionTmo),
rdBuflen_(0),
writer_(0),
reader_(0),
connectTime_(0),
lastActivity_(0)
{
    init();
}


PvssSocket::PvssSocket(smsc::core::network::Socket* socket) :
sock_(socket),
host_(), port_(0), connectionTmo_(0),
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


void PvssSocket::send( const Packet* packet, bool isRequest, bool force ) throw (PvssException)
{
    if ( ! writer_ )
        throw PvssException(PvssException::NOT_CONNECTED, "writer is not connected");
    if ( writer_->stopping() )
        throw PvssException(PvssException::NOT_CONNECTED, "writer is not started");
    if ( packet == 0 )
        throw PvssException(PvssException::IO_ERROR, "packet to send is null");
    if ( ! packet->isValid() )
        throw PvssException(PvssException::IO_ERROR, "packet %s is bad formed", packet->toString().c_str() );

    // fast check for queue limit (w/o locking)
    if ( !force && pendingContexts_.Count() > writer_->getConfig().getChannelQueueSizeLimit() )
        throw PvssException(PvssException::CLIENT_BUSY, "Queue size limit exceeded for channel: %p",this);

    std::auto_ptr<WriteContext> writeContext(new WriteContext(packet->getSeqNum(),util::currentTimeMillis()));

    // packet serialization
    writer_->serialize(*packet,writeContext->buffer);
    int packetSize = writeContext->buffer.GetPos();
    if ( log_->isDebugEnabled() ) {
        smsc_log_debug(log_,"writing %s: %s", isRequest ? "request" : "response",
                       packet->toString().c_str());
        std::string dump;
        util::HexDump hd;
        hd.hexdump( dump, writeContext->buffer.get(), packetSize );
        smsc_log_debug( log_, "write length=%d data=%s", packetSize-4, dump.c_str() );
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
    writer_->notify();
}


bool PvssSocket::wantToSend()
{
    if ( !isConnected() ) return false;
    if ( wrContext_.get() && wrBuffer_.GetPos() < wrBuffer_.getSize() ) { return true; }
    WriteContext* ctx;
    {
        MutexGuard mg(pendingContextMon_);
        // extract the first item from the queue (checking for expiration?)
        if ( ! pendingContexts_.Pop(ctx) ) return false;
        wrContext_.reset(ctx);
        // FIXME: we have to check expiration time here
    }
    wrBuffer_.setExtBuf( const_cast<char*>(ctx->buffer.get()), ctx->buffer.GetPos() );
    return true;
}


void PvssSocket::sendData( Core& core )
{
    assert(wrContext_.get());
    if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) {
        int res = wrBuffer_.getSize() - wrBuffer_.GetPos();
        smsc_log_debug(log_,"writing %d/%d bytes",res,wrBuffer_.getSize());
        res = sock_->Write( wrBuffer_.get()+wrBuffer_.GetPos(), res );
        if (res<=0) {
            smsc_log_warn(log_,"sendData: write failed: %d", res);
            core.reportPacket(wrContext_->seqNum, *socket(), Core::FAILED);
            wrContext_.reset(0);
            core.handleError(PvssException(PvssException::IO_ERROR,"error writing context"),*this);
            return;
        }

        lastActivity_ = util::currentTimeMillis();
        wrBuffer_.SetPos( wrBuffer_.GetPos() + res );

        if ( wrBuffer_.GetPos() < wrBuffer_.getSize() ) return;
    }
    core.reportPacket(wrContext_->seqNum, *socket(), Core::SENT);
    wrContext_.reset(0);
}


/// ???
void PvssSocket::processInput( Core& core )
{
    if ( rdBuffer_.GetPos() < 4 ) {
        // reading length
        int res;
        res = sock_->Read(rdBuffer_.get()+rdBuffer_.GetPos(),4-rdBuffer_.GetPos());
        if (res <= 0) {
            core.handleError(PvssException(PvssException::IO_ERROR,"error reading packet length"), *this);
            return;
        }
        lastActivity_ = util::currentTimeMillis();
        rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
        if ( rdBuffer_.GetPos() >= 4 ) {
            rdBuflen_ = ntohl(*reinterpret_cast<const uint32_t*>(rdBuffer_.get()))+4;
            if ( rdBuflen_ > 70000 ) {
                core.handleError(PvssException(PvssException::IO_ERROR, "too large packet: %d",rdBuflen_), *this);
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
        core.handleError(PvssException(PvssException::IO_ERROR,"error reading packet data"), *this);
        return;
    }
    lastActivity_ = util::currentTimeMillis();
    rdBuffer_.SetPos(rdBuffer_.GetPos()+res);
    if ( rdBuffer_.GetPos() < rdBuflen_ ) return;
    Protocol::Buffer readbuf(rdBuffer_.get()+4,rdBuflen_-4);
    readbuf.SetPos(rdBuflen_-4);
    if ( log_->isDebugEnabled() ) {
        std::string dump;
        util::HexDump hd;
        hd.hexdump(dump,readbuf.get(),readbuf.GetPos());
        smsc_log_debug(log_,"read length=%d data=%s",readbuf.GetPos(),dump.c_str());
    }
    std::auto_ptr<Packet> packet;
    try {
        packet.reset(core.getProtocol()->deserialize(readbuf));
    } catch (PvssException& e) {
        core.handleError(e,*this);
        return;
    }
    rdBuffer_.SetPos(0);
    rdBuflen_ = 0;
    core.receivePacket(packet,*this);
}


void PvssSocket::connect() throw (exceptions::IOException)
{
    if ( isConnected() ) return;
    {
        MutexGuard mg(pendingContextMon_);
        if ( isConnected() ) return;
    }
    smsc_log_info(log_,"Connecting channel %p to host=%s:%d tmo=%d", this,
                  host_.c_str(), port_, connectionTmo_ );
    if ( sock_->Init(host_.c_str(),port_,connectionTmo_) == -1 ) {
        throw exceptions::IOException("cannot init socket on channel %p", this);
    }
    if ( sock_->Connect() == -1 ) {
        throw exceptions::IOException("cannot connect socket on channel %p", this);
    }
}


void PvssSocket::disconnect()
{
    // FIXME: what to do to pendingContexts queue ?
    sock_->Close();
    rdBuflen_ = 0;
    rdBuffer_.SetPos(0);
    wrBuffer_.SetPos(wrBuffer_.getSize());
}

void PvssSocket::registerWriter( PacketWriter* writer )
{
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
        char buf[50];
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
