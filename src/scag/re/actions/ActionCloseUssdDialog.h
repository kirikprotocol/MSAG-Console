#ifndef __SCAG_RULE_ENGINE_ACTION_WAIT__
#define __SCAG_RULE_ENGINE_ACTION_WAIT__

#include "ActionAbstractWait.h"

namespace scag { namespace re { namespace actions {

class ActionOperationWait : public ActionAbstractWait
{
    ActionOperationWait(const ActionOperationWait&);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionOperationWait() {}
};

}}}


#endif
