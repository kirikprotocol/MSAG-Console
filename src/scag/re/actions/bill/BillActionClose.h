#ifndef _BILL_ACTION_CLOSE_
#define _BILL_ACTION_CLOSE_

#include "scag/re/actions/Action.h"

namespace scag { namespace re {namespace actions {

class BillActionClose : public Action
{
    std::string m_sStatus;
    std::string m_sMessage;

    bool m_StatusExist;
    bool m_MsgExist;

    bool actionCommit;

    void SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    BillActionClose() {}
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
    virtual bool run(ActionContext& context);
};

}}}

#endif
