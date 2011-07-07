#include <sys/time.h>
#include <poll.h>
#include <algorithm>

// #include "core/network/Socket.hpp"
// #include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "scag/exc/SCAGExceptions.h"
// #include "scag/util/UnlockMutexGuard.h"
#include "scag/pvss/base/PersCall.h"
#include "scag/pvss/base/PersClientException.h"
#include "scag/pvss/base/PersServerResponse.h"
#include "PvssStreamClient.h"
#include "PvssConnection.h"
#include "PvssConnector.h"
#include "PvssReader.h"
#include "PvssWriter.h"


using namespace smsc::core::synchronization;

namespace {

class PvssCmdDestroy : public scag2::pvss::PersCallInitiator
{
public:
    virtual void continuePersCall( scag2::pvss::PersCall* call, bool ) {
        delete call;
    }
};

PvssCmdDestroy pvssCmdDestroy;

class PvssReqSyncer : public scag2::pvss::PersCallInitiator
{
public:
    PvssReqSyncer( scag2::pvss::PersClient& c ) : c_(c) {}
    bool call( scag2::pvss::PersCall* call ) {
        MutexGuard mg(mon_);
        if ( ! c_.callAsync(call,*this) ) return false;
        mon_.wait();
        return true;
    }
    virtual void continuePersCall( scag2::pvss::PersCall*, bool )
    {
        MutexGuard mg(mon_);
        mon_.notify();
    }

private:
    smsc::core::synchronization::EventMonitor mon_;
    scag2::pvss::PersClient&                  c_; 
};

}


