#include <poll.h>
#include "PersClient2.h"
#include "scag/util/singleton/Singleton2.h"
#include "core/threads/Thread.hpp"
#include "core/threads/ThreadPool.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/network/Socket.hpp"
#include "scag/exc/SCAGExceptions.h"

namespace {

bool inited = false;
Mutex initLock;

}

namespace scag2 {

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
               int _maxCallsCount,
               unsigned clients );

    lcm::LongCallContextBase* getContext( int tmo );

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

    void finishCalls( lcm::LongCallContextBase* ctx );
    virtual bool call( lcm::LongCallContextBase* ctx );
    virtual int getClientStatus();

public:
    std::string host;
    int         port;
    int         timeout;
    int         pingTimeout;
    int         reconnectTimeout;

private:
    int      maxCallsCount;
    unsigned clients_;

    // smsc::core::network::Socket sock;
    bool isStopping;
    smsc::logger::Logger* log_;
    // Mutex mtx;
    // EventMonitor connectMonitor;

    EventMonitor              queueMonitor_;
    lcm::LongCallContextBase* headContext_;
    lcm::LongCallContextBase* tailContext_;
    unsigned                  callsCount_;
    unsigned                  connected_;
    smsc::core::threads::ThreadPool tp_;
};


// ============================================================
class PersClientTask : public smsc::core::threads::ThreadedTask
{
public:
    PersClientTask( PersClientImpl* pers );
    // : pers_(pers) {
    // log_ = smsc::logger::Logger::getInstance( "perstask" );
    // }

protected:
    virtual int Execute();
    virtual const char* taskName() { return "perstask"; }
    virtual void onRelease() { this->disconnect(); }

    // void stop();
    void ping();
    void connect();
    void disconnect();
    void sendPacket( SerialBuffer& sb );
    void readPacket( SerialBuffer& sb );
    void writeAllTo( const char* buf, uint32_t sz );
    void readAllTo( char* buf, uint32_t sz );

private:
    PersClientImpl*             pers_;
    smsc::logger::Logger*       log_;
    smsc::logger::Logger*       logd_;
    smsc::core::network::Socket sock;
    bool                        connected_;
    time_t                      actTS_;
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
                       int _maxCallsCount,
                       unsigned clients ) //throw(PersClientException)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            PersClientImpl& pc = SinglePC::Instance();
            pc.init( _host, _port, _timeout, _pingTimeout, _reconnectTimeout, _maxCallsCount, clients );
            inited = true;
        }
    }
}


void PersClient::Init( const config::PersClientConfig& cfg )
{
    Init( cfg.host.c_str(), cfg.port, cfg.timeout, cfg.pingTimeout, cfg.reconnectTimeout, cfg.maxCallsCount, cfg.connections );
} 


// =========================================================================
PersClientImpl::PersClientImpl() :
config::ConfigListener(config::PERSCLIENT_CFG),
port(0),
timeout(10),
pingTimeout(100),
reconnectTimeout(10),
maxCallsCount_(1000),
clients_(0),
isStopping(true),
log_(0),
headContext_(0),
tailContext_(0),
callsCount_(0),
connected_(0)
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
    finishCalls(ctx);
}


void PersClientImpl::init( const char *_host, int _port, int _timeout, int _pingTimeout, int _reconnectTimeout, int _maxCallsCount, unsigned clients )
{
    smsc_log_info( log_, "PersClient init host=%s:%d timeout=%d, pingtimeout=%d reconnectTimeout=%d maxWaitingRequestsCount=%d connections=%d",
                   _host, _port, _timeout, _pingTimeout, _reconnectTimeout, _maxCallsCount,
                   clients );
    connected_ = 0;
    callsCount_ = 0;
    host = _host;
    port = _port;
    timeout = _timeout;
    pingTimeout = _pingTimeout;
    reconnectTimeout = _reconnectTimeout;
    maxCallsCount = _maxCallsCount;
    clients_ = clients;
    for ( unsigned i = 0; i < clients_; ++i ) {
        tp_.startTask( new PersClientTask(this) );
    }
    isStopping = false;
}


