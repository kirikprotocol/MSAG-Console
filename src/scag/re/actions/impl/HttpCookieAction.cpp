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
    /*
     bool bExist;

    CheckParameter(params, propertyObject, "cookie", "name", true, true, strName, bExist);
    CheckParameter(params, propertyObject, "cookie", "value", true, set, strValue, bExist);
    CheckParameter(params, propertyObject, "cookie", "path", false, set, strPath, bPathExist);
    CheckParameter(params, propertyObject, "cookie", "domain", false, set, strDomain, bDomainExist);
    CheckParameter(params, propertyObject, "cookie", "expires", false, set, strExpires, bExpiresExist);
     */

    name_.init(params,propertyObject);
    value_.init(params,propertyObject);
    path_.init(params,propertyObject);
    domain_.init(params,propertyObject);
    expires_.init(params,propertyObject);
    smsc_log_debug(logger, "Action '%s' inited: name=%s, value=%s, path=%s, domain=%s, expires=%s",
                   opname(), name_.getStringValue(), value_.getStringValue(), path_.getStringValue(),
                   domain_.getStringValue(), expires_.getStringValue());
}


bool CookieAction::run(ActionContext& context)
{
    smsc_log_debug(logger, "Run '%s': name=%s, value=%s, path=%s, domain=%s, expires=%s",
                   opname(), name_.getStringValue(), value_.getStringValue(),
                   path_.getStringValue(), domain_.getStringValue(), expires_.getStringValue());

    const char* nm = name_.getValue(context);
    if ( !nm || !strlen(nm) ) return false;

    HttpCommand& hc = (HttpCommand&)context.getSCAGCommand();

    switch (atype_) 
    {
    case SET: {
        const char* val = value_.getValue(context);
        if (!val) return false;

        const size_t vallen = strlen(val);
        if(!vallen && !hc.isResponse())
        {
            smsc_log_debug(logger, "'%s' 'Empty 'value' parameter %s, use del-cookie",
                           opname(), value_.getStringValue());
            return false;
        }

        Cookie* c = hc.setCookie(nm, val);

        if ( vallen ) {
            if ( !setCookieParam(context, path_, c) ||
                 !setCookieParam(context, domain_, c) || 
                 !setCookieParam(context, expires_, c) ) {
                return false;
            }
        }
        smsc_log_debug(logger, "Cookie is set name=%s, value=%s, path=%s, domain=%s, expires=%s",
                       nm, c->value.c_str(), c->getParam("path").c_str(), c->getParam("domain").c_str(), c->getParam("expires").c_str());
        break;
    }
    case GET: {

        Cookie* c = hc.getCookie(nm);
        if (c)
        {
            Property* p = value_.getProperty(context);
            if (!p)
            {
                smsc_log_warn(logger,"Action '%s': invalid 'value' property '%s'",
                              opname(), value_.getStringValue());
                return false;
            }
            const Property::string_type sss(c->value.c_str(),c->value.size());
            p->setStr(sss);

            if(!getCookieParam(context, path_, c) ||
               !getCookieParam(context, domain_, c) ||
               !getCookieParam(context, expires_, c))
                return false;

            smsc_log_debug(logger, "Cookie is got name=%s, value=%s, path=%s, domain=%s, expires=%s",
                           nm, c->value.c_str(), c->getParam("path").c_str(), c->getParam("domain").c_str(), c->getParam("expires").c_str());
        } else {
            smsc_log_debug(logger, "Cookie doesn't exists name=%s", nm);
        }
        break;
    }
    case DEL : {
        hc.delCookie(nm);
        break;
    }
    } // switch
    return true;
}


bool CookieAction::setCookieParam( ActionContext& context, StringField& f, Cookie *c)
{
    if(f.isFound())
    {
        const char* p = f.getValue(context);
        if (!p) return false;

        if(strlen(p))
            c->setParam(f.getName(), std::string(p));
        else
            smsc_log_debug(logger, "'%s' Empty %s parameter", opname(), f.getName());
    }
    return true;
}


bool CookieAction::getCookieParam(ActionContext& context, StringField& f, Cookie *c)
{
    Property* p;
    if(f.isFound())
    {
        Property* p = f.getProperty(context);
        if (!p)
        {
            smsc_log_warn(logger,"Action '%s': invalid '%s' property '%s'", opname(), f.getName(), f.getStringValue());
            return false;
        }
        const std::string& ss(c->getParam(f.getName()));
        const Property::string_type sss(ss.c_str(),ss.size());
        p->setStr(sss);
    }
    return true;
}

/*
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
 */

IParserHandler * CookieAction::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'httpcookie:': cannot have a child object");
}

bool CookieAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
