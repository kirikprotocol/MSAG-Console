#include "HttpAdapter.h"

namespace scag { namespace re { namespace http {

using namespace scag::util::properties;

Hash<AccessType> HttpCommandAdapter::RequestFieldsAccess = HttpCommandAdapter::InitRequestAccess();
Hash<AccessType> HttpCommandAdapter::ResponseFieldsAccess = HttpCommandAdapter::InitResponseAccess();
Hash<AccessType> HttpCommandAdapter::DeliveryFieldsAccess = HttpCommandAdapter::InitDeliveryAccess();

Hash<AccessType> HttpCommandAdapter::InitRequestAccess()
{
    Hash<AccessType> hs;

    hs.Insert("abonent", atRead);
    hs.Insert("usr", atRead);
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
    hs.Insert("usr", atRead);
    hs.Insert("status", atReadWrite);
    hs.Insert("message", atReadWrite);

    return hs;
}

Hash<AccessType> HttpCommandAdapter::InitDeliveryAccess()
{
    Hash<AccessType> hs;

    hs.Insert("abonent", atRead);
    hs.Insert("usr", atRead);
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
        prop = new AdapterProperty(name, this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strncmp(name.c_str(), "param-", 6))
        prop = new AdapterProperty(name, this, cmd.getQueryParameter(name.c_str() + 6));
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name, this, cmd.getAddress());
    else if(!strcmp(name.c_str(), "site"))
        prop = new AdapterProperty(name, this, cmd.getSite());
    else if(!strcmp(name.c_str(), "path"))
        prop = new AdapterProperty(name, this, cmd.getSitePath());
    else if(!strcmp(name.c_str(), "filename"))
        prop = new AdapterProperty(name, this, cmd.getSiteFileName());
    else if(!strcmp(name.c_str(), "port"))
        prop = new AdapterProperty(name, this, cmd.getSitePort());
    else if(!strcmp(name.c_str(), "message"))
        prop = new AdapterProperty(name, this, cmd.getMessageText());
    else if(!strcmp(name.c_str(), "usr"))
        prop = new AdapterProperty(name, this, cmd.getUSR());

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
        prop = new AdapterProperty(name, this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strcmp(name.c_str(), "status"))
        prop = new AdapterProperty(name, this, cmd.getStatus());
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name, this, cmd.getAddress());
    else if(!strcmp(name.c_str(), "usr"))
        prop = new AdapterProperty(name, this, cmd.getUSR());
    else if(!strcmp(name.c_str(), "message"))
        prop = new AdapterProperty(name, this, cmd.getMessageText());

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
        prop = new AdapterProperty(name, this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strcmp(name.c_str(), "status"))
        prop = new AdapterProperty(name, this, cmd.getStatus());
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name, this, cmd.getAddress());
    else if(!strcmp(name.c_str(), "usr"))
        prop = new AdapterProperty(name, this, cmd.getUSR());
    else if(!strcmp(name.c_str(), "delivered"))
        prop = new AdapterProperty(name, this, cmd.getDelivered());

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

void HttpCommandAdapter::changed(AdapterProperty& property)
{
    if(command.getCommandId() == HTTP_RESPONSE)
    {
        HttpResponse& cmd = (HttpResponse&)command;

        if(!strncmp(property.GetName().c_str(), "header-", 7))
            cmd.setHeaderField(property.GetName().c_str() + 7, property.getStr());
        else if(!strcmp(property.GetName().c_str(), "status"))
            cmd.setStatus(property.getInt());
/*        else if(!strcmp(property.GetName().c_str(), "abonent"))
            cmd.setAbonent(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "usr"))
            cmd.setUSR(property.getInt());*/
        else if(!strcmp(property.GetName().c_str(), "message"))
            cmd.setMessageText(property.getStr());
    }
    else if(command.getCommandId() == HTTP_REQUEST)
    {
        HttpRequest& cmd = (HttpRequest&)command;

        if(!strncmp(property.GetName().c_str(), "header-", 7))
            cmd.setHeaderField(property.GetName().c_str() + 7, property.getStr());
        else if(!strncmp(property.GetName().c_str(), "param-", 6))
            cmd.setQueryParameter(property.GetName().c_str() + 6, property.getStr());
/*        else if(!strcmp(property.GetName().c_str(), "abonent"))
            cmd.setAbonent(property.getStr());*/
        else if(!strcmp(property.GetName().c_str(), "site"))
            cmd.setSite(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "path"))
            cmd.setSitePath(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "filename"))
            cmd.setSiteFileName(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "port"))
            cmd.setSitePort(property.getInt());
/*        else if(!strcmp(property.GetName().c_str(), "message"))
            cmd.setMessageText(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "usr"))
            cmd.setUSR(property.getInt());*/
    }
}


}}}
