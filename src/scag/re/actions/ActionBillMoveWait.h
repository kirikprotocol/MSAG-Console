#ifndef __SCAG_RULE_ENGINE_ACTION_BILL_MOVE_WAIT__
#define __SCAG_RULE_ENGINE_ACTION_BILL_MOVE_WAIT__

#include "ActionAbstractWait.h"

namespace scag { namespace re { namespace actions {

class ActionBillMoveWait : public ActionAbstractWait
{
    ActionBillMoveWait(const ActionBillMoveWait&);
    std::string m_sStatus;
    std::string m_sMessage;

    bool m_MsgExist;
    FieldType m_ftMessage;

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
    ActionBillMoveWait() {}
};

}}}


#endif
