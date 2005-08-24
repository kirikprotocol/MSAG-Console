#ifndef __SCAG_RULE_ENGINE_ACTION_SET__
#define __SCAG_RULE_ENGINE_ACTION_SET__

#include "ActionContext.h"
#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionSet : public Action
{
    ActionSet(const ActionSet &);
    std::string Variable;
    std::string Value;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual bool run(ActionContext& context);

    ActionSet();
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual ~ActionSet();

};



}}}


#endif
