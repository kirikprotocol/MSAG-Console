#include <string>

#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#include "HttpCommand.h"
#include "HttpProcessor.h"
#include "HttpRouter.h"

#include "XMLHandlers.h"

namespace scag { namespace transport { namespace http {

XERCES_CPP_NAMESPACE_USE

void HttpRouterImpl::init(std::string& cfg)
{
    route_cfg_file = cfg + "/http_routes.xml";
    ReloadRoutes();
    routes = NULL;
}

HttpRoute HttpRouterImpl::findRoute(const std::string& addr, const std::string& URL)
{
    MutexGuard mt(GetRouteMutex);

    AddressURLKey k(addr.c_str(), URL.c_str());
    uint8_t len;
    do{
        try{
            HttpRoute *r = AddressURLMap->Get(k);
            return *r;
        }
        catch(HashInvalidKeyException &e)
        {
            len = k.mask.cut();
            throw RouteNotFoundException();
        }
    }while(len > 1);
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
    HttpRoute *rt;
    for(int i = 0; i < r->Count(); i++)
    {
        rt = &(*r)[i];
        rid->Insert(rt->id.c_str(), rt);

        for(int j = 0; j < rt->masks.Count(); j++)
        {
            AddressURLKey auk(rt->masks[j].c_str(), rt->url.c_str());
            auh->Insert(auk, rt);
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
        smsc_log_info(logger, "Routes reloading was not successful");
        delete r;
        delete h;
        delete auh;
        throw e;
    }

    smsc_log_info(logger, "Routes reloading finished");
}

void HttpRouterImpl::ParseFile(const char* _xmlFile, HandlerBase* handler)
{
    int errorCount = 0;

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
        errorCount = parser.getErrorCount();
    }
    catch (const OutOfMemoryException&)
    {
        smsc_log_error(logger,"Terminate parsing: XMLPlatform: OutOfMemoryException");
        throw Exception("XMLPlatform: OutOfMemoryException");
    }
    catch (const XMLException& toCatch)
    {
        StrX msg(toCatch.getMessage());

        smsc_log_error(logger,"Terminate parsing: XMLException: %s", msg.localForm());
        throw Exception("XMLException: %s", msg.localForm());
    }
    catch (Exception& e)
    {
        smsc_log_error(logger,"Terminate parsing: %s",e.what());
        throw e;
    }
    catch (...)
    {
        smsc_log_error(logger,"Terminate parsing: unknown fatal error");
        throw Exception("unknown fatal error");
    }

    if (errorCount > 0) 
        smsc_log_error(logger,"Error parsing: some errors occured");
}
    
}}}
