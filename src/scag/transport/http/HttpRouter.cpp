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
    serviceIdMap = NULL;
    masksMap = NULL;
    hostsMap = NULL;
    pathsMap = NULL;
    defInPlace = NULL;
    defOutPlace = NULL;
    AddressURLMap = NULL;
    XMLPlatformUtils::Initialize("en_EN.UTF-8");
}

HttpRouterImpl::~HttpRouterImpl()
{
    delete routes;
    delete routeIdMap;
    delete serviceIdMap;
    delete AddressURLMap;
    delete defInPlace;
    delete defOutPlace;

    XMLPlatformUtils::Terminate();
}

void HttpRouterImpl::init(const std::string& cfg)
{
    route_cfg_file = cfg;
}

bool HttpTraceRouter::getTraceRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port, std::vector<std::string>& trace)
{
    MutexGuard mt(GetRouteMutex);
    TmpBuf<char, 512> pt(512);
    std::string s;
    char buf[150], *pathPtr;
    buf[19] = 0;
    uint32_t hid, mid, pid, *p;

    trace.push_back("");

    if(AddressURLMap == NULL)
    {
        trace.push_back("No loaded routes");
        return false;
    }

    s = site + ':' + lltostr(port, buf + 19);

    if(!(p = hostsMap->GetPtr(s.c_str())))
    {
        trace.push_back("Host not found: " + s);
        return false;
    }

    trace.push_back("Host found: " + s);
    hid = *p;

    pt.Append(path.c_str(), path.length() + 1);
    pathPtr = pt.get();
    uint32_t ptLen = pt.GetPos();

    while(ptLen)
    {
        trace.push_back(std::string("Searching path: ") + pathPtr);
        if((p = pathsMap->GetPtr(pathPtr)))
        {
            trace.push_back(std::string("Path found: ") + pathPtr);
            pid = *p;
            uint32_t addrLen = addr.length();
            strcpy(buf, addr.c_str());
            while(addrLen > 0)
            {
                trace.push_back(std::string("Searching address mask: ") + buf);
                if((p = masksMap->GetPtr(buf)))
                {
                    trace.push_back(std::string("Address mask found: ") + buf);
                    AddressURLKey auk(*p, hid, pid);
                    HttpRouteInt **rt, *r;
                    if((rt = AddressURLMap->GetPtr(auk)))
                    {
                        r = *rt;
                        trace.push_back("Route Found: " + r->id);

                        trace.push_back("Route info:");

                        sprintf(buf, "RouteId: %d;ServiceId:%d", r->id, r->service_id);
                        std::string s1 = "RouteName: " + r->name + buf;
                        if(r->enabled) s1 +=";Enabled";
                        if(r->def) s1 += ";Default";
                        if(r->transit) s1 += ";Transit";
                        
                        trace[0] = s1;

                        trace.push_back("Masks:");
                        for(int i = 0; i < r->masks.Count(); i++)
                            trace.push_back(r->masks[i]);

                        trace.push_back("Urls:");
                        buf[19] = 0;
                        for(int i = 0; i < r->sites.Count(); i++)
                        {
                            trace.push_back(r->sites[i].host + ':' + lltostr(r->sites[i].port, buf + 19) + (r->sites[i].def ? ";Default" : ""));
                            for(int j = 0; j < r->sites[i].paths.Count(); j++)
                                trace.push_back(r->sites[i].paths[j]);
                        }

                        return true;
                    }
                    else
                        trace.push_back("No matching route: Host: " + s + "Path: " + pathPtr + "Mask:" + buf);
                }
                buf[--addrLen] = '?';
            }
        }

        if(pathPtr[ptLen - 1] == '/') ptLen--;

        while(ptLen > 0 && pathPtr[ptLen - 1] != '/')
            ptLen--;
        pathPtr[ptLen] = 0;
    }

    trace.push_back("No matches found");
    return false;
}

HttpRoute HttpRouterImpl::findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port)
{
    MutexGuard mt(GetRouteMutex);
    TmpBuf<char, 512> pt(512);
    std::string s;
    char buf[50], *pathPtr;
    buf[19] = 0;
    uint32_t hid, mid, pid, *p;

//    Address adr(addr.c_str());
    s = site + ':' + lltostr(port, buf + 19);

    if(!(p = hostsMap->GetPtr(s.c_str())))
        throw RouteNotFoundException();            

    hid = *p;

    pt.Append(path.c_str(), path.length() + 1);
    pathPtr = pt.get();
    uint32_t ptLen = pt.GetPos();

    while(ptLen)
    {
        if((p = pathsMap->GetPtr(pathPtr)))
        {
            pid = *p;
            uint32_t addrLen = addr.length();
            strcpy(buf, addr.c_str());
            while(addrLen > 0)
            {
                if((p = masksMap->GetPtr(buf)))
                {
                    AddressURLKey auk(*p, hid, pid);
                    HttpRouteInt **rt;
                    if((rt = AddressURLMap->GetPtr(auk)) && (*rt)->enabled)
                        return *(*rt);
                }
                buf[--addrLen] = '?';
            }
        }

        if(pathPtr[ptLen - 1] == '/') ptLen--;

        while(ptLen > 0 && pathPtr[ptLen - 1] != '/')
            ptLen--;
        pathPtr[ptLen] = 0;
    }

    throw RouteNotFoundException();
}

