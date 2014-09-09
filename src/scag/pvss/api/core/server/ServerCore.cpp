#include <ctime>
#include <cstring>
#include <cstdio>

#include "ServerCore.h"
#include "ContextQueue.h"
#include "ServerNewContext.h"
#include "Worker.h"
#include "scag/pvss/api/packets/PingResponse.h"
#include "scag/pvss/api/packets/ErrorResponse.h"
#include "scag/pvss/api/packets/ResponseVisitor.h"
#include "scag/pvss/api/packets/RequestVisitor.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/counter/Accumulator.h"
#include "scag/counter/Manager.h"
#include "util/findConfigFile.h"
#include "license/check/license.hpp"

using namespace smsc::core::buffers;
using smsc::core::synchronization::MutexGuard;

namespace {

using namespace scag2::pvss;

struct ErrorResponseVisitor : public ResponseVisitor
{
    virtual bool visitErrResponse( ErrorResponse& resp ) { status = false; return true; }
    virtual bool visitPingResponse( PingResponse& resp ) { return false; }
    virtual bool visitAuthResponse( AuthResponse& resp ) { return false; }
    virtual bool visitProfileResponse( ProfileResponse& resp ) { status = true; return true; }
    bool status;
};

struct PRVisitor : public RequestVisitor
{
    virtual bool visitPingRequest( PingRequest& ) { return false; }
    virtual bool visitAuthRequest( AuthRequest& ) { return false; }
    virtual bool visitProfileRequest( ProfileRequest& ) { return true; }
};

}


