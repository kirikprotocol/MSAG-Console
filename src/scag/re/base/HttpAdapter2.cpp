#include "HttpAdapter2.h"
#include "EventHandlerType.h"

namespace scag2 {
namespace re {
namespace http {

using namespace util::properties;
using namespace transport::http;

Hash<AccessType> HttpCommandAdapter::RequestFieldsAccess = HttpCommandAdapter::InitRequestAccess();
Hash<AccessType> HttpCommandAdapter::ResponseFieldsAccess = HttpCommandAdapter::InitResponseAccess();
Hash<AccessType> HttpCommandAdapter::DeliveryFieldsAccess = HttpCommandAdapter::InitDeliveryAccess();

Hash<AccessType> HttpCommandAdapter::InitRequestAccess()
{
    Hash<AccessType> hs;

    hs.Insert("abonent", atRead);
    // hs.Insert("usr", atRead);
    hs.Insert("site", atReadWrite);
    hs.Insert("path", atReadWrite);
    hs.Insert("filename", atReadWrite);
    hs.Insert("port", atReadWrite);
    hs.Insert("message", atRead);

    return hs;
}

Hash<AccessType> HttpCommandAdapter::InitResponseAccess()
{
    Hash<AccessType> hs;

    hs.Insert("abonent", atRead);
    // hs.Insert("usr", atRead);
    hs.Insert("status", atReadWrite);
    hs.Insert("message", atReadWrite);

    return hs;
}

Hash<AccessType> HttpCommandAdapter::InitDeliveryAccess()
{
    Hash<AccessType> hs;

    hs.Insert("abonent", atRead);
    // hs.Insert("usr", atRead);
    hs.Insert("status", atRead);
    hs.Insert("delivered", atRead);

    return hs;
}

AccessType HttpCommandAdapter::CheckAccess(int handlerType,const std::string& name)
{
    AccessType *a = NULL;

    if(!strncmp(name.c_str(), "header-", 7))
        return handlerType == EH_HTTP_DELIVERY ? atRead : atReadWrite;
        
    if(handlerType == EH_HTTP_REQUEST)
    {
        if(!strncmp(name.c_str(), "param-", 6))
            return atReadWrite;

        a = RequestFieldsAccess.GetPtr(name.c_str());
    }
    else if(handlerType == EH_HTTP_RESPONSE)
        a = ResponseFieldsAccess.GetPtr(name.c_str());
    else if(handlerType == EH_HTTP_DELIVERY)
        a = DeliveryFieldsAccess.GetPtr(name.c_str());

    if(a) return *a;

    return atNoAccess;
}

HttpCommandAdapter::~HttpCommandAdapter()
{
    char *key;
    AdapterProperty* value1 = 0;
    
    PropertyPool.First();

    Hash<AdapterProperty*>::Iterator it = PropertyPool.getIterator();

    while(it.Next(key, value1))
        delete value1;
}

Property* HttpCommandAdapter::getRequestProperty(const std::string& name)
{
    AdapterProperty **p = PropertyPool.GetPtr(name.c_str());
    if(p) return *p;

    AdapterProperty* prop = NULL;

    HttpRequest& cmd = (HttpRequest&)command;

    if(!strncmp(name.c_str(), "header-", 7))
        prop = new AdapterProperty(name.c_str(), this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strncmp(name.c_str(), "param-", 6))
        prop = new AdapterProperty(name.c_str(), this, cmd.getQueryParameter(name.c_str() + 6));
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getAddress());
    else if(!strcmp(name.c_str(), "site"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getSite());
    else if(!strcmp(name.c_str(), "path"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getSitePath());
    else if(!strcmp(name.c_str(), "filename"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getSiteFileName());
    else if(!strcmp(name.c_str(), "port"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getSitePort());
    else if(!strcmp(name.c_str(), "message"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getMessageText());
    // else if(!strcmp(name.c_str(), "usr"))
    // prop = new AdapterProperty(name.c_str(), this, cmd.getUSR());

    if(prop)
        PropertyPool.Insert(name.c_str(), prop);
        
    return prop;
}

Property* HttpCommandAdapter::getResponseProperty(const std::string& name)
{
    AdapterProperty **p = PropertyPool.GetPtr(name.c_str());
    if(p) return *p;

    AdapterProperty* prop = NULL;

    HttpResponse& cmd = (HttpResponse&)command;

    if(!strncmp(name.c_str(), "header-", 7))
        prop = new AdapterProperty(name.c_str(), this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strcmp(name.c_str(), "status"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getStatus());
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getAddress());
    // else if(!strcmp(name.c_str(), "usr"))
    // prop = new AdapterProperty(name.c_str(), this, cmd.getUSR());
    else if(!strcmp(name.c_str(), "message"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getMessageText());

    if(prop)
        PropertyPool.Insert(name.c_str(), prop);
        
    return prop;
}

Property* HttpCommandAdapter::getDeliveryProperty(const std::string& name)
{
    AdapterProperty **p = PropertyPool.GetPtr(name.c_str());
    if(p) return *p;

    AdapterProperty* prop = NULL;

    HttpResponse& cmd = (HttpResponse&)command;

    if(!strncmp(name.c_str(), "header-", 7))
        prop = new AdapterProperty(name.c_str(), this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strcmp(name.c_str(), "status"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getStatus());
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getAddress());
    // else if(!strcmp(name.c_str(), "usr"))
    // prop = new AdapterProperty(name.c_str(), this, cmd.getUSR());
    else if(!strcmp(name.c_str(), "delivered"))
        prop = new AdapterProperty(name.c_str(), this, cmd.getDelivered());

    if(prop)
        PropertyPool.Insert(name.c_str(), prop);
        
    return prop;
}

Property* HttpCommandAdapter::getProperty(const std::string& name)
{
    if(command.getCommandId() == HTTP_REQUEST)
        return getRequestProperty(name);
    else if(command.getCommandId() == HTTP_RESPONSE)
        return getResponseProperty(name);
    else if(command.getCommandId() == HTTP_DELIVERY)
        return getDeliveryProperty(name);

    return 0;
}


void HttpCommandAdapter::delProperty( const std::string& name )
{
    if ( !strncmp(name.c_str(),"header-",7) ) {
        command.removeHeaderField(name.c_str()+7);
    } else if ( !strncmp(name.c_str(),"param-",6) &&
                command.getCommandId() == HTTP_REQUEST ) {
        HttpRequest& cmd = static_cast<HttpRequest&>(command);
        cmd.delQueryParameter(name.c_str()+6);
    } else {
        smsc::logger::Logger* logr = smsc::logger::Logger::getInstance("http.adapt");
        smsc_log_warn(logr,"delProperty(%s): not allowed to del");
    }
}


void HttpCommandAdapter::changed(AdapterProperty& property)
{
    if(command.getCommandId() == HTTP_RESPONSE)
    {
        HttpResponse& cmd = (HttpResponse&)command;

        if(!strncmp(property.getName().c_str(), "header-", 7))
            cmd.setHeaderField(property.getName().c_str() + 7, property.getStr().c_str());
        else if(!strcmp(property.getName().c_str(), "status"))
            cmd.setStatus(property.getInt());
/*        else if(!strcmp(property.getName().c_str(), "abonent"))
            cmd.setAbonent(property.getStr());
        else if(!strcmp(property.getName().c_str(), "usr"))
            cmd.setUSR(property.getInt());*/
        else if(!strcmp(property.getName().c_str(), "message"))
            cmd.setMessageText(property.getStr().c_str());
    }
    else if(command.getCommandId() == HTTP_REQUEST)
    {
        HttpRequest& cmd = (HttpRequest&)command;

        if(!strncmp(property.getName().c_str(), "header-", 7))
            cmd.setHeaderField(property.getName().c_str() + 7, property.getStr().c_str());
        else if(!strncmp(property.getName().c_str(), "param-", 6))
            cmd.setQueryParameter(property.getName().c_str() + 6, property.getStr().c_str());
/*        else if(!strcmp(property.getName().c_str(), "abonent"))
            cmd.setAbonent(property.getStr());*/
        else if(!strcmp(property.getName().c_str(), "site"))
            cmd.setSite(property.getStr().c_str());
        else if(!strcmp(property.getName().c_str(), "path"))
            cmd.setSitePath(property.getStr().c_str());
        else if(!strcmp(property.getName().c_str(), "filename"))
            cmd.setSiteFileName(property.getStr().c_str());
        else if(!strcmp(property.getName().c_str(), "port"))
            cmd.setSitePort(property.getInt());
/*        else if(!strcmp(property.getName().c_str(), "message"))
            cmd.setMessageText(property.getStr());
        else if(!strcmp(property.getName().c_str(), "usr"))
            cmd.setUSR(property.getInt());*/
    }
}


}}}
