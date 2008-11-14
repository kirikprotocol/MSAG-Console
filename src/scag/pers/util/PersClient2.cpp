#include <poll.h>
#include "PersClient2.h"
#include "PersCallParams.h"
#include "scag/util/singleton/Singleton2.h"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "scag/exc/SCAGExceptions.h"
#include "scag/util/UnlockMutexGuard.h"

namespace {

bool inited = false;
Mutex initLock;

}

namespace scag2 {

using util::UnlockMutexGuard;
using util::singleton::SingletonHolder;

namespace pers {
namespace util {

class PersClientTask;

class PersClientImpl : public PersClient, public config::ConfigListener
{
public:
    PersClientImpl();
    virtual ~PersClientImpl();
    virtual void Stop();
    void init( const char *_host,
               int _port,
               int _timeout,
               int _pingTimeout,
               int _reconnectTimeout,
               unsigned _maxCallsCount,
               int expireTmo,
               unsigned clients,
               bool     async );

    lcm::LongCallContextBase* getContext( int tmo, PersClientTask& task );

    /// increment connection count
    void incConnect();

    /// decrement connection count, if count == 0 then finish all current queued calls
    void decConnect();

    void wait( int sec ) {
        MutexGuard mg(queueMonitor_);
        queueMonitor_.wait( sec*1000 );
    }

private:
    // void init();
    // void startClients();
    virtual void configChanged();

    // virtual int Execute();

    void finishCalls( lcm::LongCallContextBase* ctx, bool drop );
    virtual bool call( lcm::LongCallContextBase* ctx );
    virtual int getClientStatus();

public:
    std::string host;
    int         port;
    int         timeout;
    int         pingTimeout;
    int         reconnectTimeout;
    int         expireTimeout;

private:
    unsigned maxCallsCount_;
    unsigned clients_;
    bool     async_;

    // smsc::core::network::Socket sock;
    bool isStopping;
    smsc::logger::Logger* log_;
    // Mutex mtx;
    // EventMonitor connectMonitor;

    struct Pipe {
        Pipe() { ::pipe(fd); }
        inline void notify() const { ::write(fd[1], "0", 1); }
        inline int rpipe() const { return fd[0];}
        inline void close() { ::close(fd[0]); ::close(fd[1]); }
        bool operator == ( const Pipe& p ) const {
            return (fd[0] == p.fd[0]) && (fd[1] == p.fd[1]);
        }

        // inline int wpipe() const { return fd[1];}
    private:
        int fd[2];
    };

    EventMonitor              queueMonitor_;
    lcm::LongCallContextBase* headContext_;
    lcm::LongCallContextBase* tailContext_;
    unsigned                  callsCount_;
    unsigned                  connected_;
    std::list<Pipe>           waitingPipes_;   // a list of waiting pipes
    std::list<Pipe>           freePipes_;      // a list of free pipes
    unsigned                  waitingStreams_; // a number of streams waiting w/o pipe
    smsc::core::threads::ThreadPool tp_;
};


// ============================================================
class PersClientTask : public smsc::core::threads::ThreadedTask
{
protected:
    /// a request sent to a server
    struct Call {
        Call(lcm::LongCallContextBase* c, int32_t s ) : serial(s), ctx(c), stamp(time(0)) {}
        int32_t                   serial;
        lcm::LongCallContextBase* ctx;
        time_t                    stamp;
    };

public:
    PersClientTask( PersClientImpl* pers, bool async );
    // : pers_(pers) {
    // log_ = smsc::logger::Logger::getInstance( "perstask" );
    // }

    /// return true if the task has any asynchronous requests
    bool hasRequests() const { return ! callqueue_.empty(); } // no locking here

    /// controlled poll
    void pollwait( int tmo, int rpipe );

protected:
    virtual int Execute();
    virtual const char* taskName() { return "perstask"; }
    virtual void onRelease() { this->disconnect(); }

