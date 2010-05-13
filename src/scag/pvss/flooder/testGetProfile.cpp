#include <unistd.h>
#include "logger/Logger.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "scag/pvss/api/core/client/impl/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/GetProfileCommand.h"
#include "scag/pvss/api/packets/GetProfileResponse.h"

void usage( const char* prog )
{
    std::fprintf(stderr,"Usage: %s HOST PORT [-a|-o|-s|-p|PROFKEY ...]\n",prog);
}

using namespace scag2::pvss;

int main( int argc, char** argv )
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_WARN );
    smsc::logger::Logger* logmain = smsc::logger::Logger::getInstance("main");

    // 1
    if ( argc < 3 ) {
        usage(argv[0]);
        std::terminate();
    }

    const std::string host = argv[1];
    int port = atoi(argv[2]);

    smsc_log_info(logmain,"connecting to %s:%u",host.c_str(),port);

    core::client::ClientConfig clientConfig;
    clientConfig.setHost(host);
    clientConfig.setPort(port);
    clientConfig.setMaxReaderChannelsCount(5);
    clientConfig.setMaxWriterChannelsCount(5);
    clientConfig.setReadersCount(2);
    clientConfig.setWritersCount(2);

    std::auto_ptr< core::client::Client > client
        ( new core::client::ClientCore(new core::client::ClientConfig(clientConfig),
                                       new pvap::PvapProtocol ));

    try {
        client->startup();
    } catch ( PvssException& e ) {
        smsc_log_error(logmain,"client startup exception: %s", e.what());
        std::terminate();
    }

    // waiting for connection
    sleep(1);

    // processing
    ScopeType scope = SCOPE_ABONENT;
    for ( char** p = argv+3; *p != 0; ++p ) {
        if ( strlen(*p) == 0 ) continue;
        if ( (*p)[0] == '-' && strlen(*p) == 2 ) {
            switch ( (*p)[1] ) {
            case 'a' : scope = SCOPE_ABONENT; break;
            case 'o' : scope = SCOPE_OPERATOR; break;
            case 'p' : scope = SCOPE_PROVIDER; break;
            case 's' : scope = SCOPE_SERVICE; break;
            default : {
                smsc_log_warn(logmain,"Unknown switch: %s",*p);
            }
            } // switch
            continue;
        }

        ProfileKey pk;
        if (scope == SCOPE_ABONENT) {
            pk.setAbonentKey(*p);
        } else {
            int32_t intk = atoi(*p);
            switch (scope) {
            case SCOPE_OPERATOR : pk.setOperatorKey(intk); break;
            case SCOPE_PROVIDER : pk.setProviderKey(intk); break;
            case SCOPE_SERVICE  : pk.setServiceKey(intk); break;
            default: {
                smsc_log_warn(logmain,"Unknown scope: %u",scope);
                continue;
            }
            }
        }
        std::auto_ptr<Request> req(new ProfileRequest(pk,new GetProfileCommand()));
        std::auto_ptr<Response> resp;
        try {
            resp = client->processRequestSync(req);
        } catch ( std::exception& e ) {
            smsc_log_warn(logmain,"exception: %s",e.what());
            continue;
        }
        if ( ! resp.get() ) {
            smsc_log_warn(logmain,"response is NULL");
            continue;
        }
        if ( resp->getStatus() != 0 ) {
            smsc_log_warn(logmain,"response has bad status: %u", resp->getStatus() );
            continue;
        }
        
        const ProfileResponse* pr = static_cast<const ProfileResponse*>(resp.get());
        const GetProfileResponse* gpr = static_cast<const GetProfileResponse*>(pr->getResponse());
        std::printf("================================================\n"
                    "profile %s is fetched, %u properties\n",
                    pk.toString().c_str(),unsigned(gpr->getContent().size()));
        for ( std::vector<GetProfileResponseComponent*>::const_iterator i = gpr->getContent().begin();
              i != gpr->getContent().end();
              ++i ) {
            std::printf("%s\n",(*i)->getVarName().c_str());
        }
    }
    client->shutdown();
    return 0;
}
