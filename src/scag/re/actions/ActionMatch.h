#ifndef __SCAG_RULE_ENGINE_ACTION_MATCH__
#define __SCAG_RULE_ENGINE_ACTION_MATCH__

#include "ActionContext.h"
#include "Action.h"
#include "util/regexp/RegExp.hpp"


namespace scag { namespace re { namespace actions {

using namespace smsc::util::regexp;

class ActionMatch : public Action
{
    ActionMatch(const ActionMatch&);

    std::string s_Regexp;
    std::string s_Value;
    std::wstring w_Regexp;
    std::wstring w_Value;


    std::string s_Result;
    std::wstring w_Result;

    RegExp *re;
    FieldType ftValue;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionMatch ();

    virtual ~ActionMatch();

};


}}}


#endif

