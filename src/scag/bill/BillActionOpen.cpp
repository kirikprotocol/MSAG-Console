#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace bill {


void BillActionOpen::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.bill.actions");

    FieldType ft;
    std::string temp;
    bool bExist;

    m_CategoryFieldType = CheckParameter(params, propertyObject, "bill:open", "category", true, true, temp, bExist);
    m_category = ConvertWStrToStr(temp);

    m_MediaTypeFieldType = CheckParameter(params, propertyObject, "bill:open", "content-type", true, true, temp, bExist);
    m_mediaType = ConvertWStrToStr(temp);

    m_StatusFieldType = CheckParameter(params, propertyObject, "bill:open", "status", true, false, temp, bExist);
    m_sStatus = ConvertWStrToStr(temp);

    m_MsgFieldType = CheckParameter(params, propertyObject, "bill:open", "msg", false, false, temp, m_MsgExist);

    if (m_MsgExist) 
        m_sMessage = ConvertWStrToStr(temp);

    smsc_log_debug(logger,"Action 'bill:open' init...");
}


IParserHandler * BillActionOpen::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("BillAction 'bill:open' cannot include child objects");
}

bool BillActionOpen::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void BillActionOpen::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK)
{
    Property * property = context.getProperty(m_sStatus);
    if (!property) 
    {
        smsc_log_debug(logger,"BillAction 'bill:open' :: Invalid property %s for status", m_sStatus.c_str());
        return;
    }

    if (isOK) 
    {
        property->setInt(0);
        return;
    }

    property->setInt(1);

    if (m_MsgExist) 
    {
        property = context.getProperty(m_sMessage);

        if (!property) 
        {
            smsc_log_debug(logger,"BillAction 'bill:open' :: Invalid property %s for msg", m_sMessage.c_str());
            return;
        }
        property->setStr(std::string(errorMsg));
    }

    return;

}

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'BillActionOpen'...");

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;


    /////////////////////////////////////////////
    std::string category;
    std::string mediaType;

    if (m_MediaTypeFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_mediaType);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for content-type", false);
            smsc_log_error(logger,"BillAction 'bill:open' :: Invalid property %s for content-type", m_mediaType.c_str());
            return true;
        }
        mediaType = ConvertWStrToStr(property->getStr());
    } 
    else 
        mediaType = m_mediaType;


    if (m_CategoryFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_category);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for category", false);
            smsc_log_error(logger,"BillAction 'bill:open' :: Invalid property %s for category", m_category.c_str());
            return true;
        }
        category = ConvertWStrToStr(property->getStr());
    } 
    else 
        category = m_category;


    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionOpen: Fatal error in action - operation from ActionContext is invalid");
        SetBillingStatus(context, "operation from ActionContext is invalid", false);
        return true;
    }

    BillingManager& bm = BillingManager::Instance();


    TariffRec * tariffRec;
    try {
        tariffRec = context.getTariffRec(category, mediaType);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"BillAction 'bill:open' cannot process. Delails: %s", e.what()); 
        SetBillingStatus(context, e.what(), false);
        return true;
    }

    if (!tariffRec) 
    {
        smsc_log_warn(logger,"BillAction 'bill:open' cannot process. Delails: Cannot find TariffRec"); 
        SetBillingStatus(context, "Cannot find TariffRec", false);
        return true;
    }

    smsc::inman::interaction::ChargeSms op;
    EventMonitor monitor;

    try 
    {
        context.fillChargeOperation(op, *tariffRec);
        int BillId = bm.ChargeBill(op, &monitor, *tariffRec);

        //TODO: Понять какое время нужно ждать до таймаута
        monitor.wait(1000);
        TransactionStatus transactionStatus = bm.GetStatus(BillId);

        if ((TRANSACTION_INVALID == transactionStatus)||(TRANSACTION_NOT_STARTED == transactionStatus)) 
        {
            smsc_log_error(logger,"BillAction 'bill:open': billing transaction deny");
            SetBillingStatus(context, "billing transaction deny", false);
            context.makeBillEvent(TRANSACTION_REFUSED, *tariffRec, ev);
        }
        else if (TRANSACTION_WAIT_ANSWER == transactionStatus) 
        {
            smsc_log_error(logger,"BillAction 'bill:open': billing transaction time out");
            SetBillingStatus(context, "billing transaction time out", false);
            context.makeBillEvent(TRANSACTION_TIME_OUT, *tariffRec, ev);
        }
        else 
        {
            operation->attachBill(BillId);
            context.makeBillEvent(TRANSACTION_OPEN, *tariffRec, ev);
            bm.sendReject(BillId);

            statistics.registerSaccEvent(ev);
            SetBillingStatus(context, "", true);

            return true;
        }
      
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"BillAction 'bill:open' unable to process bill:open. Delails: %s", e.what());
        SetBillingStatus(context, e.what(), false);
        return true;
    }

    statistics.registerSaccEvent(ev);
    return true;
}

}}

