#ifndef SCAG_TRANSPORT_HTTP_ROUTER
#define SCAG_TRANSPORT_HTTP_ROUTER

#include <string>

#include <xercesc/sax/HandlerBase.hpp>

#include <sms/sms_const.h>
#include <sms/sms.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/XHash.hpp>
#include <util/crc32.h>

#include <logger/Logger.h>

#include "HttpCommand.h"
#include "RouterTypes.h"

namespace scag { namespace transport { namespace http {

XERCES_CPP_NAMESPACE_USE

using smsc::sms::MAX_ADDRESS_VALUE_LENGTH;
using smsc::sms::Address;
using smsc::core::buffers::Array;
using smsc::core::buffers::XHash;
using smsc::util::crc32;

class RouteNotFoundException{};

class HttpRouter
{
public:
    virtual HttpRoute findRoute(const std::string& addr, const std::string& URL) = 0;
    virtual HttpRoute getRoute(const std::string& routeId) = 0;

protected:
    virtual ~HttpRouter() {}
};

typedef XHash<AddressURLKey, HttpRoute*, AddressURLKey> AddressURLHash;
typedef Hash<HttpRoute*> RouteHash;

class HttpRouterImpl:public HttpRoute
{
    Mutex ReloadMutex, GetRouteMutex;

    std::string route_cfg_file;

    RouteArray* routes;
    RouteHash* routeIdMap;
    AddressURLHash* AddressURLMap;

    smsc::logger::Logger *logger;

    void ParseFile(const char* _xmlFile, HandlerBase* handler);
    void BuildMaps(RouteArray*, RouteHash*, AddressURLHash*);
public:
    HttpRouterImpl();

    void init(const std::string& cfg);

    HttpRoute findRoute(const std::string& addr, const std::string& URL);
    HttpRoute getRoute(const std::string& routeId);

    void ReloadRoutes();

    ~HttpRouterImpl();
};

}}}

#endif //SCAG_TRANSPORT_HTTP_ROUTER
