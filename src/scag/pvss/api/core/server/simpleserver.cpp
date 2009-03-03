#include "ServerCore.h"
#include "ServerContext.h"
#include "ServerConfig.h"
#include "Worker.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"

using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::server;

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

    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Server > server( new ServerCore( serverConfig, *protocol.get() ) );

    try {
        server->startup();
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }
    
    EventMonitor waitObj;
    {
        MutexGuard mg(waitObj);
        waitObj.wait(100000);
    }

    smsc_log_info(logger,"going to shutdown");
    server->shutdown();
    return 0;
}