    // void stop();
    void ping();
    bool asyncRead();
    void connect( bool fromDisconnect = false );
    bool disconnect( bool tryToReconnect = false );
    void sendPacket(SerialBuffer& sb);
    void readPacket(SerialBuffer& sb);
    void writeAllTo( const char* buf, uint32_t sz );
    void readAllTo( char* buf, uint32_t sz );
    int32_t getNextSerial();
    void addCall( int32_t serial, lcm::LongCallContextBase* ctx );

private:
    PersClientImpl*             pers_;
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



inline unsigned GetLongevity(PersClient*) { return 251; }
typedef SingletonHolder< PersClientImpl > SinglePC;

PersClient& PersClient::Instance() 
{
    if ( ! inited ) {
        MutexGuard mg(initLock);
        if (!inited) throw exceptions::SCAGException("Pers client not inited");
    }
    return SinglePC::Instance();
}


void PersClient::Init( const char *_host,
                       int _port,
                       int _timeout,
                       int _pingTimeout,
                       int _reconnectTimeout,
                       unsigned _maxCallsCount,
                       int expireTmo,
                       unsigned clients,
                       bool     async ) //throw(PersClientException)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            PersClientImpl& pc = SinglePC::Instance();
            pc.init( _host, _port, _timeout, _pingTimeout, _reconnectTimeout, _maxCallsCount, expireTmo, clients, async );
            inited = true;
        }
    }
}


void PersClient::Init( const config::PersClientConfig& cfg )
{
    Init( cfg.host.c_str(), cfg.port, cfg.timeout, cfg.pingTimeout, cfg.reconnectTimeout, cfg.maxCallsCount, cfg.expireTimeout, cfg.connections, cfg.async );
} 


// =========================================================================
PersClientImpl::PersClientImpl() :
config::ConfigListener(config::PERSCLIENT_CFG),
port(0),
timeout(10),
pingTimeout(100),
reconnectTimeout(10),
expireTimeout(10),
maxCallsCount_(1000),
clients_(0),
async_(false),
isStopping(true),
log_(0),
headContext_(0),
tailContext_(0),
callsCount_(0),
connected_(0),
waitingStreams_(0)
{
    log_ = smsc::logger::Logger::getInstance("persclient");
}


PersClientImpl::~PersClientImpl()
{
    Stop();
}


void PersClientImpl::Stop()
{
    isStopping = true;
    tp_.stopNotify();
    {
        MutexGuard mg(queueMonitor_);
        queueMonitor_.notifyAll();
        for ( std::list<Pipe>::const_iterator i = waitingPipes_.begin();
              i != waitingPipes_.end();
              ++i ) {
            i->notify();
        }
        queueMonitor_.wait(50);
    }
    tp_.shutdown();
    lcm::LongCallContextBase* ctx = 0;
    {
        MutexGuard mg(queueMonitor_);
        ctx = headContext_;
        headContext_ = tailContext_ = 0;
        callsCount_ = 0;
    }
    finishCalls(ctx,true);
}


void PersClientImpl::init( const char *_host,
                           int _port,
                           int _timeout, 
                           int _pingTimeout,
                           int _reconnectTimeout,
                           unsigned _maxCallsCount,
                           int expireTmo,
                           unsigned clients,
                           bool async )
{
    smsc_log_info( log_, "PersClient init host=%s:%d timeout=%d, pingtimeout=%d reconnectTimeout=%d maxWaitingRequestsCount=%d expireTmo=%d connections=%d async=%d",
                   _host, _port, _timeout, _pingTimeout, _reconnectTimeout, _maxCallsCount,
                   expireTmo,
                   clients, async ? 1 : 0 );
    connected_ = 0;
    callsCount_ = 0;
    host = _host;
    port = _port;
    timeout = _timeout;
    pingTimeout = _pingTimeout;
    reconnectTimeout = _reconnectTimeout;
    expireTimeout = expireTmo;
    maxCallsCount_ = _maxCallsCount;
    clients_ = clients;
    async_ = async;
    for ( unsigned i = 0; i < clients_; ++i ) {
        tp_.startTask( new PersClientTask(this, async_) );
    }
    isStopping = false;
}


lcm::LongCallContextBase* PersClientImpl::getContext( int tmo, PersClientTask& task )
{
    if ( isStopping || !connected_ ) return 0;
    lcm::LongCallContextBase* ctx = 0;
    MutexGuard mg(queueMonitor_);
    do {
        if ( headContext_ || tmo <= 0 ) break;

        // no context found
        if ( task.hasRequests() ) {
            if ( freePipes_.empty() ) {
                waitingPipes_.push_back( Pipe() );
            } else {
                waitingPipes_.push_back( freePipes_.back() );
                freePipes_.pop_back();
            }
            Pipe p( waitingPipes_.back() );
            {
                UnlockMutexGuard umg(queueMonitor_);
                task.pollwait(tmo,p.rpipe());
            }
            std::list<Pipe>::iterator i = std::find( waitingPipes_.begin(), waitingPipes_.end(), p );
            if ( i != waitingPipes_.end() ) {
                waitingPipes_.erase(i);
            }
            freePipes_.push_back(p);
        } else {
            ++waitingStreams_;
            queueMonitor_.wait( tmo * 1000 );
            --waitingStreams_;
        }
        if ( isStopping || !connected_ ) return 0;
    } while ( false );
    ctx = headContext_;
    if (headContext_) {
        headContext_ = headContext_->next;
        // if (!headContext_) tailContext_ = 0;
        --callsCount_;
    }
    return ctx;
}


void PersClientImpl::incConnect()
{
    MutexGuard mg(queueMonitor_);
    ++connected_;
}


void PersClientImpl::decConnect()
{
    lcm::LongCallContextBase* ctx = 0;
    {
        MutexGuard mg(queueMonitor_);
        if ( --connected_ == 0 ) {
            ctx = headContext_;
            headContext_ = tailContext_ = 0;
            callsCount_ = 0;
        }
    }
    finishCalls(ctx,false);
}


void PersClientImpl::configChanged()
{
    // FIXME: any reaction on config change?
}


void PersClientImpl::finishCalls( lcm::LongCallContextBase* ctx, bool drop )
{
    while ( ctx ) {
        lcm::LongCallContextBase* c = ctx;
        PersCallParams* persParam = (PersCallParams*)ctx->getParams();
        if ( persParam ) {
            persParam->setStatus(NOT_CONNECTED);
            // persParam->exception = strs[NOT_CONNECTED];
        }
        ctx = ctx->next;
        c->initiator->continueExecution(c,drop);
    }
}


bool PersClientImpl::call( lcm::LongCallContextBase* ctx )
{
    if ( isStopping || !connected_ || !ctx ) return false;
    MutexGuard mg(queueMonitor_);
    if ( isStopping || !connected_ ) return false;
    if ( callsCount_ >= maxCallsCount_ ) return false;

    ctx->next = 0;
    if ( headContext_ )
        tailContext_->next = ctx;
    else
        headContext_ = ctx;
    tailContext_ = ctx;
    ++callsCount_;
    if ( waitingStreams_ > 0 ) {
        queueMonitor_.notify();
    } else if ( ! waitingPipes_.empty() ) {
        waitingPipes_.front().notify();
        freePipes_.push_back( waitingPipes_.front() );
        waitingPipes_.erase( waitingPipes_.begin() );
    }
    return true;
}


int PersClientImpl::getClientStatus()
{
    if ( ! connected_ ) return NOT_CONNECTED;
    MutexGuard mg(queueMonitor_);
    if ( callsCount_ >= maxCallsCount_ ) return CLIENT_BUSY;
    return 0;
}


// ==================================================================

PersClientTask::PersClientTask( PersClientImpl* pers, bool async ) :
pers_(pers),
async_(async),
log_(0),
logd_(0),
connected_(false),
actTS_(time(0)),
serial_(0)
{
    log_ = smsc::logger::Logger::getInstance("perstask");
    logd_ = smsc::logger::Logger::getInstance("pers.dump");
}


void PersClientTask::pollwait( int tmo, int rpipe )
{
    struct pollfd pfd[2];
    pfd[0].fd = sock.getSocket();
    pfd[0].events = POLLIN;
    pfd[1].fd = rpipe;
    pfd[1].events = POLLIN;
    int rc = poll( pfd, 2, tmo*1000 );
    if ( rc < 0 ) {
        smsc_log_warn(log_, "pollwait failed: %d", rc);
    } else if ( rc > 0 ) {
        if ( ( pfd[1].revents & POLLIN ) ) {
            char buf[10];
            read( rpipe, buf, sizeof(buf) );
        }
    }
}


int PersClientTask::Execute()
{
    smsc_log_debug( log_, "perstask started" );
    while ( true ) {

        if ( isStopping ) break;

        if ( ! connected_ ) {
            try {
                this->connect();
            } catch (...) {
            }
            if ( ! connected_ ) {
                if ( isStopping ) break;
                pers_->wait( pers_->reconnectTimeout );
                continue;
            }
        }

        if ( hasRequests() ) {
            // check if there is any results from server
            if ( asyncRead() ) continue;
        }

        int tmo = actTS_ + pers_->pingTimeout - time(0);
        lcm::LongCallContextBase* ctx = pers_->getContext(tmo, *this);
        if ( isStopping ) {
            if ( ctx ) ctx->initiator->continueExecution(ctx, true);
            break;
        }

        if ( ! ctx ) {
            ping();
            continue;
        }

        // processing
        PersCallParams* p = static_cast< PersCallParams* >(ctx->getParams());
        smsc_log_debug(log_, "ExecutePersCall: lcmcmd=%d perscmd=%d/%s %s/%d",
                       ctx->callCommandId, p->cmdType(), persCmdName(p->cmdType()), p->getStringKey(), p->getIntKey() );
        try {
            SerialBuffer sb;
            int32_t serial = getNextSerial();
            p->fillSB(sb,serial);
            if ( ! p->status() ) {
                sendPacket(sb);
                if ( async_ ) {
                    addCall(serial,ctx);
                } else {
                    readPacket(sb);
                    p->readSB(sb);
                }
            }
        } catch ( PersClientException& e ) {
            smsc_log_warn( log_, "execute failed: pers exc=%s", e.what() );
            p->setStatus( e.getType(), e.what() );
            this->disconnect( true );
        } catch ( std::exception& e ) {
            smsc_log_warn( log_, "execute failed: exc=%s", e.what() );
            p->setStatus( -1, e.what() );
        } catch (...) {
            p->setStatus( -1, "lcm: Unknown exception" );
        }

        ctx->initiator->continueExecution(ctx,false);
        actTS_ = time(0);
    }
    this->disconnect(false);
    smsc_log_debug( log_, "perstask finished" );
    return 0;
}


void PersClientTask::ping()
{
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
            addCall(serial,0);
        } else {
            readPacket(sb);
            PersServerResponseType rt = (PersServerResponseType)sb.ReadInt8();
            if ( rt != RESPONSE_OK )
                throw PersClientException( SERVER_ERROR );
            actTS_ = now;
        }
        smsc_log_debug( log_, "ping sent");
    } catch ( PersClientException& e ) {
        smsc_log_warn( log_, "ping failed: %s", e.what() );
        this->disconnect();
        // it will try to connect later
    } catch (...) {
        smsc_log_error(log_, "unknown exception");
    }
}


