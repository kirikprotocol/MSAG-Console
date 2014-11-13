#ifndef SCAG_TRANSPORT_HTTP_ROUTER
#define SCAG_TRANSPORT_HTTP_ROUTER

#include <string>
#include <vector>

#include <xercesc/sax/HandlerBase.hpp>

#include <sms/sms_const.h>
#include <sms/sms.h>
#include <core/buffers/Array.hpp>
#include <core/buffers/XHash.hpp>
#include <core/buffers/IntHash.hpp>
#include "core/buffers/Hash.hpp"
#include <core/synchronization/Mutex.hpp>
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
using smsc::core::buffers::IntHash;
using smsc::core::buffers::Hash;
using smsc::core::buffers::HashInvalidKeyException;
using smsc::util::crc32;
using smsc::core::synchronization::Mutex;

class RouteNotFoundException{};

class HttpRouter
{
public:
    virtual HttpRoute findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port) = 0;
    virtual HttpRoute findRouteByServiceId(const std::string& addr, uint32_t sid, const std::string& path) = 0;
    virtual HttpRoute findRouteByRouteId(const std::string& addr, uint32_t rid, const std::string& path) = 0;
    virtual HttpRoute getRoute(uint32_t routeId) = 0;

    virtual ~HttpRouter() {}
};

typedef XHash<AddressURLKey, HttpRouteInt*, AddressURLKey> AddressURLHash;
typedef IntHash<HttpRouteInt*> RouteHash;
typedef IntHash<HttpRouteInt*> ServiceIdHash;

class HttpRouterImpl:public HttpRouter
{
protected:
    Mutex ReloadMutex, GetRouteMutex;

    std::string route_cfg_file;

    RouteArray* routes;
    RouteHash* routeIdMap;
    ServiceIdHash* serviceIdMap;
    Hash<uint32_t> *pathsMap, *hostsMap, *masksMap;
    AddressURLHash* AddressURLMap;
    PlacementKindArray *defInPlace;
    PlacementKindArray *defOutPlace;

    smsc::logger::Logger *logger;

    void ParseFile(const char* _xmlFile, HandlerBase* handler);
    void BuildMaps(RouteArray *r, RouteHash *rid, ServiceIdHash *sid, AddressURLHash *auh, Hash<uint32_t>* mh, Hash<uint32_t>* hh, Hash<uint32_t>* ph);
    uint32_t getId(Hash<uint32_t>* h, const std::string& s, uint32_t& id);
    bool checkRoute(HttpRouteInt* rt, const std::string& addr, const std::string& path);
public:
    HttpRouterImpl();

    void init(const std::string& cfg);

    HttpRoute findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port);
    HttpRoute getRoute(uint32_t routeId);
    HttpRoute findRouteByServiceId(const std::string& addr, uint32_t sid, const std::string& path);
    HttpRoute findRouteByRouteId(const std::string& addr, uint32_t rid, const std::string& path);

    void getDefaultInPlacement(PlacementKindArray& r) { for(int i= 0; i< PLACEMENT_KIND_COUNT; i++) r[i] = (*defInPlace)[i]; };
    void getDefaultOutPlacement(PlacementKindArray& r) { for(int i= 0; i< PLACEMENT_KIND_COUNT; i++) r[i] = (*defOutPlace)[i]; };

    void ReloadRoutes();

    ~HttpRouterImpl();
};

class HttpTraceRouter: public HttpRouterImpl
{
protected:
    void init(const std::string& cfg);
    void routeInfo(HttpRouteInt* r, std::vector<std::string>& trace);    
    bool checkTraceRoute(HttpRouteInt* rt, const std::string& addr, const std::string& path, std::vector<std::string>& trace);
public:
    static HttpTraceRouter& Instance();
    static void Init(const std::string& cfg);

    bool getTraceRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port, std::vector<std::string>& trace);
    bool getTraceRouteById(const std::string& addr, const std::string& path, uint32_t rid, uint32_t sid, std::vector<std::string>& trace);
    ~HttpTraceRouter() {}
};


}}}

#endif //SCAG_TRANSPORT_HTTP_ROUTER
