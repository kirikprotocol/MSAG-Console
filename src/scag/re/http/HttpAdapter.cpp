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

    hs.Insert("abonent", atReadWrite);
    hs.Insert("site", atReadWrite);
    hs.Insert("path", atReadWrite);
    hs.Insert("port", atReadWrite);
    hs.Insert("message", atReadWrite);

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

    HttpRequest& cmd = (HttpRequest&)command;

    if(!strncmp(name.c_str(), "header-", 7))
        prop = new AdapterProperty(name, this, cmd.getHeaderField(name.c_str() + 7));
    else if(!strncmp(name.c_str(), "param-", 6))
        prop = new AdapterProperty(name, this, cmd.getQueryParameter(name.c_str() + 6));
    else if(!strcmp(name.c_str(), "abonent"))
        prop = new AdapterProperty(name, this, cmd.getAbonent());
    else if(!strcmp(name.c_str(), "site"))
        prop = new AdapterProperty(name, this, cmd.getSite());
    else if(!strcmp(name.c_str(), "path"))
        prop = new AdapterProperty(name, this, cmd.getSitePath());
    else if(!strcmp(name.c_str(), "port"))
        prop = new AdapterProperty(name, this, cmd.getSitePort());
    else if(!strcmp(name.c_str(), "message"))
    {
        std::string str;
        TmpBuf<char, 1024> buf(1024);
        const std::wstring& wstr = cmd.getMessageText();
        const wchar_t* wchrs  = wstr.c_str();

        for(int i = 0; i < wstr.length(); i++)
        {
            char r = (*wchrs >> 8);
            buf.Append(&r, 1);
            r = *(wchrs++);
            buf.Append(&r, 1);
        }

        str.assign(buf.get(), buf.GetPos());
        prop = new AdapterProperty(name, this, str);
    }

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
        HttpRequest& cmd = (HttpRequest&)command;

        if(!strcmp(property.GetName().c_str(), "abonent"))
            cmd.setAbonent(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "site"))
            cmd.setSite(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "path"))
            cmd.setSitePath(property.getStr());
        else if(!strcmp(property.GetName().c_str(), "port"))
            cmd.setSitePort(property.getInt());
        else if(!strcmp(property.GetName().c_str(), "message"))
        {
            std::string str = property.getStr();
            TmpBuf<wchar_t, 1024> buf(1024);
            std::wstring wstr;
            const char* chrs  = str.c_str();

            for(int i = 0; i < str.length(); i+=2)
            {
                wchar_t r = (chrs[i] << 8) + chrs[i + 1];
                buf.Append(&r, 1);
            }

            wstr.assign(buf.get(), buf.GetPos());
            cmd.setMessageText(wstr);
        }
    }
}

}}}
