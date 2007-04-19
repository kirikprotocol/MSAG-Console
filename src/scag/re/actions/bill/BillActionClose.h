#ifndef _BILL_ACTION_CLOSE_
#define _BILL_ACTION_CLOSE_

#include "scag/re/actions/Action.h"
#include "scag/re/actions/LongCallAction.h"

namespace scag { namespace re {namespace actions {

class BillActionClose : public LongCallAction
{
    std::string m_sStatus;
    std::string m_sMessage;
    std::string m_sBillID;
    uint32_t BillID;

    bool m_StatusExist;
    bool m_MsgExist;
    bool m_BillIDExist;

    bool actionCommit;

    void SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    BillActionClose() {}
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);
};

}}}

#endif
