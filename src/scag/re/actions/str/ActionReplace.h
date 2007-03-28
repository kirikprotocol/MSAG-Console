#ifndef __SCAG_RULE_ENGINE_ACTION_REPLACE__
#define __SCAG_RULE_ENGINE_ACTION_REPLACE__

#include "scag/re/actions/ActionContext.h"
#include "scag/re/actions/Action.h"
#include "util/regexp/RegExp.hpp"


namespace scag { namespace re { namespace actions {

using namespace smsc::util::regexp;

class ActionReplace : public Action
{
    ActionReplace(const ActionReplace&);

    RegExp *re;
    std::string m_sVar, m_sValue, m_sReplace, m_sResult, m_sRegexp, m_wstrVar, m_wstrReplace;
    FieldType ftVar, ftValue, ftReplace, ftResult, ftRegexp;
    int m_type;

protected:
    bool getStrProperty(ActionContext& context, std::string& str, const char *field_name, std::string& val);
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