bool PersClientTask::asyncRead()
{
    pollfd pfd;
    pfd.fd = sock.getSocket();
    pfd.events = POLLIN;
    time_t now = time(0);
    // remove old requests
    while ( hasRequests() ) {
        Call& c = callqueue_.front();
        if ( c.stamp + pers_->expireTimeout < now ) {
            smsc_log_info( log_, "async request has expired: %d", c.serial );
            if ( c.ctx ) {
                PersCallParams* params = static_cast<PersCallParams*>(c.ctx->getParams());
                params->setStatus(TIMEOUT);
                c.ctx->initiator->continueExecution(c.ctx,false);
            }
            callhash_.Delete( c.serial );
            callqueue_.pop_front();
        } else {
            break;
        }
    }

    if ( ! hasRequests() ) return false;

    bool res = false;
    while ( true ) {
        int rc = poll( &pfd, 1, 0);
        if ( rc < 0 ) {
            smsc_log_warn( log_, "async poll failed: %d", rc );
            throw PersClientException(READ_FAILED);
        } else if ( (rc > 0) && (pfd.revents & POLLIN) != 0 ) {
            SerialBuffer sb;
            readPacket(sb);
            int32_t serial = sb.ReadInt32();
            // find out a given call
            Callqueue::iterator* i = callhash_.GetPtr(serial);
            if ( !i ) {
                smsc_log_warn( log_, "async unknown serial: %d", serial );
                continue;
            }
            Call c = **i;
            callhash_.Delete(serial);
            callqueue_.erase(*i);
            if ( ! c.ctx ) continue; // a ping ?
            PersCallParams* params = static_cast<PersCallParams*>( c.ctx->getParams() );
            try {
                params->readSB(sb);
            } catch (...) {
                params->setStatus(BAD_RESPONSE);
            }
            c.ctx->initiator->continueExecution( c.ctx, false );
            res = true;
        } else {
            break;
        }
    }
    return res;
}