namespace scag2 {
namespace pvss {
namespace client {

/*
// ============================================================
class PersClientTask : public smsc::core::threads::ThreadedTask
{
protected:
    /// return current time (msec) counting from thread start time
    typedef int utime_type;
    utime_type utime() const;

    /// a request sent to a server
    struct Call {
        Call( PersCall* c, int32_t s, utime_type t ) : serial(s), ctx(c), stamp(t) {}
        void continueExecution( int stat, bool drop );
        int32_t                   serial;
        PersCall*                 ctx;
        utime_type                stamp;
    };

public:
    PersClientTask( PvssStreamClient* pers, bool async );

    /// return true if the task has any asynchronous requests
    bool hasRequests() const { return ! callqueue_.empty(); } // no locking here

    /// controlled poll, return true if something has been read from socket
    bool pollwait(int tmomsec, int rpipe);

protected:
    virtual int Execute();
    virtual const char* taskName() { return "perstask"; }
    virtual void onRelease() { this->disconnect(); }

    void ping();
    bool asyncRead();
    void connect( bool fromDisconnect = false );
    bool disconnect( bool tryToReconnect = false );
    void sendPacket(SerialBuffer& sb);
    int32_t readPacket(SerialBuffer& sb);
    void writeAllTo( const char* buf, uint32_t sz );
    void readAllTo( char* buf, uint32_t sz );
    int32_t getNextSerial();
    void addCall( int32_t serial, PersCall* ctx, utime_type utm );

private:
    PvssStreamClient*           pers_;
    struct timeval              time0_;
    bool                        async_;
    smsc::logger::Logger*       log_;
    smsc::logger::Logger*       logd_;
    smsc::core::network::Socket sock;
    bool                        connected_;
    time_t                      actTS_;
    int                         serial_;

    typedef std::list< Call > Callqueue;
    typedef smsc::core::buffers::IntHash< Callqueue::iterator > Callhash;
    Callqueue                 callqueue_;
    Callhash                  callhash_;
};
 */


// =========================================================================
PvssStreamClient::PvssStreamClient() :
port(0),
timeout(10),
pingTimeout(100),
reconnectTimeout(10),
async(true),
maxCallsCount_(1000),
clients_(0),
isStopping(true),
log_(smsc::logger::Logger::getInstance("pvss.clnt")),
headContext_(0),
tailContext_(0),
callsCount_(0),
connector_(0)
{
}


PvssStreamClient::~PvssStreamClient()
{
    Stop();
    readers_.Empty();
    writers_.Empty();
    connector_ = 0;
}


void PvssStreamClient::Stop()
{
    isStopping = true;
    for ( int i = 0; i < writers_.Count(); ++i ) {
        writers_[i]->notify();
    }
    tp_.stopNotify();
    {
        MutexGuard mg(queueMonitor_);
        queueMonitor_.notifyAll();
        queueMonitor_.wait(50);
        readers_.Empty();
        writers_.Empty();
        connector_ = 0;
    }
    tp_.shutdown();
    PersCall* ctx = 0;
    {
        MutexGuard mg(queueMonitor_);
        ctx = headContext_;
        headContext_ = tailContext_ = 0;
        callsCount_ = 0;
    }
    finishCalls(ctx,true);
    for ( int i = 0; i < connections_.Count(); ++i ) {
        delete connections_[i];
    }
    connections_.Empty();
}


void PvssStreamClient::init( const char *_host,
                             int _port,
                             int _timeout, 
                             int _pingTimeout,
                             int _reconnectTimeout,
                             unsigned _maxCallsCount,
                             unsigned clients,
                             unsigned connPerThread,
                             bool aSync )
{
    if ( ! isStopping ) return;
    isStopping = false;
    smsc_log_info( log_, "PersClient init host=%s:%d timeout=%d, pingtimeout=%d reconnectTimeout=%d maxWaitingRequestsCount=%d connections=%d connPerThread=%d",
                   _host, _port, _timeout, _pingTimeout, _reconnectTimeout,
                   _maxCallsCount, clients, connPerThread );
    callsCount_ = 0;
    host = _host;
    port = _port;
    timeout = _timeout;
    pingTimeout = _pingTimeout;
    reconnectTimeout = _reconnectTimeout;
    maxCallsCount_ = _maxCallsCount;
    clients_ = clients;
    connPerThread_ = connPerThread;
    async = aSync;
    tp_.startTask( connector_ = new PvssConnector( *this ) );
    for ( unsigned i = 0; i < clients_; ++i ) {
        if ( i % connPerThread_ == 0 ) {
            PvssWriter* w = new PvssWriter( *this );
            tp_.startTask(w);
            writers_.Push(w);
            PvssReader* r = new PvssReader( *this, w );
            tp_.startTask(r);
            readers_.Push(r);
        }
        PvssConnection* con = new PvssConnection( *this );
        connections_.Push( con );
        connector_->addConnection( *con );
    }
}


void PvssStreamClient::waitForCalls( int msec )
{
    MutexGuard mg(queueMonitor_);
    if ( isStopping ) return;
    if ( headContext_ ) return;
    queueMonitor_.wait( msec );
}


PersCall* PvssStreamClient::getCall()
{
    if ( isStopping ) return 0;
    if ( ! connected_.Count() ) return 0;
    MutexGuard mg(queueMonitor_);
    if ( ! headContext_ ) return 0;
    PersCall* ctx = headContext_;
    headContext_ = headContext_->next();
    --callsCount_;
    return ctx;
}


PersCall* PvssStreamClient::createPingCall()
{
    if ( isStopping ) return 0;
    PersCall* ctx = new PersCall( PT_UNKNOWN, new PersCommandPing, 0 );
    setInitiator( ctx, &pvssCmdDestroy );
    return ctx;
}


PersCall* PvssStreamClient::createAuthCall()
{
    if ( isStopping ) return 0;
    PersCall* ctx = new PersCall( PT_UNKNOWN, new PersCommandAuth, 0 );
    setInitiator( ctx, &pvssCmdDestroy );
    return ctx;
}


void PvssStreamClient::connected( PvssConnection& conn )
{
    MutexGuard mg(connMonitor_);
    for ( int i = 0; i < connected_.Count(); ++i ) {
        if ( connected_[i] == &conn ) return;
    }
    if ( isStopping ) return;
    connected_.Push(&conn);
    int i = 0;
    for ( ; i < readers_.Count(); ++i ) {
        if ( readers_[i]->sockets() < connPerThread_ ) {
            readers_[i]->addConnection( conn );
            writers_[i]->addConnection( conn );
            // writers_[i]->notify();
            // queueMonitor_.notify();
            break;
        }
    }
    if ( i >= readers_.Count() ) {
        smsc_log_error( log_, "logic error: no r/w thread available" );
        abort();
    }
}


void PvssStreamClient::disconnected( PvssConnection& conn )
{
    MutexGuard mg(connMonitor_);
    for ( int i = 0; i < connected_.Count(); ++i ) {
        if ( connected_[i] == &conn ) {
            connected_.Delete(i);
            if ( ! isStopping && connector_ ) {
                connector_->addConnection(conn);
            }
        }
    }
}


void PvssStreamClient::checkConnections()
{
    PersCall* ctx = 0;
    {
        MutexGuard mg(queueMonitor_);
        if ( connected_.Count() == 0 ) {
            ctx = headContext_;
            headContext_ = tailContext_ = 0;
            callsCount_ = 0;
        }
    }
    if ( ctx ) finishCalls( ctx, isStopping );
}


/*
void PvssStreamClient::configChanged()
{
    // FIXME: any reaction on config change?
}
 */


void PvssStreamClient::finishCalls( PersCall* ctx, bool drop )
{
    while ( ctx ) {
        ctx->setStatus(NOT_CONNECTED);
        PersCall* c = ctx;
        ctx = ctx->next();
        c->initiator()->continuePersCall(c, drop);
    }
}


bool PvssStreamClient::callAsync( PersCall* ctx, PersCallInitiator& fromwho )
{
    if ( !ctx ) return false;
    MutexGuard mg(queueMonitor_);
    do {
        // PersCallParams* p = static_cast<PersCallParams*>(ctx->getParams());
        if ( isStopping ) {
            ctx->setStatus( CANT_CONNECT );
        } else if ( ! connected_.Count() ) {
            ctx->setStatus( NOT_CONNECTED );
        } else if ( callsCount_ >= maxCallsCount_ ) {
            ctx->setStatus( CLIENT_BUSY );
        } else if ( ctx->initiator() && ctx->initiator() != &fromwho ) {
            smsc_log_warn(log_, "perscall already has initiator" );
            ctx->setStatus( BAD_REQUEST );
        } else {
            setInitiator( ctx, &fromwho );
            break;
        }
        return false;
    } while ( false );
    if ( headContext_ )
        setNext( tailContext_, ctx );
    else
        headContext_ = ctx;
    tailContext_ = ctx;
    ++callsCount_;
    queueMonitor_.notify();
    return true;
}


bool PvssStreamClient::callSync( PersCall* call )
{
    PvssReqSyncer s(*this);
    return s.call( call );
}


int PvssStreamClient::getClientStatus()
{
    MutexGuard mg(queueMonitor_);
    if ( ! connected_.Count() ) return NOT_CONNECTED;
    if ( callsCount_ >= maxCallsCount_ ) return CLIENT_BUSY;
    return 0;
}


// ==================================================================
/*
PersClientTask::utime_type PersClientTask::utime() const
{
    struct timeval tv;
    ::gettimeofday( &tv, 0 );
    utime_type t = utime_type((tv.tv_sec - time0_.tv_sec)*1000 + (int(tv.tv_usec) - int(time0_.tv_usec)) / 1000);
    return t;
}


void PersClientTask::Call::continueExecution( int stat, bool drop )
{
    if ( ! ctx ) return;
    ctx->setStatus( stat );
    ctx->initiator()->continuePersCall( ctx, drop );
}


PersClientTask::PersClientTask( PvssStreamClient* pers, bool async ) :
pers_(pers),
async_(async),
log_(0),
logd_(0),
connected_(false),
actTS_(time(0)),
serial_(0)
{
    ::gettimeofday(&time0_, 0);
    log_ = smsc::logger::Logger::getInstance("perstask");
    logd_ = smsc::logger::Logger::getInstance("pers.dump");
}


bool PersClientTask::pollwait(int tmomsec,int rpipe)
{
    bool res = false;
    struct pollfd pfd[2];
    pfd[0].fd = sock.getSocket();
    pfd[0].events = POLLIN;
    int np = 1;
    if ( rpipe > 0 ) {
        np = 2;
        pfd[1].fd = rpipe;
        pfd[1].events = POLLIN;
    }
    // smsc_log_debug( log_, "perspoll: tmo=%d", tmomsec );
    int rc = poll( pfd, np, tmomsec );
    if ( rc < 0 ) {
        smsc_log_warn(log_, "pollwait failed: %d", rc);
    } else if ( rc > 0 ) {
        if ( rpipe > 0 && ( pfd[1].revents & POLLIN ) ) {
            // smsc_log_debug( log_, "perspoll: waked up, on ctrl" );
            char buf[10];
            ::read( rpipe, buf, sizeof(buf) );
            // smsc_log_debug( log_, "perspoll: ctrl has been read" );
            // } else {
            // smsc_log_debug( log_, "perspoll: waked up, no ctrl" );
        }
        if ( pfd[0].revents & POLLIN ) {
            try {
                res = asyncRead();
            } catch(...) {
            }
        }
        // } else {
        // smsc_log_debug( log_, "perspoll: timeouted" );
    }
    return res;
}


int PersClientTask::Execute()
{
    smsc_log_info( log_, "perstask started" );
    while ( true ) {

        if ( isStopping ) break;

        if ( ! connected_ ) {
            try {
                this->connect();
            } catch (...) {
            }
            if ( ! connected_ ) {
                if ( isStopping ) break;
                pers_->wait( pers_->reconnectTimeout*1000 );
                continue;
            }
        }

        int tmo = int(actTS_ + pers_->pingTimeout - time(0));
        if ( tmo < 0 ) tmo = 0;
        tmo *= 1000;
        PersCall* ctx = pers_->getCall(tmo, *this);
        if ( isStopping ) {
            if ( ctx ) ctx->initiator()->continuePersCall(ctx,true);
            break;
        }

        if ( ! ctx ) {
            ping();
            continue;
        } else if ( hasRequests() ) {
            // try to pollwait again
            pollwait(0,0);
        }

        // processing
        int32_t serial = async_ ? getNextSerial() : 0;
        try {
            SerialBuffer sb;
            ctx->fillSB(sb,serial);
            if ( ! ctx->status() ) {
                sendPacket(sb);
                const utime_type utm = utime();
                smsc_log_debug(log_, "perscall: serial=%d stamp=%u perscmd=%d/%s %s/%d",
                               serial,
                               utm,
                               ctx->cmdType(),
                               persCmdName(ctx->cmdType()),
                               ctx->getStringKey(), ctx->getIntKey() );
                if ( async_ ) {
                    addCall( serial,ctx,utm );
                    continue;
                } else {
                    readPacket(sb);
                    ctx->readSB(sb);
                }
            }
        } catch ( PersClientException& e ) {
            smsc_log_warn( log_, "execute pers exception: exc=%s", e.what() );
            ctx->setStatus( e.getType(), e.what() );
            this->disconnect(true);
        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "execute exception: exc=%s", e.what() );
            ctx->setStatus( UNKNOWN_EXCEPTION, e.what() );
        } catch (...) {
            smsc_log_warn( log_, "execute unknown exception" );
            ctx->setStatus( UNKNOWN_EXCEPTION, "pers: Unknown exception" );
        }

        ctx->initiator()->continuePersCall(ctx,false);
    }
    this->disconnect(false);
    smsc_log_info( log_, "perstask finished" );
    return 0;
}


void PersClientTask::ping()
{
    if ( pers_->pingTimeout <= 0 ) return;
    time_t now = time(0);
    if ( actTS_ + pers_->pingTimeout > now ) return;
    SerialBuffer sb;
    uint32_t sz = 0;
    sb.WriteInt32(sz);
    int32_t serial = getNextSerial();
    if ( async_ ) {
        sb.WriteInt32(serial);
    }
    sb.WriteInt8(PC_PING);
    sz = sb.length();
    sb.SetPos(0);
    sb.WriteInt32(sz);
    sb.SetPos(sz);
    try {
        sendPacket(sb);
        if ( async_ ) {
            const utime_type utm = utime();
            smsc_log_debug(log_, "persping: serial=%d lcmcmd=%d stamp=%u",
                           serial, PC_PING, utm );
            addCall(serial,0,utm);
        } else {
            readPacket(sb);
            PersServerResponseType rt = (PersServerResponseType)sb.ReadInt8();
            if ( rt != RESPONSE_OK )
                throw PersClientException( SERVER_ERROR );
            actTS_ = now;
        }
        // smsc_log_debug( log_, "ping sent");
    } catch ( PersClientException& e ) {
        smsc_log_warn( log_, "ping failed: %s", e.what() );
        this->disconnect(true);
        // it will try to connect later
    } catch (...) {
        smsc_log_warn(log_, "ping: unknown exception");
    }
}


bool PersClientTask::asyncRead()
{
    bool res = false;
    do {
        SerialBuffer sb;
        int32_t serial = readPacket(sb);
        // find out a given call
        Callqueue::iterator* i = callhash_.GetPtr(serial);
        if ( !i ) {
            smsc_log_debug( log_, "async unknown serial=%d", serial );
            // continue;
        } else {
            Call c = **i;
            callhash_.Delete(serial);
            callqueue_.erase(*i);
            if ( c.ctx ) {
                // valid context
                try {
                    c.ctx->readSB(sb);
                } catch (...) {
                    c.ctx->setStatus(BAD_RESPONSE);
                }
                c.ctx->initiator()->continuePersCall(c.ctx,false);
                res = true;
            }
        }

        // trying to poll one more time
        if ( ! hasRequests() ) break;

        pollfd pfd;
        pfd.fd = sock.getSocket();
        pfd.events = POLLIN;
        int rc = poll( &pfd, 1, 0);
        if ( rc < 0 ) {
            smsc_log_warn( log_, "async poll failed: %d", rc );
            break;
        } else if ( (rc > 0) && (pfd.revents & POLLIN) != 0 ) {
            continue;
        } else {
            break;
        }
    } while ( true );

    const utime_type now = utime();
    // remove old requests
    while ( hasRequests() ) {
        Call& c = callqueue_.front();
        const utime_type tmo = now - (c.stamp + pers_->timeout);
        if ( tmo > 0 ) {
            smsc_log_debug( log_, "async request has expired: serial=%d, stamp=%u, tmo=%u",
                            c.serial, c.stamp, tmo );
            c.continueExecution(TIMEOUT,false);
            callhash_.Delete( c.serial );
            callqueue_.pop_front();
        } else {
            break;
        }
    }
    return res;
}


void PersClientTask::connect( bool fromDisconnect )
{
    if ( connected_ ) return;
    int res = 0;
    do {
        smsc_log_info( log_, "Connecting to persserver host=%s:%d timeout=%d",
                       pers_->host.c_str(), pers_->port, pers_->timeout );

        if ( sock.Init( pers_->host.c_str(), pers_->port, pers_->timeout) == -1 ||
             sock.Connect() == -1 ) {
            res = CANT_CONNECT;
            break;
        }

        char resp[3];
        if (sock.Read(resp, 2) != 2)
        {
            sock.Close();
            res = CANT_CONNECT;
            break;
        }
        resp[2] = 0;
        if (!strcmp(resp, "SB"))
        {
            sock.Close();
            res = SERVER_BUSY;
            break;
        } else if (strcmp(resp, "OK")) {
            sock.Close();
            res = UNKNOWN_RESPONSE;
            break;
        }

        connected_ = true;
        if (!fromDisconnect) pers_->incConnect();
        actTS_ = time(0);

        if ( async_ ) {
            SerialBuffer sb;
            sb.WriteInt32(0);
            sb.WriteInt8(PC_BIND_ASYNCH);
            uint32_t len = sb.length();
            sb.SetPos(0);
            sb.WriteInt32(len);
            sb.SetPos(len);
            try {
                async_ = false;
                sendPacket(sb);
                readPacket(sb);
                PersServerResponseType rt = (PersServerResponseType)sb.ReadInt8();
                if ( rt != RESPONSE_OK ) {
                    res = SERVER_ERROR;
                }
            } catch (...) {
                res = UNKNOWN_EXCEPTION;
            }
            async_ = true;
        }
    } while ( false );

    if ( res )  {
        disconnect(false);
        smsc_log_info( log_, "Connection failed: %d %s", res, exceptionReasons[res] );
        throw PersClientException(PersClientExceptionType(res));
    }
    smsc_log_info(log_, "PersClientTask connected");
}


bool PersClientTask::disconnect( bool tryToReconnect )
{
    if ( !connected_ ) return false;
    try {
        sock.Close();
        connected_ = false;
        if ( tryToReconnect ) {
            this->connect( true );
            return true; // connected
        }
    } catch (...) {
    }
    pers_->decConnect();
    // clean up all sent packets
    while ( hasRequests() ) {
        Call& c = callqueue_.front();
        c.continueExecution( CANT_CONNECT, !tryToReconnect );
        callqueue_.pop_front();
    }
    callhash_.Empty();
    smsc_log_info( log_, "PersClientTask disconnected" );
    return false;
}


void PersClientTask::sendPacket( SerialBuffer& bsb )
{
    for(;;)
    {
        try{
            this->connect();
            if ( hasRequests() ) pollwait(0,0);
            writeAllTo( bsb.c_ptr(), bsb.length() );
            actTS_ = time(0);
            smsc_log_debug( logd_, "write to socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str() );
            return;
        }
        catch ( PersClientException &e )
        {
            smsc_log_warn( log_, "PersClientException: %s", e.what() );
            if ( !this->disconnect( true ) ) throw;
        }
    }
}


int32_t PersClientTask::readPacket( SerialBuffer& bsb )
{
    if (!connected_)
        throw PersClientException(NOT_CONNECTED);

    bsb.Empty();
    char tmp_buf[1024];
    readAllTo( tmp_buf, static_cast<uint32_t>(sizeof(uint32_t)) );
    bsb.Append(tmp_buf, static_cast<uint32_t>(sizeof(uint32_t)));
    bsb.SetPos(0);
    uint32_t sz = bsb.ReadInt32() - static_cast<uint32_t>(sizeof(uint32_t));
    smsc_log_debug(logd_, "%d bytes will be read from socket", sz);

    while (sz > 0)
    {
        uint32_t minsz = (sz > 1024 ? 1024 : sz);
        readAllTo(tmp_buf, minsz);
        bsb.Append(tmp_buf, minsz);
        sz -= minsz;
    }
    smsc_log_debug(logd_, "read from socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str());
    bsb.SetPos(0);
    sz = bsb.ReadInt32();
    actTS_ = time(0);
    int32_t serial = async_ ? bsb.ReadInt32() : 0;
    unsigned int pos = bsb.GetPos();
    int8_t resp = bsb.ReadInt8();
    bsb.SetPos(pos);
    smsc_log_debug( log_, "persread: serial=%d sz=%u result=%u/%s", serial, sz,
                    resp, persServerResponse(resp) );
    return serial;
}


void PersClientTask::writeAllTo( const char* buf, uint32_t sz )
{
    int cnt;
    uint32_t wr = 0;
    struct pollfd pfd;    

    while (sz)
    {
        pfd.fd = sock.getSocket();
        pfd.events = POLLOUT;
        int rc = poll(&pfd, 1, pers_->timeout);
        if ( rc < 0) {
            throw PersClientException(SEND_FAILED);
        } else if ( rc == 0 ) {
            // timeout
            if ( wr == 0 && hasRequests() && pollwait(0,0) ) continue;
            throw PersClientException(TIMEOUT);
        } else if ( pfd.revents & (POLLERR | POLLHUP | POLLNVAL) ) {
            throw PersClientException(SEND_FAILED);
        } else if ( !(pfd.revents & POLLOUT) ) {
            throw PersClientException(SEND_FAILED);
        }

        cnt = sock.Write(buf + wr, sz);
        if (cnt <= 0)
            throw PersClientException(SEND_FAILED);

        wr += cnt;
        sz -= cnt;
    }
}


void PersClientTask::readAllTo( char* buf, uint32_t sz )
{
    int cnt;
    uint32_t rd = 0;
    struct pollfd pfd;

    while(sz)
    {
        pfd.fd = sock.getSocket();
        pfd.events = POLLIN;
        if(poll(&pfd, 1, pers_->timeout) <= 0 || !(pfd.revents & POLLIN))
            throw PersClientException(TIMEOUT);

        cnt = sock.Read(buf + rd, sz);
        if(cnt <= 0)
            throw PersClientException(READ_FAILED);

        rd += cnt;
        sz -= cnt;
    }
}


int32_t PersClientTask::getNextSerial()
{
    int32_t s = ++serial_;
    if ( !s ) s = ++serial_;
    return s;
}


void PersClientTask::addCall( int32_t serial, PersCall* ctx, utime_type utm )
{
    if ( callhash_.Exist(serial) ) {
        smsc_log_error( log_, "non-unique serial %d", serial );
        ctx->initiator()->continuePersCall( ctx, false );
        return;
    }
    callqueue_.push_back( Call(ctx,serial,utm) );
    Callqueue::iterator i = callqueue_.end();
    callhash_.Insert( serial, --i );
}
 */

}
}
}
