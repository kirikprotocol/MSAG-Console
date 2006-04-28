#ifndef __SCAG_RULE_ENGINE_ACTION_WAIT__
#define __SCAG_RULE_ENGINE_ACTION_WAIT__

#include <string>
#include "Action.h"

namespace scag { namespace re { namespace actions {

class ActionOperationWait : public Action
{
    ActionOperationWait(const ActionOperationWait&);
    std::string m_sTime;
    int m_opType;
    FieldType m_ftTime;
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionOperationWait() : m_opType(-1) {};

};

}}}


#endif