namespace scag2 {
namespace pvss {
namespace core {
namespace server {

ServerCore::ServerCore( ServerConfig* config,
                        Protocol* protocol ) :
Core(config,protocol), Server(),
log_(smsc::logger::Logger::getInstance(taskName())),
loge_(smsc::logger::Logger::getInstance("pvssEsrv")),
started_(false),
syncDispatcher_(0),
last_(config->getStatisticsInterval()),
hasNewStats_(false),
exceptions_(new ExceptionCount),
nextLicenseTime_(0),
licenseFileTime_(0)
{
}


ServerCore::~ServerCore()
{
    shutdown();
}


void ServerCore::checkLicenseFile()
{
    const time_t now = time(0);
    if ( nextLicenseTime_ >= now ) {
        return;
    }
    MutexGuard mg(licenseLock_);
    if ( nextLicenseTime_ >= now ) {
        return;
    }
    const char* what;
    do {
        const std::string licenseIni = smsc::util::findConfigFile("license.ini");
        const std::string licenseSig = smsc::util::findConfigFile("license.sig");
        struct stat st;
        if ( 0 != stat(licenseIni.c_str(),&st) ) {
            what = "license if not found";
            break;
        }
        if ( licenseFileTime_ == st.st_mtime ) {
            // file has not been changed
            return;
        }
        static const char* keys[] = {
            "Organization",
            "Hostids",
            "LicenseExpirationDate",
            "LicenseType",
            "Product"
        };
        smsc::core::buffers::Hash<std::string> licenseHash;
        if ( !smsc::license::check::CheckLicense( licenseIni.c_str(),
                                                  licenseSig.c_str(),
                                                  licenseHash,
                                                  keys,
                                                  sizeof(keys)/sizeof(keys[0])) ) {
            what = "invalid license";
            break;
        }
        licenseFileTime_ = st.st_mtime;
        nextLicenseTime_ = now + 600;

        if ( !smsc::license::check::checkHostIds(licenseHash["Hostids"].c_str()) ) {
          what = "invalid license.";
          break;
//          throw std::runtime_error("code 1");
        }

        return;

    } while ( false );
    smsc_log_error(log_,"%s",what);
    fprintf(stderr,"%s\n",what);
    std::terminate();
}


bool ServerCore::acceptChannel( PvssSocketPtr& channel )
{
    regset_.create(channel.get());
    try {
        const bool accepted = Core::registerChannel(channel,util::currentTimeMillis());
        if ( accepted ) {
            PvssSockPtr ptr(channel.get());
            MutexGuard mg(channelMutex_);
            managedChannels_.push_back(ptr);
            channels_.push_back(ptr);
            return true;
        }
    } catch (...) {}
    regset_.pop(channel.get());
    return false;
}


void ServerCore::acceptOldChannel( PvssSocketBase* channel )
{
    regset_.create(channel);
    {
        MutexGuard mg(channelMutex_);
        channels_.push_back(PvssSockPtr(channel));
    }
}


void ServerCore::contextProcessed( ServerContextPtr& context) // /* throw (PvssException) */ 
{
    try {
        sendResponse(context);
        // writer will notify of sending success/failure
        return;
    } catch ( PvssException& e ) {
        smsc_log_debug( loge_, "exception(%u): %s", __LINE__, e.what());
        countExceptions( e.getType(), "sendResponse" );
    } catch (std::exception& e) {
        // static unsigned counter = 0;
        // if ( counter++ % 100 == 0 )
        smsc_log_debug( loge_, "exception(%u): %s", __LINE__, e.what());
        countExceptions( PvssException::UNKNOWN, "sendResponse" );
    }
    if (context.get()) {
        // writing failure is notified immediately
        context->setState(ServerContext::FAILED);
        reportContext(context.get());
    }
}


void ServerCore::receivePacket( std::auto_ptr<Packet> packet, PvssSocket& channel )
{
    if ( !packet.get() || !packet->isRequest() ) {
        countExceptions(PvssException::BAD_REQUEST, "recvPacket" );
        return;
    }

    updateChannelActivity(channel);
    Request* req = static_cast<Request*>(packet.release());
    ServerContextPtr ctx(new ServerNewContext(req,channel));
    receiveContext( ctx );
}


void ServerCore::receiveOldPacket( ServerContextPtr& ctx )
{
    // do we need some checks on this context?
    receiveContext(ctx);
}


void ServerCore::reportPacket( uint32_t seqNum,
                               PvssSocketBase& channel,
                               PacketState state )
{
    ServerContextPtr ctx;
    Response* response = 0;
    {
        ContextRegistryPtr ptr = regset_.get(&channel);
        if (!ptr) {
//            smsc_log_debug( loge_,"packet seqNum=%d on channel %p sock=%p reported, but registry is not found",
//                            seqNum, &channel, channel.getSocket());
            countExceptions( PvssException::UNKNOWN, "reportMissReg");
            return;
        }
        ContextPtr i = ptr->get(seqNum);
        if ( !i ) {
//            smsc_log_debug(loge_, "packet seqNum=%d on channel %p sock=%p reported as %s, but not found",
//                           seqNum, &channel, channel.getSocket(),
//                           packetStateToString(state));
            countExceptions( PvssException::UNKNOWN, "reportMissSeqnum" );
            return;
        }
        response = i->getResponse().get();
        ctx.reset( static_cast<ServerContext*>(i.get()) );
        if ( ! response ) {
            // cannot be: response must be present
            smsc_log_error(loge_, "packet seqNum=%d on channel %p sock=%p reported as %s, but no resp in context found",
                           seqNum, &channel, channel.getSocket(),
                           packetStateToString(state));
            countExceptions( PvssException::UNKNOWN, "reportMissResp");
            return;
        }
        switch (state) {
        case SENT : ctx->setState( ServerContext::SENT ); break;
        case EXPIRED : ctx->setState( ServerContext::FAILED ); break;
        case FAILED : ctx->setState( ServerContext::FAILED ); break;
        default: ctx->setState( ServerContext::FAILED ); break;
        }
    }
    if (ctx.get()) reportContext(ctx.get());
}


void ServerCore::init()
{
    if (acceptor_.get()) return;
    acceptor_.reset(new Acceptor(getConfig(),*this));
    acceptor_->init();
}


void ServerCore::startup( SyncDispatcher& dispatcher ) /* throw (PvssException) */ 
{
    if (started_) return;
    MutexGuard mg(startMutex_);
    if (started_) return;

    smsc_log_info(log_,"Starting PVSS (sync_disp) server %s", getConfig().toString().c_str() );
    init();
    if ( !acceptor_.get() ) throw PvssException(PvssException::UNKNOWN, "no acceptor found");

    syncDispatcher_ = &dispatcher;

    startupIO();
    try {
        // create workers
        for (unsigned i = 0; ; ++i) {
            SyncLogic* logic = dispatcher.getSyncLogic(i);
            if (!logic) break;
            Worker* worker = new Worker(*logic,*this);
            workers_.Push(worker);
            threadPool_.startTask(worker,false);
        }
        // acceptor_.reset(new Acceptor(getConfig(),*this));
        // acceptor_->init();
        threadPool_.startTask(acceptor_.get(),false);
    } catch (PvssException& exc) {
        smsc_log_error(log_,"Sync dispatcher start error: %s",exc.what());
        if (acceptor_.get()) acceptor_->shutdown();
        shutdownIO(false);
        throw exc;
    }

    started_ = true;
    threadPool_.startTask(this,false);
}


void ServerCore::startup( AsyncDispatcher& dispatcher ) /* throw (PvssException) */ 
{
    if (started_) return;
    MutexGuard mg(startMutex_);
    if (started_) return;

    smsc_log_info(log_,"Starting PVSS (async_disp) server %s", getConfig().toString().c_str() );
    init();
    if ( !acceptor_.get() ) throw PvssException(PvssException::UNKNOWN, "no acceptor found");

    startupIO();
    try {
        // create a dispatcher thread
        dispatcher_.reset( new AsyncDispatcherThread(*this,dispatcher) );
        dispatcher_->init();
        threadPool_.startTask(dispatcher_.get(),false);
        threadPool_.startTask(acceptor_.get(),false);
    } catch (PvssException& exc) {
        smsc_log_error(log_,"Async dispatcher start error: %s",exc.what());
        if (acceptor_.get()) acceptor_->shutdown();
        if (dispatcher_.get()) {
            dispatcher_->shutdown();
            dispatcher_.reset(0);
        }
        shutdownIO(false);
        throw exc;
    }

    started_ = true;
    // threadPool_.startTask(this,false);
}


void ServerCore::shutdown()
{
    if (!started_) return;
    MutexGuard mg(startMutex_);
    if (!started_) return;

    smsc_log_info(log_, "Server is shutting down...");

    started_ = false;
    if (acceptor_.get()) acceptor_->shutdown();
    shutdownIO(true); // write pending
    smsc_log_info(log_,"All readers/writers are stopped");

    // destroy contexts in regset
    {
        do {
            ContextRegistryPtr ptr = regset_.popAny();
            if (!ptr) break;
            typedef ContextRegistry::ProcessingList ProcList;
            ProcList pl;
            ptr->popAll(pl);
            for ( ProcList::iterator i = pl.begin(), ie = pl.end(); i != ie; ++i ) {
                ServerContext* ctx = static_cast<ServerContext*>(i->get());
                ctx->setState( ServerContext::FAILED );
                reportContext(ctx);
            }
        } while (true);
    }

    stop();
    {
        smsc_log_info(log_,"sending notify to channel mutex");
        MutexGuard mgc(channelMutex_);
        channelMutex_.notify();
    }
    // stop all workers
    for ( int i = 0; i < workers_.Count(); ++i ) {
        workers_[i]->shutdown();
    }
    waitUntilReleased();

    {
        MutexGuard mgs(statMutex_);
        char buf[400];
        const std::string caught = totalExceptions_.toString();
        snprintf(buf, sizeof(buf),
                 "server active parts are shutdowned, statistics follows:\n"
                 "== Total: %s%s%s%s%s",
                 total_.toString().c_str(),
                 caught.empty() ? "" : "\n         total err: ",
                 caught.empty() ? "" : caught.c_str(),
                 syncDispatcher_ ? "\n    Data: " : "",
                 syncDispatcher_ ? syncDispatcher_->reportStatistics().c_str() : "");
        smsc_log_info(log_,"%s",buf);
    }

    for ( int i = 0; i < workers_.Count(); ++i ) {
        delete workers_[i];
    }
    workers_.Empty();
    if ( dispatcher_.get() ) {
        dispatcher_->shutdown();
        dispatcher_.reset(0);
    }
    {
        // move all channels to dead
        MutexGuard mgc(channelMutex_);
        std::copy( channels_.begin(), channels_.end(), std::back_inserter(deadChannels_) );
        managedChannels_.clear();
        channels_.clear();
    }
    destroyDeadChannels();
}


int ServerCore::doExecute()
{
    const int minTimeToSleep = 400; // 10 msec
    // util::msectime_type timeToSleep = getConfig().getProcessTimeout();
    // util::msectime_type currentTime = util::currentTimeMillis();
    // util::msectime_type nextWakeupTime = currentTime + timeToSleep;

    smsc_log_info(log_,"Server started");
    Statistics totalStat;
    Statistics lastStat;
    // register counters to be delivered via snmp
    counter::Manager& mgr = counter::Manager::getInstance();
    counter::CounterPtrAny cntConn = mgr.registerAnyCounter( new counter::Accumulator("sys.connections.new") );
    counter::CounterPtrAny cntTotalReq  = mgr.registerAnyCounter( new counter::Accumulator("sys.requests.total") );
    counter::CounterPtrAny cntReqPerSec  = mgr.registerAnyCounter( new counter::Accumulator("sys.requests.speed") );
    while (!isStopping)
    {
        // smsc_log_debug(log_,"cycling clientCore");
        // currentTime = util::currentTimeMillis();
        // int timeToWait = int(nextWakeupTime-currentTime);
        int timeToWait = minTimeToSleep;
        checkLicenseFile();
        std::auto_ptr<ExceptionCount> exceptions;
        {
            MutexGuard mg(statMutex_);
            statMutex_.wait(timeToWait);
            if ( !hasNewStats_ ) continue;
            totalStat = total_;
            lastStat = last_;
            hasNewStats_ = false;
            last_.reset();
            exceptions = exceptions_;
            exceptions_.reset(new ExceptionCount);
        }
        // to fix statistics on channels
        destroyDeadChannels();
        unsigned conns;
        {
            MutexGuard mg(channelMutex_);
            conns = unsigned(managedChannels_.size());
        }

        totalExceptions_.add( *exceptions.get() );

        char buf[80];
        std::string fullstat;
        fullstat.reserve(512);
        snprintf(buf,sizeof(buf),"Statistics follows: connections=%u", conns );
        fullstat.append(buf);
        //01234567890123456789
        //== Total: HH:MM:SS  req/resp/err=
        //        speed(1/s): req/resp/err=
        //         total err: 6/workProcTmo:12
        //    Last: HH:MM:SS  req/resp/err=
        //        speed(1/s): req/resp/err=
        //          last err: 6/workProcTmo:1
        //    Data:  abonents=xxxxxx locations=16 storages=128
        fullstat.append("\n== Total: ");
        fullstat.append(totalStat.toString());
        std::string caught = totalExceptions_.toString();
        if ( ! caught.empty() ) {
            //                 01234567890123456789
            fullstat.append("\n         total err: ");
            fullstat.append(caught);
        }
        fullstat.append("\n    Last: ");
        fullstat.append(lastStat.toString());
        caught = exceptions->toString();
        if ( ! caught.empty() ) {
            fullstat.append("\n          last err: ");
            fullstat.append(caught);
        }
        util::msectime_type e10ms;
        const unsigned scale = lastStat.prescale(e10ms);
        if ( syncDispatcher_ ) {
            fullstat.append("\n  R/W [pf/s(kb/s)]:");
            fullstat.append(syncDispatcher_->flushIOStatistics(scale,unsigned(e10ms)));
            fullstat.append("\n    Data: ");
            fullstat.append(syncDispatcher_->reportStatistics());
        }

        // counters
        cntConn->setValue(conns);
        cntTotalReq->setValue(totalStat.requests);
        cntReqPerSec->setValue((lastStat.requests*scale+e10ms/2)/e10ms);

        smsc_log_info(log_,"%s",fullstat.c_str());

    }
    smsc_log_info( log_, "Server shutdowned" );
    return 0;
}


void ServerCore::receiveContext( ServerContextPtr& ctx )
{
    // FIXME: update channel activity should be here
    Request* req = ctx->getRequest().get();
    PvssException exc("",PvssException::OK);

    if ( !req ) {
        // smsc_log_error( log_, "context does not contain request");
        countExceptions( PvssException::BAD_REQUEST, "recvNoReq" );
        return;
    }

    uint32_t seqNum = req->getSeqNum();
    const char* what = "recvGeneric"; // short string for statistics
    try {

        if ( !req->isValid() ) {
            // smsc_log_error( log_, "request is not valid");
            what = "recvBadReq";
            throw PvssException(PvssException::BAD_REQUEST,"request is bad formed");
        }

        PRVisitor prv;
        ProfileRequest* preq;
        if ( req->visit(prv) ) {
            preq = static_cast< ProfileRequest* >(req);
        } else {
            preq = 0;
        }

        if (preq) {
            bool needTiming = false;
            unsigned totalRequests;

            // a new request has come
            {
                MutexGuard mg(statMutex_);
                static util::msectime_type timingCounter = 0;
                totalRequests = ++total_.requests;
                ++last_.requests;
                util::msectime_type currentTime = checkStatistics();
                static unsigned successiveTimings = 0;
                if ( successiveTimings ) {
                    --successiveTimings;
                    needTiming = true;
                } else if (currentTime - timingCounter > util::msectime_type(getConfig().getTimingInterval())) {
                    // more than 5 seconds
                    /*
                     // NOTE: simple randomization
                     static unsigned counter = 0;
                     ++counter;
                     if ( (counter % 7) == 0 ) break;
                     counter += unsigned(reinterpret_cast<uint64_t>(static_cast<const void*>(req)));
                     needTiming = true;
                     */
                    timingCounter = currentTime;
                    successiveTimings = getConfig().getTimingSeriesSize() - 1;
                    needTiming = true;
                }
            }

            if ( needTiming ) {
                preq->startTiming();
                char buf[30];
                std::sprintf(buf,"#%u",totalRequests % 100);
                preq->timingComment(buf);
            }
        }

        smsc_log_debug(log_,"packet received %p: %s", req, req->toString().c_str());

        if (!started_) {
            what = "recvAfterStop";
            throw PvssException(PvssException::SERVER_SHUTDOWN,"Server is down");
        }

        if ( req->isPing() ) {
            what = "recvPingSend";
            ctx->setResponse(new PingResponse(seqNum,Response::OK));
            sendResponse(ctx);
            return;
        }

        ContextQueue* queue = 0;
        if ( syncDispatcher_ ) {
            const int idx = syncDispatcher_->getIndex(*req);
            if ( idx > workers_.Count() ) {
                what = "recvNoWorker";
                throw PvssException(PvssException::INVALID_KEY,"cannot dispatch");
            }
            Worker* worker = workers_[idx];
            queue = & worker->getQueue();
        } else if ( dispatcher_.get() ) {
            queue = & dispatcher_->getQueue();
        } else {
            what = "recvNoDisptch";
            throw PvssException(PvssException::CONFIG_INVALID,"dispatcher is not found");
        }

        what = "recvToQueue";
        queue->requestReceived(ctx.get()); // may throw server_busy
        return;

    } catch ( PvssException& e ) {
        smsc_log_debug(loge_, "exception(%u): %s",__LINE__, e.what());
        countExceptions( e.getType(), what );
        exc = e;
    } catch (std::exception& e) {
        smsc_log_debug(loge_, "exception(%u): %s",__LINE__, e.what());
        exc = PvssException(e.what(),PvssException::UNKNOWN);
        countExceptions( PvssException::UNKNOWN, what );
    }

    try {
        ctx->setResponse(new ErrorResponse(seqNum,exc.getType(),exc.what()));
        sendResponse(ctx);
    } catch ( PvssException& e ) {
        smsc_log_debug(loge_,"exception(%u): %s", __LINE__, e.what());
        countExceptions( e.getType(), "recvSendErr" );
    } catch ( std::exception& e ) {
        smsc_log_debug(loge_,"exception(%u): %s", __LINE__, e.what());
        countExceptions( PvssException::UNKNOWN, "recvSendErr" );
    }
}


void ServerCore::sendResponse( ServerContextPtr& ctx ) /* throw (PvssException) */ 
{
    int seqNum = ctx->getSeqNum();
    PvssSocketBase* socket = ctx->getSocket();
//    smsc_log_debug(log_,"sendResp ctx=%p seq=%u on channel %p sock=%p",ctx.get(),seqNum,socket,socket->getSocket());
    const Response* response = ctx->getResponse().get();
    if (!response) {
        throw PvssException(PvssException::BAD_RESPONSE,"response is null");
    }

    ContextRegistryPtr ptr = regset_.get(socket);
    if (!ptr) {
        throw PvssException(PvssException::UNKNOWN,
                            "context registry is not found for channel %p sock=%p",
                            socket,socket->getSocket());
    }
    if ( !response->isValid() ) {
        throw PvssException(PvssException::BAD_RESPONSE,"response '%s' is not valid",
                            response->toString().c_str());
    }
    
    if ( ! ptr->push(ctx.get()) ) {
        throw PvssException(PvssException::SERVER_BUSY,
                            "seqnum=%d is already in registry", seqNum );
    }

    {
        ErrorResponseVisitor erv;
        if ( const_cast<Response*>(response)->visit(erv) ) {
            MutexGuard mg(statMutex_);
            if ( ! erv.status ) {
                ++total_.errors;
                ++last_.errors;
            } else {
                ++total_.responses;
                ++last_.responses;
            }
            checkStatistics();
        }
    }

    try {
 //       smsc_log_debug(log_,"sending response %p to channel %p sock=%p",
 //                      response, socket, socket->getSocket());
        ctx->sendResponse();
    } catch (...) {
        ptr->pop(seqNum);
        // logic should be notified externally as it may take ctx ownership
        throw;
    }
}


void ServerCore::reportContext( ServerContext* ctx )
{
    if ( !ctx ) {
        // smsc_log_warn(log_,"null context reported");
        countExceptions(PvssException::UNKNOWN, "reportNull");
        return;
    }
    ServerContextPtr ctxPtr(ctx);

    Response* response = ctx->getResponse().get();
    if ( !response ) {
        // smsc_log_warn(log_,"context with null response reported");
        countExceptions(PvssException::UNKNOWN, "reportNull" );
        return;
    }

    uint32_t seqNum = ctx->getSeqNum();

    PvssSocketBase* socket = ctx->getSocket();
    if (!socket) {
        smsc_log_debug(log_,"null socket in ctx=%p seq=%u",ctx,unsigned(seqNum));
        countExceptions(PvssException::UNKNOWN,"reportNoSock");
        return;
    }

    ContextRegistryPtr ptr = regset_.get(socket);
    if (!ptr) {
//        smsc_log_debug(log_,"context registry is not found for ctx=%p seq=%u channel %p sock=%p",
//                       ctx,unsigned(seqNum),socket,socket->getSocket());
        countExceptions(PvssException::UNKNOWN,"reportNoReg");
        return;
    }

    // extract from context registry
    if ( !ptr->pop(seqNum).get() ) {
//        smsc_log_debug(log_,"unknown resp ctx=%p seq=%u is reported on channel %p sock=%p",
//                       ctx,unsigned(seqNum),socket,socket->getSocket());
        countExceptions(PvssException::UNKNOWN, "reportUnkSeq" );
        return;
    }

    if ( ctx->getState() == ServerContext::SENT ) {
        {
            MutexGuard mg(statMutex_);
            ++total_.successes;
            ++last_.successes;
            checkStatistics();
        }
        if ( response->isPing() ) {
            smsc_log_debug(log_,"PING response sent");
            return;
        } else {
            smsc_log_debug(log_,"Response '%s' sent",response->toString().c_str());
            ErrorResponseVisitor erv;
            if ( response->visit(erv) && !erv.status ) return;
        }
    } else {
        // failure
        {
            MutexGuard mg(statMutex_);
            ++total_.failures;
            ++last_.failures;
            checkStatistics();
        }
        if ( response->isPing() ) {
            smsc_log_debug(log_,"PING response was not sent, state: %s",
                           ctx->getState() == ServerContext::FAILED ? "FAILED" : "EXPIRED" );
            return;
        } else {
            smsc_log_debug(log_,"Response '%s' was not sent, state: %s",response->toString().c_str(),
                           ctx->getState() == ServerContext::FAILED ? "FAILED" : "EXPIRED" );
            ErrorResponseVisitor erv;
            if ( response->visit(erv) && !erv.status ) return;
        }
    }

    ContextQueue* queue = ctx->getRespQueue();
    if ( !queue ) {
        smsc_log_debug(loge_,"packet seqNum=%d has no resp queue set",seqNum);
        countExceptions( PvssException::UNKNOWN, "reportNoQueue");
    } else {
        queue->reportResponse(ctx);
    }
}


void ServerCore::closeChannel( PvssSocketBase& socket )
{
    bool our = false;
    {
        MutexGuard mg(channelMutex_);
        ChannelList::iterator i = std::find( channels_.begin(),
                                             channels_.end(),
                                             &socket );
        if ( i == channels_.end() ) return;
        deadChannels_.push_back(*i);
        channels_.erase(i);

        i = std::find( managedChannels_.begin(),
                       managedChannels_.end(),
                       &socket);
        if ( i == managedChannels_.end() ) {
            // this one is an external channel, should not be worried about...
            socket.getSocket()->Close();
        } else {
            managedChannels_.erase(i);
            our = true;
        }
    }
    if (our) {
        Core::closeChannel(static_cast<PvssSocket&>(socket));
    }
    destroyDeadChannels();
}


void ServerCore::stopCoreLogic()
{
    // send signals to all queue, wait until all workers are finished.
    smsc_log_info(log_,"signalling worker thread queues");
    for ( int i = 0; i < workers_.Count(); ++i ) {
        workers_[i]->getQueue().stop();
    }
    if ( dispatcher_.get() ) {
        dispatcher_->getQueue().stop();
    }
    smsc_log_info(log_,"waiting until all workers threads process their requests" );
    while (true) {
        MutexGuard mg(logicMon_);
        int workingCount = 0;
        for ( int i = 0; i < workers_.Count(); ++i ) {
            if ( ! workers_[i]->stopping() ) ++workingCount;
        }
        if ( dispatcher_.get() && !dispatcher_->stopping() ) ++workingCount;
        if ( workingCount == 0 ) break;
        logicMon_.wait(200);
    }

    // all workers and async logic dispatcher are stopping,
    // i.e. they have no pending processing requests and async logic is also stopped.
    // Now we have to wait until all pending outgoing contexts are written to their sockets.
}


void ServerCore::destroyDeadChannels()
{
    ChannelList allDead;
    {
        MutexGuard mg(channelMutex_);
        if ( deadChannels_.empty() ) return;
        allDead.swap(deadChannels_);
    }

    for ( ChannelList::iterator i = allDead.begin();
          i != allDead.end();
          ++i ) {

        ContextRegistryPtr ptr = regset_.pop(i->get());
        if (!ptr) continue;
        typedef ContextRegistry::ProcessingList ProcList;
        ProcList pl;
        ptr->popAll(pl);
        for ( ProcList::iterator i = pl.begin(), ie = pl.end(); i != ie; ++i ) {
            ServerContext* ctx = static_cast<ServerContext*>(i->get());
            ctx->setState( ServerContext::FAILED );
            reportContext( ctx );
        }
    }
}


util::msectime_type ServerCore::checkStatistics()
{
    const util::msectime_type currentTime = util::currentTimeMillis();
    total_.checkTime(currentTime);
    if ( last_.checkTime(currentTime) ) {
        hasNewStats_ = true;
        statMutex_.notify();
        /*
        smsc_log_info(log_, "total: %s", total_.toString().c_str() );
        smsc_log_info(log_, "last %u sec: %s", unsigned(last_.accumulationTime/1000), last_.toString().c_str() );
        last_.reset();
         */
    } else if ( last_.elapsedTime == 0 ) {
        // the first entrance to statistics
        // adjusting to the edge of time slots
        static util::msectime_type ref = 0;
        if (!ref) {
            time_t now = std::time(0);
            struct tm nowtm;
            std::memset(&nowtm,0,sizeof(nowtm));
            gmtime_r(&now,&nowtm);
            nowtm.tm_sec = 0;
            nowtm.tm_min = 0;
            nowtm.tm_hour = 0;
            ref = util::msectime_type(mktime(&nowtm)) * 1000;
        }
        const util::msectime_type wid = getConfig().getStatisticsInterval();
        const util::msectime_type nbins = (last_.startTime - ref + wid/2) / wid;
        const util::msectime_type endt = ref + (nbins+1)*wid;
        last_.accumulationTime = endt - last_.startTime;
    }
    return currentTime;
}


void ServerCore::countExceptions( PvssException::Type et, const char* where )
{
    MutexGuard mg(statMutex_);
    exceptions_->count( et, where );
}


std::string ServerCore::ExceptionCount::toString() const
{
    std::string result;
    result.reserve(200);
    int k;
    bool first = true;
    Hash< unsigned >* hash;
    char buf[60];
    unsigned curlen = 0;
    for ( Count::Iterator i(count_); i.Next(k,hash); ) {
        char* where;
        unsigned value;
        for ( Hash< unsigned >::Iterator j(hash); j.Next(where,value); ) {
            snprintf(buf,sizeof(buf),"%u/%s:%u", k, where, value);
            unsigned buflen = unsigned(strlen(buf));
            if ( curlen + buflen > 73 ) {
                //               01234567890123456789
                result.append("\n                    ");
                curlen = 20;
            } else if ( first ) {
                first = false;
                curlen = 20; // initial padding
            } else {
                result.append("  ");
                curlen += 2;
            }
            result.append(buf);
            curlen += buflen;
        }
    }
    return result;
}


void ServerCore::ExceptionCount::add( const ExceptionCount& o )
{
    int k;
    Hash< unsigned >* ohash;
    for ( Count::Iterator i(o.count_); i.Next(k,ohash); ) {
        if ( ohash && ohash->GetCount() ) {
            Hash< unsigned >* ptr = count_.GetPtr(k);
            if ( !ptr ) ptr = & count_.Insert(k,Hash<unsigned>());
            char* where;
            unsigned value;
            for ( Hash< unsigned >::Iterator j(ohash); j.Next(where,value); ) {
                unsigned* count = ptr->GetPtr(where);
                if ( !count ) count = ptr->SetItem(where,0);
                (*count) += value;
            }
        }
    }
}


void ServerCore::ExceptionCount::count( PvssException::Type et, const char* where )
{
    Hash< unsigned >* ptr = count_.GetPtr(et);
    if ( !ptr ) ptr = & count_.Insert(et,Hash<unsigned>());
    unsigned* count = ptr->GetPtr(where);
    if ( !count ) count = ptr->SetItem(where,0);
    ++(*count);
}

} // namespace server
} // namespace core
} // namespace pvss
} // namespace scag2
