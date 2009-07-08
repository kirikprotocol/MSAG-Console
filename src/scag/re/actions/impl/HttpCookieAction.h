#ifndef _SCAG_RE_ACTIONS_IMPL_HTTPCOOKIEACTION_
#define _SCAG_RE_ACTIONS_IMPL_HTTPCOOKIEACTION_

#include "scag/re/base/Action2.h"
#include "scag/transport/http/base/HttpCommand2.h"

namespace scag2 {
namespace  re {
namespace actions {

class CookieAction : public Action
{
public:
    CookieAction() : set(false) {}
    CookieAction(bool _set) : set(_set) {}
    virtual ~CookieAction() {}
    virtual bool run(ActionContext& context);
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual const char* opname() const { return set ? "http:set-cookie" : "http:get-cookie"; }

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
    bool getCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, transport::http::Cookie *c);
    bool setCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, transport::http::Cookie *c);

protected:
    bool set, bPathExist, bDomainExist, bExpiresExist;
    std::string strName;
    std::string strValue;
    std::string strPath;
    std::string strDomain;
    std::string strExpires;
    FieldType ftName, ftValue, ftPath, ftDomain, ftExpires;
};

}}}

#endif
