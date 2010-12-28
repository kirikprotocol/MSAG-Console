#include "route_manager.h"
#include "load_routes.h"
#include "scag/config/impl/ConfigManager2.h"

using namespace scag::transport::smpp::router;
using namespace scag2::config;

inline void checkarg( char** p ) {
    if ( ! *p ) {
        throw std::runtime_error("An argument is required");
    }
}

int main( int argc, char** argv )
{
    smsc::logger::Logger::initForTest(smsc::logger::Logger::LEVEL_DEBUG);

    const char* from = 0;
    smsc::sms::Address src, dst;
    bool dump = false;

    for ( char** p = argv+1; *p != 0; ++p ) {
        const std::string arg(*p);

        if ( arg == "-h" || arg == "--help" ) {
            continue;
        }

        if ( arg == "-s" || arg == "--src" ) {
            checkarg( ++p );
            src = smsc::sms::Address(*p);
            continue;
        }

        if ( arg == "-d" || arg == "--dst" ) {
            checkarg( ++p );
            dst = smsc::sms::Address(*p);
            continue;
        }

        if ( arg == "-f" || arg == "--from" ) {
            checkarg( ++p );
            from = *p;
            continue;
        }

        if ( arg == "--dump" ) {
            dump = true;
            break;
        }
    }

    // loading rm
    std::vector< std::string > traceit;
    {
        // will be cleaned up by singleton
        ConfigManagerImpl* cmi = new ConfigManagerImpl;
        cmi->Init();
    }
    RouteManager rm;
    RouteConfig& rc = ConfigManager::Instance().getRouteConfig();
    loadRoutes( &rm, rc, dump ? &traceit : 0 );

    if ( dump ) {
        printf("------------------------------------------------------\n");
        printf("the routes are loaded, trace follows:\n");
        for ( std::vector< std::string >::const_iterator i = traceit.begin();
              i != traceit.end();
              ++i ) {
            printf(" %s\n",i->c_str());
        }
        traceit.clear();
        rm.dumpInto(traceit);
        printf("------------------------------------------------------\n");
        printf("the dump follows:\n");
        for ( std::vector< std::string >::const_iterator i = traceit.begin();
              i != traceit.end();
              ++i ) {
            printf(" %s\n",i->c_str());
        }
        printf("------------------------------------------------------\n");
        return 0;
    }

    RouteInfo ri;
    const bool res = rm.lookup( from, src, dst, ri, &traceit );
    printf("------------------------------------------------------\n");
    printf("the route is %sfound, trace follows:\n",res ? "" : "NOT ");
    for ( std::vector< std::string >::const_iterator i = traceit.begin();
          i != traceit.end();
          ++i ) {
        printf(" %s\n",i->c_str());
    }
    printf("------------------------------------------------------\n");
    return 0;
}
