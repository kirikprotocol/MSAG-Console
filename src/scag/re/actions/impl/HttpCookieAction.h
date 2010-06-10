#ifndef _SCAG_RE_ACTIONS_IMPL_HTTPCOOKIEACTION_
#define _SCAG_RE_ACTIONS_IMPL_HTTPCOOKIEACTION_

#include "scag/re/base/Action2.h"
#include "scag/re/base/StringField.h"
#include "scag/transport/http/base/HttpCommand2.h"

namespace scag2 {
namespace  re {
namespace actions {

class CookieAction : public Action
{
public:
    typedef enum {
            GET = 1,
            SET = 2,
            DEL = 3
    } ActionType;

    CookieAction( ActionType at = GET ) :
    atype_(at),
    name_(*this,"name",true,true),
    value_(*this,"value",true,at == SET),
    path_(*this,"path",true,at == SET),
    domain_(*this,"domain",true,at == SET),
    expires_(*this,"expires",true,at == SET) {}

    virtual ~CookieAction() {}
    virtual bool run(ActionContext& context);
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual const char* opname() const {
        return ( atype_ == GET ? "http:get-cookie" :
                 ( atype_ == SET ? "http:set-cookie" : "http:del-cookie" ));
    }

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    // bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
    bool getCookieParam(ActionContext& context, StringField& f, transport::http::Cookie *c);
    bool setCookieParam(ActionContext& context, StringField& f, transport::http::Cookie *c);

protected:
    ActionType  atype_;
    StringField name_;
    StringField value_;
    StringField path_;
    StringField domain_;
    StringField expires_;

    /*
    bool set, bPathExist, bDomainExist, bExpiresExist;
    std::string strName;
    std::string strValue;
    std::string strPath;
    std::string strDomain;
    std::string strExpires;
    FieldType ftName, ftValue, ftPath, ftDomain, ftExpires;
     */
};

}}}

#endif
