#include <cassert>
#include "scag/util/io/HexDump.h"
#include "PvssConnection.h"
#include "scag/pvss/base/PersClientException.h"
#include "PvssStreamClient.h"

using namespace smsc::core::synchronization;

namespace {
using namespace scag2::pvss::client;
using namespace scag2::util::storage;

std::auto_ptr<char> bufdump( SerialBuffer& buffer, unsigned buflen )
{
    scag2::util::HexDump hd;
    std::auto_ptr<char> buf(new char[hd.hexdumpsize(buflen)+hd.strdumpsize(buflen)+10]);
    char* outbuf = buf.get();
    outbuf = hd.hexdump(outbuf,buffer.ptr(),buflen);
    outbuf = hd.addstr(outbuf," (");
    outbuf = hd.strdump(outbuf,buffer.ptr(),buflen);
    outbuf = hd.addstr(outbuf,")");
    *outbuf = '\0';
    return buf;
}

}


namespace scag2 {
namespace pvss {
namespace client {

PvssConnection::PvssConnection( PvssStreamClient& pers ) :
log_(smsc::logger::Logger::getInstance("pvss.conn")),
logd_(smsc::logger::Logger::getInstance("pvss.dump")),
connected_(false),
lastActivity_(0),
lastConnect_(0),
pers_(&pers),
seqnum_(0),
rdToRead(0),
wrBufSent(0),
isReading_(false)
{
    sock_.setData(0,this);
}


void PvssConnection::processInput( bool hasSeqnum )
{
    if (rdBuffer.getPos()<4) {
        int res;
        {
            // MutexGuard mg(mtx);
            res=sock_.Read(rdBuffer.ptr()+rdBuffer.getPos(),4-rdBuffer.getPos());
        }
        if (res<=0) {
            disconnect();
            return;
        }
        lastActivity_ = time(NULL);
        rdBuffer.setPos( rdBuffer.getPos() + res );
        return;
    }

    if (rdToRead==0) {
        //rdToRead=ntohl(*(uint32_t*)rdBuffer);
        memcpy(&rdToRead,rdBuffer.ptr(),4);
        rdToRead = ntohl(rdToRead);
        if (rdToRead>70000)
        {
            smsc_log_warn(log_,"command is too large:%d", rdToRead);
            // MutexGuard mg(mtx);
            disconnect();
            return;
        }
        rdBuffer.reserve( rdToRead );
    }
    int res;
    {
        // MutexGuard mg(mtx);
        res = sock_.Read(rdBuffer.ptr()+rdBuffer.getPos(),rdToRead-rdBuffer.getPos());
    }
    if (res<=0) {
        smsc_log_warn(log_, "socket error from recv");
        disconnect();
        return;
    }
    lastActivity_ = time(NULL);
    rdBuffer.setPos(rdBuffer.getPos() + res);
    if (rdBuffer.getPos() < rdToRead) return;
    int32_t seqnum;
    if ( hasSeqnum ) {
        if ( rdBuffer.getPos() < 8 ) {
            std::auto_ptr< char > buf = ::bufdump( rdBuffer, rdToRead );
            smsc_log_warn( log_, "failed to parse pvss input from=%p (seqnum): %s",
                           this, buf.get() );
            disconnect();
            return;
        }
        seqnum = ntohl(*(int32_t*)(rdBuffer.ptr()+4));
        rdBuffer.setPos(8);
    } else {
        seqnum = 0;
        rdBuffer.setPos(4);
    }
    if ( logd_->isDebugEnabled())
    {
        std::auto_ptr<char> buf = ::bufdump( rdBuffer, rdToRead );
        smsc_log_debug( logd_, "in from=%p len=%d seq=%d: %s", this, rdToRead, seqnum, buf.get() );
    }
    rdToRead = 0;
    PersCall* call = getCall(seqnum);
    if ( call ) {
        try {
            call->readSB(rdBuffer);
        } catch (...) {
            call->setStatus( BAD_RESPONSE );
        }
        call->initiator()->continuePersCall(call,false);
    }
    rdBuffer.setPos(0);
    rdBuffer.setLength(0);
    if (!pers_->async) setReading( false );
}


void PvssConnection::sendData()
{
    smsc_log_debug(log_, "sendData: %d/%d",wrBufSent,wrBuffer.length());
    if (wrBuffer.length() && wrBufSent<wrBuffer.length()) {
        int res;
        {
            // MutexGuard mg(mtx);
            res = sock_.Write(wrBuffer.ptr()+wrBufSent,wrBuffer.length()-wrBufSent);
        }
        if (res<=0) {
            smsc_log_warn(log_, "sendData: write failed:%d", res);
            disconnect();
            return;
        }
        lastActivity_ = time(NULL);
        wrBufSent += res;
        if (wrBufSent==wrBuffer.length()) {
            wrBufSent=0;
            wrBuffer.setPos(0);
            wrBuffer.setLength(0);
            if (!pers_->async) setReading(true);
        }
        smsc_log_debug(log_, "sendData: sent %d bytes",res);
        return;
    }
    wantToSend();
}


bool PvssConnection::wantToSend()
{
    if ( wrBuffer.length() > 0 ) return true;
    // smsc_log_debug( log_, "checking if we want to send" );
    PersCall* ctx = pers_->getCall();
    if ( ! ctx ) {
        const time_t now = time(0);
        if ( lastActivity_ + pers_->pingTimeout > now ) return false;
        ctx = pers_->createPingCall();
        // if ( ctx ) smsc_log_debug( log_, "ping call" );
    }
    if ( ! ctx ) return false;
    prepareWrBuffer( ctx );
    return true;
}


bool PvssConnection::isReading() const
{
    MutexGuard mg(regmtx_);
    return isReading_;
}


void PvssConnection::connect()
{
    if ( connected_ ) return;
    {
        MutexGuard mg(regmtx_);
        if ( connected_ ) return;
    }
    time_t now = time(0);
    if ( nextConnectTime() > now ) return;
    lastConnect_ = now;
    int res = 0;
    do {
        smsc_log_info( log_, "Connecting to pvss host=%s:%d timeout=%d",
                       pers_->host.c_str(), pers_->port, pers_->timeout );
        if ( sock_.Init( pers_->host.c_str(), pers_->port, pers_->timeout) == -1 ||
             sock_.Connect() == -1 ) {
            res = CANT_CONNECT;
            break;
        }

        char resp[3];
        if (sock_.Read(resp, 2) != 2)
        {
            sock_.Close();
            res = CANT_CONNECT;
            break;
        }
        resp[2] = 0;
        if (!strcmp(resp, "SB"))
        {
            sock_.Close();
            res = SERVER_BUSY;
            break;
        } else if (strcmp(resp, "OK")) {
            sock_.Close();
            res = UNKNOWN_RESPONSE;
            break;
        }
        connected_ = true;
        isReading_ = false;
        smsc_log_info( log_, "Connected to pvss host=%s:%d",
                       pers_->host.c_str(), pers_->port );
        // cleanup
        wrBuffer.setPos(0);
        wrBuffer.setLength(0);
        wrBufSent = 0;
        rdBuffer.setPos(0);
        rdBuffer.setLength(0);
        rdToRead = 0;

        if ( pers_->async ) {
            PersCall* ctx = pers_->createAuthCall();
            if ( ctx ) prepareWrBuffer( ctx );
            while ( connected_ && wrBuffer.length() > 0 ) {
                sendData();
            }
            // receiving answer
            do {
                processInput( false );
            } while ( connected_ && rdBuffer.getPos() != 0 );
        }
    } while ( false );
    if ( res ) {
        smsc_log_warn( log_, "Cannot connect to pvss host=%s:%d: status=%d msg=%s",
                       pers_->host.c_str(), pers_->port, res, exceptionReasons[res]);
    }
}


time_t PvssConnection::nextConnectTime() const
{
    return lastConnect_ + pers_->reconnectTimeout;
}


void PvssConnection::disconnect()
{
    if ( ! connected_ ) return;
    {
        MutexGuard mg(regmtx_);
        if ( ! connected_ ) return;
        connected_ = false;
        lastConnect_ = 0;
        sock_.Close();
        // cleanup all calls in registry
        // WE SHOULD NOT DO IT HERE BECAUSE IT IS A LONG OPERATION.
        // callqueue_.erase( callqueue_.begin(), callqueue_.end() );
        // pers_->disconnected( *this );
    }
    smsc_log_info( log_, "disconnected from pvss host=%s:%d", pers_->host.c_str(), pers_->port );
}


void PvssConnection::dropCalls()
{
    if ( connected_ ) return;
    MutexGuard mg(regmtx_);
    if ( connected_ ) return;
    callqueue_.erase( callqueue_.begin(), callqueue_.end() );
    callhash_.Empty();
}


void PvssConnection::dropExpiredCalls()
{
    MutexGuard mg(regmtx_);
    const msectime_type now = msectime();
    while ( ! callqueue_.empty() ) {
        Call c = callqueue_.front();
        const msectime_type tmo = now - (c.stamp + pers_->timeout);
        if ( tmo > 0 ) {
            if ( c.ctx) {
                c.ctx->setStatus(TIMEOUT);
                c.ctx->initiator()->continuePersCall(c.ctx,false);
            }
            callhash_.Delete( c.seqnum );
            callqueue_.pop_front();
        } else {
            break;
        }
    }
}


void PvssConnection::prepareWrBuffer( PersCall* ctx )
{
    assert( wrBuffer.getPos() == 0 && wrBuffer.length() == 0 && wrBufSent == 0 );
    int32_t seqnum;
    if ( ctx->cmdType() == PC_BIND_ASYNCH || ! pers_->async ) {
        seqnum = 0;
    } else {
        seqnum = ++seqnum_;
        if ( ! seqnum ) seqnum = ++seqnum_;
    }
    try {
        ctx->fillSB(wrBuffer,seqnum);
        if ( ctx->status() == 0 ) {
            wrBufSent = 0;
            if ( logd_->isDebugEnabled() ) {
                std::auto_ptr<char> buf = ::bufdump( wrBuffer, wrBuffer.length() );
                smsc_log_debug( logd_, "out to=%p len=%d seq=%d: %s", this, wrBuffer.length(), seqnum, buf.get() );
            }
            addCall(seqnum,ctx);
            // smsc_log_debug(log_,"Prepared buffer size %d",wrBuffer.length());
            ctx = 0;
        }
    } catch ( PersClientException& e ) {
        smsc_log_warn( log_, "exception in serialization: %s", e.what() );
        ctx->setStatus( e.getType(), e.what() );
    } catch ( std::exception& e ) {
        smsc_log_warn( log_, "exception in serialization: %s", e.what() );
        ctx->setStatus( UNKNOWN_EXCEPTION, e.what() );
    } catch ( ... ) {
        smsc_log_warn( log_, "exception in serialization: unknown" );
        ctx->setStatus( UNKNOWN_EXCEPTION );
    }
    if ( ctx ) {
        wrBuffer.setPos(0);
        wrBuffer.setLength(0);
        ctx->initiator()->continuePersCall(ctx,false);
    }
}


void PvssConnection::addCall( int32_t seqnum, PersCall* ctx )
{
    {
        MutexGuard mg(regmtx_);
        if ( ! callhash_.Exist(seqnum) ) {
            callqueue_.push_back( Call(seqnum,ctx,msectime()) );
            Callqueue::iterator i = callqueue_.end();
            callhash_.Insert( seqnum, --i );
            return;
        }
    }
    smsc_log_error( log_, "non-unique seqnum %d", seqnum );
    ctx->setStatus( BAD_REQUEST );
    ctx->initiator()->continuePersCall( ctx, false );
}


PersCall* PvssConnection::getCall( int32_t seqnum )
{
    {
        MutexGuard mg(regmtx_);
        Callqueue::iterator* i = callhash_.GetPtr(seqnum);
        if ( i ) {
            Call c = **i;
            callhash_.Delete( seqnum );
            callqueue_.erase(*i);
            return c.ctx;
        }
    }
    smsc_log_info( log_, "unknown seqnum=%d", seqnum );
    return 0;
}


void PvssConnection::setReading( bool state )
{
    MutexGuard mg(regmtx_);
    isReading_ = state;
}

} // namespace client
} // namespace pvss
} // namespace scag2