lcm::LongCallContextBase* PersClientImpl::getContext( int tmo )
{
    if ( isStopping || !connected_ ) return 0;
    lcm::LongCallContextBase* ctx = 0;
    MutexGuard mg(queueMonitor_);
    if ( !headContext_) {
        queueMonitor_.wait( tmo * 1000 );
        if ( isStopping || !connected_ ) return 0;
    }
    ctx = headContext_;
    if (headContext_) {
        headContext_ = headContext_->next;
        if (!headContext_) tailContext_ = 0;
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
    finishCalls( ctx );
}


void PersClientImpl::configChanged()
{
    // FIXME: any reaction on config change?
}


void PersClientImpl::finishCalls( lcm::LongCallContextBase* ctx )
{
    while ( ctx ) {
        lcm::LongCallContextBase* c = ctx;
        ctx = ctx->next;
        c->initiator->continueExecution( c, true );
    }
}


bool PersClientImpl::call( lcm::LongCallContextBase* ctx )
{
    if ( isStopping || !connected_ || !ctx ) return false;
    MutexGuard mg(queueMonitor_);
    if ( isStopping || !connected_ ) return false;
    if ( callsCount_ >= maxCallsCount ) return false;

    ctx->next = 0;
    if ( headContext_ )
        tailContext_->next = ctx;
    else
        headContext_ = ctx;
    tailContext_ = ctx;
    ++callsCount_;
    queueMonitor_.notify();
    return true;
}


int PersClientImpl::getClientStatus()
{
    if ( ! connected_ ) return NOT_CONNECTED;
    MutexGuard mg(queueMonitor_);
    if ( callsCount_ >= maxCallsCount ) return CLIENT_BUSY;
    return 0;
}


// ==================================================================

PersClientTask::PersClientTask( PersClientImpl* pers ) :
pers_(pers),
log_(0),
logd_(0),
connected_(false),
actTS_(time(0))
{
    log_ = smsc::logger::Logger::getInstance("perstask");
    logd_ = smsc::logger::Logger::getInstance("pers.dump");
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

        int tmo = actTS_ + pers_->pingTimeout - time(0);
        lcm::LongCallContextBase* ctx = pers_->getContext( tmo );
        if ( isStopping ) {
            if ( ctx ) ctx->initiator->continueExecution( ctx, true );
            break;
        }

        if ( ! ctx ) {
            ping();
            continue;
        }

        // processing
        PersCallParams* p = static_cast< PersCallParams* >(ctx->getParams());
        smsc_log_debug(log_, "ExecutePersCall: command=%d %s/%d", ctx->callCommandId, p->getStringKey(), p->getIntKey() );
        try {
            if ( p->error == 0 ) {
                SerialBuffer& b = p->proxy().buffer();
                sendPacket( b );
                readPacket( b );
            }
        } catch ( PersClientException& e ) {
            p->error = e.getType();
            p->exception = e.what();
        } catch ( std::exception& e ) {
            p->error = -1;
            p->exception = e.what();
        } catch (...) {
            p->error = -1;
            p->exception = "lcm: Unknown exception";
        }

        ctx->initiator->continueExecution( ctx, false );
        actTS_ = time(0);
    }
    this->disconnect();
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
    sb.WriteInt8(PC_PING);
    sz = sb.length();
    sb.SetPos(0);
    sb.WriteInt32(sz);
    sb.SetPos(sz);
    try {
        sendPacket(sb);
        readPacket(sb);
        PersServerResponseType rt = (PersServerResponseType)sb.ReadInt8();
        if ( rt != RESPONSE_OK )
            throw PersClientException( SERVER_ERROR );
        actTS_ = now;
        smsc_log_debug( log_, "ping sent");
    } catch ( PersClientException& e ) {
        smsc_log_error( log_, "ping failed: %s", e.what() );
        this->disconnect();
        // it will try to connect later
    } catch (...) {
        smsc_log_error(log_, "unknown exception");
    }
}


void PersClientTask::connect()
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
    connected_ = true;
    pers_->incConnect();
    actTS_ = time(0);
    smsc_log_debug(log_, "PersClient connected");
}


void PersClientTask::disconnect()
{
    pers_->decConnect();
    sock.Close();
    connected_ = false;
    smsc_log_debug( log_, "PersClient disconnected" );
}


void PersClientTask::sendPacket( SerialBuffer& bsb )
{
    uint32_t  t = 0;
    
    for(;;)
    {
        try{
            this->connect();
            writeAllTo( bsb.c_ptr(), bsb.length() );
            smsc_log_debug( logd_, "write to socket: len=%d, data=%s", bsb.length(), bsb.toString().c_str() );
            return;
        }
        catch ( PersClientException &e )
        {
            smsc_log_debug( log_, "PersClientException: %s", e.what() );
            this->disconnect();
            if (++t >= 2) throw;
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
        pfd.events = POLLOUT | POLLIN;
        if ( poll(&pfd, 1, pers_->timeout) <= 0)
            throw PersClientException(TIMEOUT);
        else if (!(pfd.revents & POLLOUT) || (pfd.revents & POLLIN))
            throw PersClientException(SEND_FAILED);		

        cnt = sock.Write(buf + wr, sz);
        if(cnt <= 0)
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

}
}
}