void PersClientTask::connect( bool fromDisconnect )
{
    if ( connected_ ) return;
    smsc_log_info( log_, "Connecting to persserver host=%s:%d timeout=%d",
                   pers_->host.c_str(), pers_->port, pers_->timeout );

    if ( sock.Init( pers_->host.c_str(), pers_->port, pers_->timeout) == -1 ||
         sock.Connect() == -1 )
        throw PersClientException(CANT_CONNECT);

    char resp[3];
    if (sock.Read(resp, 2) != 2)
    {
        sock.Close();
        throw PersClientException(CANT_CONNECT);
    }
    resp[2] = 0;
    if (!strcmp(resp, "SB"))
    {
        sock.Close();
        throw PersClientException(SERVER_BUSY);
    } else if (strcmp(resp, "OK"))
    {
        sock.Close();
        throw PersClientException(UNKNOWN_RESPONSE);
    }

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
            if ( rt != RESPONSE_OK )
                throw PersClientException( SERVER_ERROR );
        } catch (...) {
            async_ = true;
            throw;
        }
        async_ = true;
    }
    connected_ = true;
    if (!fromDisconnect) pers_->incConnect();
    actTS_ = time(0);
    smsc_log_debug(log_, "PersClientTask connected");
}


bool PersClientTask::disconnect( bool tryToReconnect )
{
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
    smsc_log_debug( log_, "PersClientTask disconnected" );
    return false;
}


void PersClientTask::sendPacket( SerialBuffer& bsb )
{
    for(;;)
    {
        try{
            this->connect();
            if ( hasRequests() ) asyncRead();
            writeAllTo( bsb.c_ptr(), bsb.length() );
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


void PersClientTask::readPacket( SerialBuffer& bsb )
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
            if ( wr == 0 && hasRequests() && asyncRead() ) continue;
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


void PersClientTask::addCall( int32_t serial, lcm::LongCallContextBase* ctx )
{
    if ( callhash_.Exist(serial) ) {
        smsc_log_error( log_, "non-unique serial %d", serial );
        ctx->initiator->continueExecution( ctx, false );
        return;
    }
    callqueue_.push_back( Call(ctx,serial) );
    Callqueue::iterator i = callqueue_.end();
    callhash_.Insert( serial, --i );
}

}
}
}
