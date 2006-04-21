#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <scag/bill/BillingManager.h>

#include "XMLHandlers.h"

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"

namespace scag { namespace transport { namespace http {

XERCES_CPP_NAMESPACE_USE

using scag::bill::BillingManager;

HttpRouterImpl::HttpRouterImpl()
{
    logger = smsc::logger::Logger::getInstance("httprouter");
    XMLPlatformUtils::Initialize();
}

HttpRouterImpl::~HttpRouterImpl()
{
    XMLPlatformUtils::Terminate();
}

void HttpRouterImpl::init(const std::string& cfg)
{
    route_cfg_file = cfg;
    routes = NULL;
    routeIdMap = NULL;
    AddressURLMap = NULL;

    ReloadRoutes();
}
                                                                        
HttpRoute HttpRouterImpl::findRoute(const std::string& addr, const std::string& site, const std::string& path, uint32_t port)
{
    MutexGuard mt(GetRouteMutex);

    AddressURLKey k(addr, site, path, port);
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
    }while(len > 1);

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

        rt->provider_id = BillingManager::Instance().getInfrastructure().GetProviderID(rt->service_id);

        rid->Insert(rt->id.c_str(), rt);

        for(int j = 0; j < rt->masks.Count(); j++)
        {
            for(int k = 0; k < rt->sites.Count(); k++)
            {
                for(int m = 0; m < rt->sites[k].paths.Count(); m++)
                {
                    smsc_log_error(logger, "AddedMapping mask: %s, URL: %s:%d%s", rt->masks[j].c_str(), rt->sites[k].host.c_str(), rt->sites[k].port, rt->sites[k].paths[m].c_str());
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
    try{

        XMLBasicHandler handler(r);
        ParseFile(route_cfg_file.c_str(), &handler);
        BuildMaps(r, h, auh);
        MutexGuard mg(GetRouteMutex);
        delete routes;
        delete routeIdMap;
        delete AddressURLMap;
        routes = r;
        routeIdMap = h;
        AddressURLMap = auh;
    }
    catch(Exception& e)
    {
        smsc_log_info(logger, "Routes reloading was not successful: %s", e.what());
        delete r;
        delete h;
        delete auh;
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
