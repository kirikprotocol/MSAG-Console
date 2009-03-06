#include "ServerCore.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "Worker.h"
#include "Dispatcher.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"

using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;

class SyncLogicImpl : public Server::SyncLogic
{
public:
    SyncLogicImpl() {}
    virtual Response* process( Request& req ) throw (PvssException) {
        return 0;
    }
    virtual void responseSent( std::auto_ptr<ServerContext>) {}
    virtual void responseFail( std::auto_ptr<ServerContext>) {}
};

class SyncDispatcherImpl : public SyncDispatcher
{
public:
    virtual unsigned getIndex( Request& req ) {
        return 0;
    }
    virtual Server::SyncLogic* getSyncLogic(unsigned idx) {
        if ( idx == 0 ) return &logic_;
        return 0;
    }
private:
    SyncLogicImpl logic_;
};


int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");

    ServerConfig serverConfig;
    try {
        serverConfig.setHost("127.0.0.1");
        serverConfig.setPort((short)1234);
        // serverConfig.setConnectionsCount(1);
        serverConfig.setReadersCount(1);
        serverConfig.setWritersCount(1);
        serverConfig.setMaxReaderChannelsCount(2);
        serverConfig.setMaxWriterChannelsCount(2);
        serverConfig.setChannelQueueSizeLimit(100);
        serverConfig.setConnectTimeout(1000);
        serverConfig.setInactivityTime(30000);
        serverConfig.setProcessTimeout(1000);
        serverConfig.setIOTimeout(500);
        serverConfig.setQueueSizeLimit(1000);
    } catch ( ConfigException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
        ::abort();
    }

    std::auto_ptr< SyncDispatcher > dispatcher( new SyncDispatcherImpl );
    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Server > server( new ServerCore( serverConfig, *protocol.get() ) );

    // synchronous variant
    try {
        server->startup(*dispatcher.get());
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }
    
    // asynchronous variant
    /*
    try {
        server->startup( Server::AsyncLogic& logic );
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }
     */

    EventMonitor waitObj;
    {
        MutexGuard mg(waitObj);
        waitObj.wait(30000);
    }

    smsc_log_info(logger,"going to shutdown");
    server->shutdown();
    return 0;
}
