#include <unistd.h>
#include "logger/Logger.h"
#include "scag/pvss/api/core/client/ClientConfig.h"
#include "scag/pvss/api/core/client/impl/ClientCore.h"
#include "scag/pvss/api/pvap/PvapProtocol.h"
#include "scag/pvss/api/packets/ProfileRequest.h"
#include "scag/pvss/api/packets/GetCommand.h"
#include "scag/pvss/api/packets/GetResponse.h"
#include "scag/pvss/api/packets/GetProfileCommand.h"
#include "scag/pvss/api/packets/GetProfileResponse.h"

void usage( const char* prog )
{
    std::fprintf(stderr,"Usage: %s HOST PORT [-a|-o|-s|-p] [-n PROPKEY] PROFKEY ...\n",prog);
}

using namespace scag2::pvss;

int main( int argc, char** argv )
{
    smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_WARN );
    // smsc::logger::Logger::Init();
    smsc::logger::Logger* logmain = smsc::logger::Logger::getInstance("main");

    // 1
    if ( argc < 3 ) {
        usage(argv[0]);
        return -1;
    }

    const std::string host = argv[1];
    int port = atoi(argv[2]);

    smsc_log_info(logmain,"connecting to %s:%u",host.c_str(),port);

    core::client::ClientConfig clientConfig;
    clientConfig.setHost(host);
    clientConfig.setPort(port);
    clientConfig.setConnectionsCount(1);
    clientConfig.setMaxReaderChannelsCount(5);
    clientConfig.setMaxWriterChannelsCount(5);
    clientConfig.setReadersCount(2);
    clientConfig.setWritersCount(2);
    clientConfig.setIOTimeout(100);

    std::auto_ptr< core::client::Client > client
        ( new core::client::ClientCore(new core::client::ClientConfig(clientConfig),
                                       new pvap::PvapProtocol ));

    try {
        client->startup();
    } catch ( PvssException& e ) {
        smsc_log_error(logmain,"client startup exception: %s", e.what());
        return 125;
    }

    // waiting for connection
    sleep(1);

    // processing
    ScopeType scope = SCOPE_ABONENT;
    int shellresult = 0;
    std::string propertyName;
    for ( char** p = argv+3; *p != 0; ++p ) {
        if ( strlen(*p) == 0 ) continue;
        if ( (*p)[0] == '-' && strlen(*p) == 2 ) {
            switch ( (*p)[1] ) {
            case 'a' : scope = SCOPE_ABONENT; break;
            case 'o' : scope = SCOPE_OPERATOR; break;
            case 'p' : scope = SCOPE_PROVIDER; break;
            case 's' : scope = SCOPE_SERVICE; break;
            case 'n' : {
                // get property name
                if ( !*++p ) {
                    smsc_log_error(logmain,"property name is not supplied");
                    return 124;
                } else if ( ! strlen(*p) ) {
                    smsc_log_error(logmain,"empty property name");
                    return 124;
                }
                propertyName = *p;
                continue;
            }
            default : {
                smsc_log_error(logmain,"Unknown switch: %s",*p);
                return 123;
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

        const bool hasProperty = !propertyName.empty();

        std::auto_ptr<Request> req;
        std::auto_ptr<Response> resp;
        if (hasProperty) {
            std::auto_ptr<GetCommand> gcmd(new GetCommand());
            gcmd->setVarName(propertyName);
            req.reset(new ProfileRequest(pk,gcmd.release()));
            propertyName = "";
        } else {
            req.reset(new ProfileRequest(pk,new GetProfileCommand()));
        }

        try {
            resp = client->processRequestSync(req);
        } catch ( std::exception& e ) {
            smsc_log_warn(logmain,"exception: %s",e.what());
            shellresult = 127;
            continue;
        }

        if ( ! resp.get() ) {
            smsc_log_warn(logmain,"response is NULL");
            shellresult = 126;
            continue;
        }
        if ( resp->getStatus() != 0 ) {
            smsc_log_warn(logmain,"response has bad status: %u", resp->getStatus() );
            shellresult = resp->getStatus();
            continue;
        }
        
        const ProfileResponse* pr = static_cast<const ProfileResponse*>(resp.get());
        if ( hasProperty ) {
            const GetResponse* gpr = static_cast<const GetResponse*>(pr->getResponse());
            std::printf("================================================\n"
                        "profile %s\n"
                        "%s\n",pk.toString().c_str(),
                        gpr->getProperty().toString().c_str());
        } else {
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
        shellresult = 0;
    }
    // struct timespec ts = { 70, 0 };
    // ::nanosleep( &ts, 0 );

    smsc_log_debug(logmain,"shutting down the pvss client");
    client->shutdown();
    smsc_log_debug(logmain,"client shutdown");
    return shellresult;
}