bool HttpRouterImpl::checkRoute(HttpRouteInt* rt, const std::string& addr, const std::string& path)
{
    StringArray* pt = &rt->defSite.paths;
    int i = 0;
    bool found = false;

    while(i < pt->Count() && !found)
    {
        std::string& s = (*pt)[i];
        if(s[s.length()-1] == '/')
        {
            if(!strncmp(path.c_str(), s.c_str(), s.length()))
                found = true;
        }
        else if(!strcmp(path.c_str(), s.c_str()))
            found = true;
        i++;
    }
    if(!found) return false;

    if(!addr.length())
    {
        if(!rt->transit)
            smsc_log_error(logger, "Route found. id=%d. But it is not transit.", rt->id);
        return rt->transit;
    }
    i = 0;
    found = false;
    StringArray* masks = &rt->masks;
    while(i < rt->masks.Count() && !found)
    {
        const char* s = rt->masks[i].c_str(), *p;
        if(rt->masks[i].length() == addr.length())
        {
            if(p = strchr(s, '?'))
            {
                if(!strncmp(s, addr.c_str(), p - s))
                    found = true;
            }
            else if(!strcmp(s, addr.c_str()))
                found = true;
        }
        i++;
    }

    if(!found) return false;

    return true;
}

HttpRoute HttpRouterImpl::findRouteByServiceId(const std::string& addr, uint32_t sid, const std::string& path)
{
    MutexGuard mt(GetRouteMutex);

    HttpRouteInt** rt;

    rt = serviceIdMap->GetPtr(sid);
    if(!rt || !checkRoute(*rt, addr, path)) throw RouteNotFoundException();

    return **rt;
}

HttpRoute HttpRouterImpl::findRouteByRouteId(const std::string& addr, uint32_t rid, const std::string& path)
{
    MutexGuard mt(GetRouteMutex);

    HttpRouteInt** rt;

    rt = routeIdMap->GetPtr(rid);
    if(!rt || !checkRoute(*rt, addr, path)) throw RouteNotFoundException();

    return **rt;
}

HttpRoute HttpRouterImpl::getRoute(uint32_t routeId)
{
    MutexGuard mt(GetRouteMutex);
    try{
        return *routeIdMap->Get(routeId);
    }
    catch(HashInvalidKeyException& e)
    {
        throw RouteNotFoundException();
    }
}

uint32_t HttpRouterImpl::getId(Hash<uint32_t>* h, const std::string& s, uint32_t& id)
{
    uint32_t *p, rid;
    if((p = h->GetPtr(s.c_str())))
        rid = *p;
    else
    {
        h->Insert(s.c_str(), ++id);
        rid = id;
    }
    return rid;
}

void HttpRouterImpl::BuildMaps(RouteArray *r, RouteHash *rid, ServiceIdHash *sid, AddressURLHash *auh, Hash<uint32_t>* mh, Hash<uint32_t>* hh, Hash<uint32_t>* ph)
{
    char buf[20];
    HttpRouteInt *rt;
    uint32_t mid = 0, hid = 0, pid = 0, cmid, chid, cpid, *p;
    std::string s;
    buf[19] = 0;
    for(int i = 0; i < r->Count(); i++)
    {
        rt = &(*r)[i];

        rt->provider_id = BillingManager::Instance().getInfrastructure().GetProviderID(rt->service_id);
        if(rt->provider_id == 0)
            throw Exception("provider id not found for service id=%d", rt->service_id);

        int i = rt->sites.Count() - 1;
        while(i > 0 && !rt->sites[i].def)
            i--;
        rt->defSite = rt->sites[i];

        if(rt->def)
            sid->Insert(rt->service_id, rt);

        rid->Insert(rt->id, rt);

        for(int j = 0; j < rt->masks.Count(); j++)
        {
            cmid = getId(mh, rt->masks[j], mid);
            for(int k = 0; k < rt->sites.Count(); k++)
            {
                s = rt->sites[k].host + ':' + lltostr(rt->sites[k].port, buf + 19);
                chid = getId(hh, s, hid);
                for(int m = 0; m < rt->sites[k].paths.Count(); m++)
                {
                    cpid = getId(ph, rt->sites[k].paths[m], pid);
                    smsc_log_debug(logger, "AddedMapping mask: %s, URL: %s:%d%s", rt->masks[j].c_str(), rt->sites[k].host.c_str(), rt->sites[k].port, rt->sites[k].paths[m].c_str());
                    AddressURLKey auk(cmid, chid, cpid);
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
    ServiceIdHash* s = new ServiceIdHash; 
    Hash<uint32_t>* mh = new Hash<uint32_t>;
    Hash<uint32_t>* ph = new Hash<uint32_t>;
    Hash<uint32_t>* hh = new Hash<uint32_t>;
    AddressURLHash* auh = new AddressURLHash;
    PlacementKindArray *inap = (PlacementKindArray*)new PlacementKindArray;
    PlacementKindArray* outap = (PlacementKindArray*)new PlacementKindArray;

    try{

        XMLBasicHandler handler(r, inap, outap);
        ParseFile(route_cfg_file.c_str(), &handler);
        BuildMaps(r, h, s, auh, mh, hh, ph);
        MutexGuard mg(GetRouteMutex);
        delete routes;
        delete routeIdMap;
        delete serviceIdMap;
        delete masksMap;
        delete pathsMap;
        delete hostsMap;
        delete AddressURLMap;
        delete defInPlace;
        delete defOutPlace;
        routes = r;
        routeIdMap = h;
        serviceIdMap = s;
        masksMap = mh;
        hostsMap = hh;
        pathsMap = ph;
        AddressURLMap = auh;
        defInPlace = inap;
        defOutPlace = outap;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Routes reloading was not successful: %s", e.what());
        delete r;
        delete h;
        delete s;
        delete auh;
        delete mh;
        delete hh;
        delete ph;
        delete inap;
        delete outap;
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
