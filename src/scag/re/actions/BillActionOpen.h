#ifndef _BILL_ACTION_OPEN_
#define _BILL_ACTION_OPEN_

#include "scag/re/actions/ActionAbstractWait.h"
#include "scag/re/actions/LongCallAction.h"

namespace scag { namespace re { namespace actions {

class BillOpenCallParams : public LongCallParams
{
public:
    BillingInfoStruct billingInfoStruct;
    TariffRec tariffRec;
    int BillId;
    std::string exception;
};

class BillActionOpen : public LongCallAction, ActionAbstractWait
{
    std::string m_sName;
    //std::string m_sServiceName;

    std::string m_sStatus;
    std::string m_sMessage;
    std::string m_sResNumber;

    std::string m_category;
    std::string m_mediaType;


    uint32_t category;
    uint32_t mediaType;

    FieldType m_StatusFieldType;
    FieldType m_MediaTypeFieldType;
    FieldType m_CategoryFieldType;
    FieldType m_MsgFieldType;

    bool m_MsgExist;
    bool m_ResNumExist;

    bool m_waitOperation;

    void SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK, const TariffRec * tariffRec);
protected:
    virtual IParserHandler * StartXMLSubSection(const std::string& name, const SectionParams& params, const ActionFactory& factory);
    virtual bool FinishXMLSubSection(const std::string& name);

    virtual bool RunBeforePostpone(ActionContext& context);
    virtual void ContinueRunning(ActionContext& context);
public:
    BillActionOpen(bool waitOperation);
    //virtual bool run(ActionContext& context)
    virtual void init(const SectionParams& params,PropertyObject propertyObject);
};



}}}


#endif


