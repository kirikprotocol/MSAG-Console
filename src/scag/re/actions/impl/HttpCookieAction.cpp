#include "HttpCookieAction.h"
#include "scag/stat/base/Statistics2.h"
#include "scag/util/properties/Properties2.h"
// #include "scag/re/CommandAdapter.h"

namespace scag2 {
namespace re {
namespace actions {

using namespace transport::http;

void CookieAction::init(const SectionParams& params, PropertyObject propertyObject)
{
    // const char * name = 0;
    bool bExist;

    CheckParameter(params, propertyObject, "cookie", "name", true, true, strName, bExist);
    CheckParameter(params, propertyObject, "cookie", "value", true, set, strValue, bExist);
    CheckParameter(params, propertyObject, "cookie", "path", false, set, strPath, bPathExist);
    CheckParameter(params, propertyObject, "cookie", "domain", false, set, strDomain, bDomainExist);
    CheckParameter(params, propertyObject, "cookie", "expires", false, set, strExpires, bExpiresExist);

    smsc_log_debug(logger, "Inited '%s-CookieAction': name=%s, value=%s, path=%s, domain=%s, expires=%s", set ? "set" : "get", strName.c_str(), strValue.c_str(), strPath.c_str(), strDomain.c_str(), strExpires.c_str());
}

bool CookieAction::run(ActionContext& context)
{
    smsc_log_debug(logger, "Run '%s-CookieAction': name=%s, value=%s, path=%s, domain=%s, expires=%s", set ? "set" : "get", strName.c_str(), strValue.c_str(), strPath.c_str(), strDomain.c_str(), strExpires.c_str());

    std::string nm, val, path, domain, expires;

    if(!getStrProperty(context, strName, "name", nm) || !nm.length())
        return false;

    HttpCommand& hc = (HttpCommand&)context.getSCAGCommand();

    if(set)
    {
        if(!getStrProperty(context, strValue, "value", val))
            return false;

        if(!val.length())
        {
            smsc_log_debug(logger, "'CookieAction 'Empty 'value' parameter %s", strValue.c_str());
            return false;
        }

        Cookie* c = hc.setCookie(nm, val);

        if(!setCookieParam(context, strPath, "path", bPathExist, c) || 
            !setCookieParam(context, strDomain, "domain", bDomainExist, c) || 
            !setCookieParam(context, strExpires, "expires", bExpiresExist, c))
                return false;

        smsc_log_debug(logger, "Cookie is set name=%s, value=%s, path=%s, domain=%s, expires=%s", nm.c_str(), c->value.c_str(), c->getParam("path").c_str(), c->getParam("domain").c_str(), c->getParam("expires").c_str());
    }
    else
    {
        Cookie* c = hc.getCookie(nm);
        if(c)
        {
            Property* p;
            if (!(p = context.getProperty(strValue))) 
            {
                smsc_log_warn(logger,"Action 'Cookie': invalid 'value' property '%s'", strValue.c_str());
                return false;
            }
            const Property::string_type sss(c->value.c_str(),c->value.size());
            p->setStr(sss);

            if(!getCookieParam(context, strPath, "path", bPathExist, c) ||
                !getCookieParam(context, strDomain, "domain", bDomainExist, c) ||
                !getCookieParam(context, strExpires, "expires", bExpiresExist, c))
                    return false;

            smsc_log_debug(logger, "Cookie is got name=%s, value=%s, path=%s, domain=%s, expires=%s", nm.c_str(), c->value.c_str(), c->getParam("path").c_str(), c->getParam("domain").c_str(), c->getParam("expires").c_str());
        }
        else
            smsc_log_debug(logger, "Cookie doesn't exists name=%s", nm.c_str());
    }

    return true;
}

bool CookieAction::setCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, Cookie *c)
{
    std::string p;

    if(exists)
    {
        if(!getStrProperty(context, str, param_name, p))
            return false;

        if(p.length())
            c->setParam(param_name, p);
        else
            smsc_log_debug(logger, "'CookieAction' Empty %s parameter", param_name);
    }
    return true;
}

bool CookieAction::getCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, Cookie *c)
{
    Property* p;
    if(exists)
    {
        if (!(p = context.getProperty(str))) 
        {
            smsc_log_warn(logger,"Action 'Cookie': invalid '%s' property '%s'", param_name, str.c_str());
            return false;
        }
        const std::string& ss(c->getParam(param_name));
        const Property::string_type sss(ss.c_str(),ss.size());
        p->setStr(sss);
    }
    return true;
}

bool CookieAction::getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val)
{
    const char *name;
    Property * p2 = 0;
    FieldType ft = ActionContext::Separate(str, name);
    if(ft != ftUnknown)  
    {
        if (!(p2 = context.getProperty(str))) 
        {
            smsc_log_warn(logger,"Action 'Cookie': invalid '%s' property '%s'", field_name, str.c_str());
            return false;
        }

        val.assign(p2->getStr().c_str(),p2->getStr().size());
    } else
        val = str;
    return true;
}

IParserHandler * CookieAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'httpcookie:': cannot have a child object");
}

bool CookieAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
