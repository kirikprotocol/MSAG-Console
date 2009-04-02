#include <memory>
#include "ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/packets/DelCommand.h"

using namespace scag2::pvss;
using namespace scag2::pvss::core;
using namespace scag2::pvss::core::client;

class SimpleResponseHandler : public Client::ResponseHandler
{
public:
    SimpleResponseHandler() : logger(smsc::logger::Logger::getInstance("resp.handl")) {}

    virtual void handleResponse( std::auto_ptr<Request> request,
                                 std::auto_ptr<Response> response ) {
        smsc_log_info( logger, "Async Request: '%s' Response: '%s'",
                       request->toString().c_str(), response->toString().c_str() );
    }

    virtual void handleError( const PvssException& exc, std::auto_ptr<Request> request) {
        smsc_log_warn( logger, "Async Request error: '%s' during processing Request: '%s'",
                       exc.what(), request->toString().c_str() );
    }

private:
    smsc::logger::Logger* logger;
};

int main()
{
    smsc::logger::Logger::Init();
    smsc::logger::Logger* logger = smsc::logger::Logger::getInstance("main");

    ClientConfig clientConfig;
    try {
        clientConfig.setHost("127.0.0.1");
        clientConfig.setPort((short)1234);
        clientConfig.setConnectionsCount(1);
        clientConfig.setReadersCount(1);
        clientConfig.setWritersCount(1);
        clientConfig.setMaxReaderChannelsCount(2);
        clientConfig.setMaxWriterChannelsCount(2);
        clientConfig.setChannelQueueSizeLimit(100);
        clientConfig.setConnectTimeout(1000);
        clientConfig.setInactivityTime(15000);
        clientConfig.setProcessTimeout(1000);
        clientConfig.setIOTimeout(500);
    } catch ( ConfigException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
        ::abort();
    }

    std::auto_ptr< Protocol > protocol( new scag2::pvss::pvap::PvapProtocol );
    std::auto_ptr< Client > client( new ClientCore( clientConfig, *protocol.get() ) );

    try {
        client->startup();
    } catch ( PvssException& e ) {
        smsc_log_error( logger, "exception: %s", e.what() );
    }
    
    EventMonitor waitObj;
    {
        MutexGuard mg(waitObj);
        waitObj.wait(100);
    }

    for ( int i = 0; i < 5; ++i ) {

        smsc_log_debug(logger,"pass #%d", i);

        std::auto_ptr< Request > req;
        {
            DelCommand* cmd = new DelCommand;
            cmd->setVarName("aaa");
            ProfileRequest* r = new ProfileRequest( cmd );
            req.reset(r);
            r->getProfileKey().setAbonentKey(".0.1.79137654079");
        }
        std::auto_ptr< Request > req2( req->clone() );

        try {
            std::auto_ptr< Response > resp = client->processRequestSync( req );
            smsc_log_info( logger, "Sync Response: %s", resp->toString().c_str() );
        } catch ( PvssException& e ) {
            smsc_log_error( logger, "Sync Error: %s, req: %s", e.what(), req->toString().c_str() );
        }

        req = req2;

        SimpleResponseHandler myResponseHandler;
        try {
            client->processRequestAsync( req, myResponseHandler );
        } catch (PvssException& e) {
            smsc_log_error( logger, "Async Error: %s, req: %s", e.what(), req->toString().c_str() );
        }

        {
            MutexGuard mg(waitObj);
            waitObj.wait(2000);
        }
    }

    {
        MutexGuard mg(waitObj);
        waitObj.wait(30000);
    }
    
    smsc_log_info(logger,"going to shutdown");
    client->shutdown();
    return 0;
}
