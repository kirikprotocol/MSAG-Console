#ifndef __SCAG_RULE_ENGINE_ACTION_ABORT__
#define __SCAG_RULE_ENGINE_ACTION_ABORT__

#include "ActionContext.h"
#include "Action.h"


namespace scag { namespace re { namespace actions {

class ActionAbort : public Action
{
    ActionAbort(const ActionAbort&);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionAbort (){};

    virtual ~ActionAbort();

};



}}}


#endif

