#ifndef __SCAG_RULE_ENGINE_ACTION_WAIT__
#define __SCAG_RULE_ENGINE_ACTION_WAIT__

#include <string>
#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionOperationWait : public Action
{
    ActionOperationWait(const ActionOperationWait&);
    std::string sTime;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionOperationWait (){};

    virtual ~ActionOperationWait();

};

}}}


#endif
