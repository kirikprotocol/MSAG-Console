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

bool HttpTraceRouter::getTraceRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port, std::vector<std::string>& trace)
{
    MutexGuard mt(GetRouteMutex);

    char buf[20];
    std::string url;

    if(AddressURLMap == NULL)
    {
        trace.push_back("");
        trace.push_back("No loaded routes");
        return false;
    }

    sprintf(buf, ":%d", port);
    url = site + (port != 80 ? buf : "") + path;

//    Address adr(addr.c_str());
    AddressURLKey k(addr, site, path, port);
    uint8_t len;

    trace.push_back("");
    do{
        try{
            HttpRouteInt *r = AddressURLMap->Get(k);
            trace.push_back("Match found:" + k.mask.toString() + ", url: " + url);
            trace.push_back("Route info:");

            char buf[100];
            std::string s;

            sprintf(buf, ";ServiceId:%d", r->service_id);
            s = "RouteId:" + r->id + buf + ";Enabled:" + (r->enabled ? "yes" : "no");
            trace[0] = s;

            trace.push_back("Masks:");
            for(int i = 0; i < r->masks.Count(); i++)
                trace.push_back(r->masks[i]);

            trace.push_back("Urls:");
            for(int i = 0; i < r->sites.Count(); i++)
            {
                std::string s1;
                sprintf(buf, ":%d", r->sites[i].port);
                s1 = r->sites[i].host + ((r->sites[i].port != 80) ? buf : "");
                for(int j = 0; j < r->sites[i].paths.Count(); j++)
                    trace.push_back(s1 + r->sites[i].paths[j]);
            }

            return true;
        }
        catch(XHashInvalidKeyException &e)
        {
            trace.push_back("Trying mask: " + k.mask.toString() + ", url: " + url + ". No match");
            len = k.mask.cut();
        }
    }while(len > 5);

    trace.push_back("No matches found");
    return false;
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
    defInAddressPlace = NULL;
    defOutAddressPlace = NULL;
    defInUSRPlace = NULL;
    defOutUSRPlace = NULL;
    XMLPlatformUtils::Initialize("en_EN.UTF-8");
}

HttpRouterImpl::~HttpRouterImpl()
{
    delete routes;
    delete routeIdMap;
    delete AddressURLMap;
    delete defInAddressPlace;
    delete defOutAddressPlace;
    delete defInUSRPlace;
    delete defOutUSRPlace;

    XMLPlatformUtils::Terminate();
}

void HttpRouterImpl::init(const std::string& cfg)
{
    route_cfg_file = cfg;
}
                                                                        
HttpRoute HttpRouterImpl::findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port)
{
    MutexGuard mt(GetRouteMutex);

//    Address adr(addr.c_str());
    AddressURLKey k(addr, site, path, port);
    uint8_t len;
    do{
        try{
            HttpRouteInt *r = AddressURLMap->Get(k);
            if(r->enabled)
                return *r;
            else
                len = k.mask.cut();
        }
        catch(XHashInvalidKeyException &e)
        {
            len = k.mask.cut();
        }
    }while(len > 5);

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
    PlacementArray* inap = new PlacementArray;
    PlacementArray* outap = new PlacementArray;
    PlacementArray* inup = new PlacementArray;
    PlacementArray* outup = new PlacementArray;

    try{

        XMLBasicHandler handler(r, inap, outap, inup, outup);
        ParseFile(route_cfg_file.c_str(), &handler);
        BuildMaps(r, h, auh);
        MutexGuard mg(GetRouteMutex);
        delete routes;
        delete routeIdMap;
        delete AddressURLMap;
        delete defInAddressPlace;
        delete defOutAddressPlace;
        delete defInUSRPlace;
        delete defOutUSRPlace;
        routes = r;
        routeIdMap = h;
        AddressURLMap = auh;
        defInAddressPlace = inap;
        defOutAddressPlace = outap;
        defInUSRPlace = inup;
        defOutUSRPlace = outup;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Routes reloading was not successful: %s", e.what());
        delete r;
        delete h;
        delete auh;
        delete inap;
        delete outap;
        delete inup;
        delete outup;
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
