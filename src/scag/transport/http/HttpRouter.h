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
    virtual HttpRoute findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port) = 0;
    virtual HttpRoute getRoute(const std::string& routeId) = 0;

    virtual ~HttpRouter() {}
};

typedef XHash<AddressURLKey, HttpRouteInt*, AddressURLKey> AddressURLHash;
typedef Hash<HttpRouteInt*> RouteHash;

class HttpRouterImpl:public HttpRouter
{
protected:
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

    HttpRoute findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port);
    HttpRoute getRoute(const std::string& routeId);

    void ReloadRoutes();

    ~HttpRouterImpl();
};

class HttpTraceRouter: public HttpRouterImpl
{
protected:
    void init(const std::string& cfg);
public:
    static HttpTraceRouter& Instance();
    static void Init(const std::string& cfg);

    std::string getTraceRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port);
    ~HttpTraceRouter() {}
};


}}}

#endif //SCAG_TRANSPORT_HTTP_ROUTER
