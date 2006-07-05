#ifndef _HTTP_ACTION_COOKIE_
#define _HTTP_ACTION_COOKIE_

#include "scag/re/actions/Action.h"
#include "scag/transport/http/RouterTypes.h"

namespace scag { namespace  transport { namespace http {

using namespace scag::re::actions;
using namespace scag::re;
using smsc::logger::Logger;

    class CookieAction : public Action
    {
    protected:
        bool set, bPathExist, bDomainExist, bExpiresExist;
        std::string strName;
        std::string strValue;
        std::string strPath;
        std::string strDomain;
        std::string strExpires;
        FieldType ftName, ftValue, ftPath, ftDomain, ftExpires;

        Logger * logger;

        virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
        virtual bool FinishXMLSubSection(const std::string& name);

        bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
        bool getCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, Cookie *c);
        bool setCookieParam(ActionContext& context, std::string& str, const char *param_name, bool exists, Cookie *c);

    public:
        CookieAction() : set(false), logger(NULL) {}
        CookieAction(bool _set) : set(_set), logger(NULL) {}
        ~CookieAction() {}
        virtual bool run(ActionContext& context);
        virtual void init(const SectionParams& params,PropertyObject propertyObject);
    };

}}}


#endif


