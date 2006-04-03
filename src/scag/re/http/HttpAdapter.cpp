#include "HttpAdapter.h"

namespace scag { namespace re { namespace http {

using namespace scag::util::properties;

/*
method r/w
site r/w
path r/w
port r/w
query r/w
site_full r/w
param_%s r
header_%s r/w
body_text r
body_binary r
body_append w

status r/w
header_%s r/w
body_append w
*/

Hash<AccessType> HttpCommandAdapter::RequestFieldsAccess = HttpCommandAdapter::InitRequestAccess();

Hash<AccessType> HttpCommandAdapter::InitRequestAccess()
{
    Hash<AccessType> hs;

    hs.Insert("method",atReadWrite);
    hs.Insert("site", atReadWrite);
    hs.Insert("path", atReadWrite);
    hs.Insert("port", atReadWrite);
    hs.Insert("query", atReadWrite);

    return hs;
}

AccessType HttpCommandAdapter::CheckAccess(int handlerType,const std::string& name)
{
    if(!strncmp(name.c_str(), "header-", 7))
        return atReadWrite;

    if(handlerType == HTTP_RESPONSE && !strcmp(name.c_str(), "status"))
        return atReadWrite;

    if(handlerType == HTTP_REQUEST)
    {
        if(!strncmp(name.c_str(), "param-", 6))
            return atRead;

        AccessType *a = RequestFieldsAccess.GetPtr(name.c_str());
        if(a) return *a;
    }

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

    if(!strncmp(name.c_str(), "header-", 7))
        prop = new AdapterProperty(name, this, command.getHeaderField(name.c_str() + 7));

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

    return 0;
}

void HttpCommandAdapter::changed(AdapterProperty& property)
{
    if(!strncmp(property.GetName().c_str(), "header-", 7))
        command.setHeaderField(property.GetName().c_str() + 7, property.getStr());
    else if(command.getCommandId() == HTTP_RESPONSE)
    {
        HttpResponse& cmd = (HttpResponse&)command;

        if(!strcmp(property.GetName().c_str(), "status"))
            cmd.setStatus(property.getInt());
    }
    else if(command.getCommandId() == HTTP_REQUEST)
    {
        //TODO: implement other request fields
    }
}

}}}
