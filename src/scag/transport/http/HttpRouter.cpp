#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <scag/bill/BillingManager.h>
#include <scag/util/singleton/Singleton.h>

#include "XMLHandlers.h"

#include "HttpCommand.h"
#include "HttpRouter.h"

namespace scag { namespace transport { namespace http {

XERCES_CPP_NAMESPACE_USE

using scag::bill::BillingManager;

using namespace scag::util::singleton;

static bool  inited = false;
static Mutex initLock;

inline unsigned GetLongevity(HttpTraceRouter*) { return 5; }
typedef SingletonHolder<HttpTraceRouter> SingleHP;

HttpTraceRouter& HttpTraceRouter::Instance()
{
    if (!inited) 
    {
        MutexGuard guard(initLock);
        if (!inited) 
            throw std::runtime_error("HttpTraceRouter not inited!");
    }
    return SingleHP::Instance();
}

void HttpTraceRouter::Init(const std::string& cfg)
{
    if (!inited)
    {
        MutexGuard guard(initLock);
        if(!inited) {
            HttpTraceRouter& hp = SingleHP::Instance();
            hp.init(cfg);
            inited = true;
        }
    }
}

std::string HttpTraceRouter::getTraceRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port)
{
    MutexGuard mt(GetRouteMutex);

    char buf[20];
    std::string url;
    std::string trace;

    if(AddressURLMap == NULL)
        return "No loaded routes";

    sprintf(buf, ":%d", port);
    url = site + (port != 80 ? buf : "") + path;

    Address adr(addr.c_str());
    AddressURLKey k(adr.toString(), site, path, port);
    uint8_t len;
    do{
        try{
            HttpRouteInt *r = AddressURLMap->Get(k);
            trace += "Match found:" + k.mask.toString() + ", url: " + url + "\n";
            trace += "Route info:\n";
            trace += r->toString();
            return trace;
        }
        catch(XHashInvalidKeyException &e)
        {
            trace += "No match: mask: " + k.mask.toString() + ", url: " + url + "\n";
            len = k.mask.cut();
        }
    }while(len > 3);

    trace += "No matches found.";
    return trace;
}

void HttpTraceRouter::init(const std::string& cfg)
{
    route_cfg_file = cfg;
}


//-----------------------------------------------------------------------------
HttpRouterImpl::HttpRouterImpl()
{
    logger = smsc::logger::Logger::getInstance("httprouter");
    routes = NULL;
    routeIdMap = NULL;
    AddressURLMap = NULL;
    defAddressPlace = NULL;
    XMLPlatformUtils::Initialize("en_EN.UTF-8");
}

HttpRouterImpl::~HttpRouterImpl()
{
    delete routes;
    delete routeIdMap;
    delete AddressURLMap;
    delete defAddressPlace;

    XMLPlatformUtils::Terminate();
}

void HttpRouterImpl::init(const std::string& cfg)
{
    route_cfg_file = cfg;
    ReloadRoutes();
}
                                                                        
PlacementArray HttpRouterImpl::getDefaultAddressPlacement()
{
    return *defAddressPlace;
}

HttpRoute HttpRouterImpl::findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port)
{
    MutexGuard mt(GetRouteMutex);

    Address adr(addr.c_str());
    AddressURLKey k(adr.toString(), site, path, port);
    uint8_t len;
    do{
        try{
            HttpRouteInt *r = AddressURLMap->Get(k);
            return *r;
        }
        catch(XHashInvalidKeyException &e)
        {
            len = k.mask.cut();
        }
    }while(len > 3);

    throw RouteNotFoundException();
}

HttpRoute HttpRouterImpl::getRoute(const std::string& routeId)
{
    MutexGuard mt(GetRouteMutex);
    try{
        return *routeIdMap->Get(routeId.c_str());
    }
    catch(HashInvalidKeyException& e)
    {
        throw RouteNotFoundException();
    }
}

void HttpRouterImpl::BuildMaps(RouteArray *r, RouteHash *rid, AddressURLHash *auh)
{
    HttpRouteInt *rt;
    for(int i = 0; i < r->Count(); i++)
    {
        rt = &(*r)[i];

//        rt->provider_id = BillingManager::Instance().getInfrastructure().GetProviderID(rt->service_id);

        rid->Insert(rt->id.c_str(), rt);

        for(int j = 0; j < rt->masks.Count(); j++)
        {
            for(int k = 0; k < rt->sites.Count(); k++)
            {
                for(int m = 0; m < rt->sites[k].paths.Count(); m++)
                {
                    smsc_log_debug(logger, "AddedMapping mask: %s, URL: %s:%d%s", rt->masks[j].c_str(), rt->sites[k].host.c_str(), rt->sites[k].port, rt->sites[k].paths[m].c_str());
                    AddressURLKey auk(rt->masks[j], rt->sites[k].host, rt->sites[k].paths[m], rt->sites[k].port);
                    auh->Insert(auk, rt);
                }
            }
        }
    }
}

void HttpRouterImpl::ReloadRoutes()
{
    MutexGuard mt(ReloadMutex);

    smsc_log_info(logger, "Routes reloading started");

    RouteArray* r = new RouteArray;
    RouteHash* h = new RouteHash;
    AddressURLHash* auh = new AddressURLHash;
    PlacementArray* ap = new PlacementArray;

    try{

        XMLBasicHandler handler(r, ap);
        ParseFile(route_cfg_file.c_str(), &handler);
        BuildMaps(r, h, auh);
        MutexGuard mg(GetRouteMutex);
        delete routes;
        delete routeIdMap;
        delete AddressURLMap;
        delete defAddressPlace;
        routes = r;
        routeIdMap = h;
        AddressURLMap = auh;
        defAddressPlace = ap;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Routes reloading was not successful: %s", e.what());
        delete r;
        delete h;
        delete auh;
        delete ap;
        throw e; 
    }

    smsc_log_info(logger, "Routes reloading finished");
}

void HttpRouterImpl::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
    SAXParser parser;
    
    try
    {
        parser.setValidationScheme(SAXParser::Val_Always);
        parser.setDoSchema(true);
        parser.setValidationSchemaFullChecking(true);
        parser.setDoNamespaces(true);

        parser.setValidateAnnotations(false);

        parser.setValidationConstraintFatal(true);

        parser.setDocumentHandler(handler);
        parser.setErrorHandler(handler);
        parser.parse(_xmlFile);
    }
    catch (const OutOfMemoryException&)
    {
        throw Exception("XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        throw Exception("XMLException: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        throw e;
    }
    catch (...)
    {
        throw Exception("unknown fatal error");
    }
}
    
}}}
