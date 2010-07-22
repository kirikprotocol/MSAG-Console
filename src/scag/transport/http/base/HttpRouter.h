#ifndef SCAG_TRANSPORT_HTTP_BASE_ROUTER
#define SCAG_TRANSPORT_HTTP_BASE_ROUTER

#include <string>
#include <vector>
#include "util/int.h"

namespace scag2 {
namespace transport {
namespace http {

class RouteNotFoundException{};

class HttpTraceRouter
{
public:
    static void setInstance( HttpTraceRouter* inst );
    static HttpTraceRouter& Instance();

    virtual ~HttpTraceRouter();

    virtual bool getTraceRoute( const std::string& addr,
                                const std::string& site,
                                const std::string& path,
                                uint32_t port,
                                std::vector<std::string>& trace ) = 0;
    virtual bool getTraceRouteById( const std::string& addr,
                                    const std::string& path,
                                    uint32_t rid,
                                    uint32_t sid,
                                    std::vector<std::string>& trace ) = 0;

    virtual void ReloadRoutes() = 0;

protected:
    HttpTraceRouter();

private:
    HttpTraceRouter( const HttpTraceRouter& );
    HttpTraceRouter& operator = ( const HttpTraceRouter& );
};

}
}
}

#endif //SCAG_TRANSPORT_HTTP_ROUTER
