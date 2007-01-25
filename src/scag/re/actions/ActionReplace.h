#ifndef __SCAG_RULE_ENGINE_ACTION_REPLACE__
#define __SCAG_RULE_ENGINE_ACTION_REPLACE__

#include "ActionContext.h"
#include "Action.h"
#include "util/regexp/RegExp.hpp"


namespace scag { namespace re { namespace actions {

using namespace smsc::util::regexp;

class ActionReplace : public Action
{
    ActionReplace(const ActionReplace&);
    std::auto_ptr<ActionParameter> paramVar, paramRegexp, paramValue, paramResult;

    RegExp *re;
    std::string m_wstrVar, m_wstrReplace;
    int m_type;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionReplace ();

    virtual ~ActionReplace();

};


}}}


#endif

