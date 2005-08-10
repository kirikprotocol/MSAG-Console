#ifndef __SCAG_RULE_ENGINE_ACTION_RETURN__
#define __SCAG_RULE_ENGINE_ACTION_RETURN__

#include "ActionContext.h"
#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionReturn : public Action
{
    std::string ReturnValue;
    ActionReturn(const ActionReturn &);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionReturn (){};

    virtual ~ActionReturn();

};



}}}


#endif

