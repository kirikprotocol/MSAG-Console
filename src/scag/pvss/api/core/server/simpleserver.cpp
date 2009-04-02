#include "ServerCore.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "Worker.h"
#include "Dispatcher.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/DelCommand.h"
#include "scag/pvss/api/packets/SetCommand.h"
#include "scag/pvss/api/packets/IncCommand.h"
#include "scag/pvss/api/packets/IncModCommand.h"
#include "scag/pvss/api/packets/BatchCommand.h"
#include "scag/pvss/api/packets/BatchResponse.h"
#include "scag/pvss/api/packets/IncResponse.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "scag/pvss/api/packets/DelResponse.h"
#include "scag/pvss/api/packets/SetResponse.h"

using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;

EventMonitor waitObj;
bool stopping = false;
smsc::logger::Logger* logger = 0;

extern "C" void appSignalHandler(int sig)
{
    smsc_log_debug( logger, "Signal %d handled !", sig );
    if (sig==SIGTERM || sig==SIGINT)
    {
        smsc_log_info(logger, "Sending notification to main thread ...");
        MutexGuard mg(waitObj);
        ::stopping = true;
        waitObj.notifyAll();
    }
    else if (sig == SIGHUP)
    {
        smsc_log_info(logger, "Reloading logger config");
        smsc::logger::Logger::Reload();
    }
}

extern "C" void atExitHandler(void)
{
    smsc::logger::Logger::Shutdown();
}

class SyncLogicProcessor : public RequestVisitor, public ProfileCommandVisitor
{
public:
    virtual bool visitPingRequest( PingRequest& req ) throw(PvapException) { return false; }
    virtual bool visitAuthRequest( AuthRequest& req ) throw(PvapException) { return false; }
    virtual bool visitProfileRequest( ProfileRequest& req ) /* throw(PvapException) */  {
        if ( ! req.isValid() ) return false;
        ProfileCommand* cmd = req.getCommand();
        return cmd->visit(*this);
        // if (resp_.get()) resp_->setStatus(Response::OK);
        // return ok;
    }

    virtual bool visitDelCommand( DelCommand& cmd ) /* throw(PvapException) */  {
        resp_.reset( new DelResponse( Response::OK ) );
        return true;
    }
    virtual bool visitSetCommand( SetCommand& cmd ) /* throw(PvapException) */  {
        resp_.reset( new SetResponse(Response::OK) );
        return true;
    }
    virtual bool visitGetCommand( GetCommand& cmd ) /* throw(PvapException) */  {
        std::auto_ptr< GetResponse > r(new GetResponse(Response::OK) );
        r->setVarName(cmd.getVarName());
        r->setStringValue("hello, world");
        r->getProperty().setTimePolicy( INFINIT, time_t(-1), uint32_t(-1) );
        resp_.reset(r.release());
        return true;
    }
    virtual bool visitIncCommand( IncCommand& cmd ) /* throw(PvapException) */  {
        std::auto_ptr< IncResponse > r( new IncResponse(Response::OK) );
        r->setResult(100);
        resp_.reset(r.release());
        return true;
    }
    virtual bool visitIncModCommand( IncModCommand& cmd ) /* throw(PvapException) */  {
        std::auto_ptr< IncResponse > r( new IncResponse(Response::OK) );
        r->setResult(100);
        resp_.reset(r.release());
        return true;
    }
    virtual bool visitBatchCommand( BatchCommand& cmd ) /* throw(PvapException) */  {
        std::auto_ptr<BatchResponse> r( new BatchResponse(Response::OK) );
        for ( std::vector< BatchRequestComponent* >::const_iterator i = cmd.getBatchContent().begin();
              i != cmd.getBatchContent().end();
              ++i ) {
            BatchRequestComponent* comp = *i;
            if ( comp->visit(*this) ) r->addComponent( static_cast<BatchResponseComponent*>(getResponse()) );
        }
        resp_.reset(r.release());
        return true;
    }

    CommandResponse* getResponse() { return resp_.release(); }

private:
    std::auto_ptr<CommandResponse> resp_;
};


class SyncLogicImpl : public Server::SyncLogic
{
public:
    SyncLogicImpl() : 
    log_(smsc::logger::Logger::getInstance("synclogic"))
    {}
    virtual Response* process( Request& req ) /* throw (PvssException) */  {
        CommandResponse* result = ( req.visit(proc_) ? proc_.getResponse() : 0 );
        smsc_log_debug(log_,"request %s processed: %s", req.toString().c_str(),
                       result ? result->toString().c_str() : "" );
        return new ProfileResponse(req.getSeqNum(),result);
    }
    virtual void responseSent( std::auto_ptr<ServerContext>) {}
    virtual void responseFail( std::auto_ptr<ServerContext>) {}
private:
    smsc::logger::Logger* log_;
    SyncLogicProcessor    proc_;
};

class SyncDispatcherImpl : public SyncDispatcher
{
public:
    virtual unsigned getIndex( Request& req ) const {
        return 0;
    }
    virtual Server::SyncLogic* getSyncLogic(unsigned idx) {
        if ( idx == 0 ) return &logic_;
        return 0;
    }
    std::string reportStatistics() const {
        return "no stats right now";
    }
private:
    SyncLogicImpl logic_;
};


int main()
{
    smsc::logger::Logger::Init();
    logger = smsc::logger::Logger::getInstance("main");
    atexit( atExitHandler );

    // --- signals
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGTERM);
    sigdelset(&set, SIGINT);
    sigdelset(&set, SIGSEGV);
    sigdelset(&set, SIGBUS);
    sigdelset(&set, SIGHUP);
    sigprocmask(SIG_SETMASK, &set, NULL);
    sigset(SIGTERM, appSignalHandler);
    sigset(SIGINT, appSignalHandler);
    sigset(SIGHUP, appSignalHandler);   

    // --- config
    ServerConfig serverConfig;
    try {
        serverConfig.setHost("0.0.0.0");
        serverConfig.setPort((short)27881);
        // serverConfig.setConnectionsCount(1);
        serverConfig.setReadersCount(2);
        serverConfig.setWritersCount(2);
        serverConfig.setMaxReaderChannelsCount(5);
        serverConfig.setMaxWriterChannelsCount(5);
        serverConfig.setChannelQueueSizeLimit(100);
        serverConfig.setConnectTimeout(500);
        serverConfig.setInactivityTime(60000);
        serverConfig.setProcessTimeout(1000);
        serverConfig.setIOTimeout(300);
        serverConfig.setQueueSizeLimit(1000);
    } catch ( ConfigException& e ) {
        smsc_log_error( logger, "exception(%u): %s", __LINE__, e.what() );
        ::abort();
    }

    std::auto_ptr< SyncDispatcher > dispatcher( new SyncDispatcherImpl );
    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Server > server( new ServerCore( serverConfig, *protocol.get() ) );

    // synchronous variant
    try {
        // server->init();
        server->startup(*dispatcher.get());
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception(%u): %s", __LINE__, e.what() );
        stopping = true;
    }
    
    // asynchronous variant
    /*
    try {
        server->startup( Server::AsyncLogic& logic );
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }
     */

    while ( true ) {
        MutexGuard mg(waitObj);
        if ( stopping ) break;
        waitObj.wait();
    }

    smsc_log_info(logger,"going to shutdown the server");
    server->shutdown();
    return 0;
}
