#ifndef _BILL_ACTION_OPEN_
#define _BILL_ACTION_OPEN_

#include "scag/re/actions/Action.h"

namespace scag { namespace bill {

using namespace scag::re::actions;
using namespace scag::re;

class BillActionOpen : public Action
{
    std::string m_sName;
    //std::string m_sServiceName;

    std::string m_sStatus;
    std::string m_sMessage;

    Logger * logger;
    std::string m_category;
    std::string m_mediaType;

    FieldType m_StatusFieldType;
    FieldType m_MediaTypeFieldType;
    FieldType m_CategoryFieldType;
    FieldType m_MsgFieldType;

    void SetBillingStatus(ActionContext& context, std::string& errorMsg, bool isOK);

protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);
public:
    BillActionOpen() : logger(0) {}
    virtual bool run(ActionContext& context);
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}


#endif


