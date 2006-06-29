#include <scag/exc/SCAGExceptions.h>
#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace bill {

BillActionOpen::BillActionOpen(bool waitOperation) : logger(0) 
{
    m_waitOperation = waitOperation;
    if (waitOperation) m_ActionName = "operation:bill_wait";
    else m_ActionName = "bill:open";
}


void BillActionOpen::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.bill.actions");

    FieldType ft;
    std::string temp;
    bool bExist;

    m_CategoryFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "category", true, true, m_category, bExist);

    if(m_CategoryFieldType == ftUnknown && !(category = atoi(m_category.c_str())))
        throw InvalidPropertyException("Action '%s': category should be integer or variable", m_ActionName.c_str());

    m_MediaTypeFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "content-type", true, true, m_mediaType, bExist);

    if(m_MediaTypeFieldType == ftUnknown && !(mediaType = atoi(m_mediaType.c_str())))
        throw InvalidPropertyException("Action '%s': content-type should be integer or variable", m_ActionName.c_str());

    m_StatusFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "status", true, false, m_sStatus, bExist);

    m_MsgFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "msg", false, false, temp, m_MsgExist);

    if (m_MsgExist)
        m_sMessage = temp;


    if (m_waitOperation) InitParameters(params, propertyObject);

    smsc_log_debug(logger,"Action '%s' init...", m_ActionName.c_str());
}


IParserHandler * BillActionOpen::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action '%s' cannot include child objects", m_ActionName.c_str());
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
        smsc_log_debug(logger,"Action '%s' :: Invalid property %s for status", m_ActionName.c_str(), m_sStatus.c_str());
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
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for msg", m_ActionName.c_str(), m_sMessage.c_str());
            return;
        }
        property->setStr(errorMsg);
    }

}

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action '%s'...", m_ActionName.c_str());

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;

    if (m_MediaTypeFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_mediaType);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for content-type", false);
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for content-type", m_ActionName.c_str(), m_mediaType.c_str());
            return true;
        }
        mediaType = property->getInt();
    } 

    if (m_CategoryFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_category);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for category", false);
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for category", m_ActionName.c_str(), m_category.c_str());
            return true;
        }
        category = property->getInt();
    } 

    if(!category || !mediaType)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Empty category or content-type", m_ActionName.c_str());
        return false;
    }

    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"Action '%s': Fatal error in action - operation from ActionContext is invalid", m_ActionName.c_str());
        SetBillingStatus(context, "operation is invalid", false);
        return true;
    }

    BillingManager& bm = BillingManager::Instance();


    TariffRec * tariffRec;
    try {

        tariffRec = context.getTariffRec(category, mediaType);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: %s", m_ActionName.c_str(), e.what()); 
        SetBillingStatus(context, e.what(), false);
        return true;
    }

    if (!tariffRec) 
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: Cannot find TariffRec", m_ActionName.c_str()); 
        SetBillingStatus(context, "Cannot find TariffRec", false);
        return true;
    }

    if(tariffRec->Price == 0)
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%d, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber, tariffRec->CategoryId, tariffRec->MediaTypeId);

    smsc::inman::interaction::ChargeSms op;
    EventMonitor monitor;
    TransactionStatus transactionStatus;
    int BillId = 0;
    context.fillChargeOperation(op, *tariffRec);

    try 
    {
        BillId = bm.ChargeBill(op, &monitor, *tariffRec);

        //TODO: ������ ����� ����� ����� ����� �� ��������
        monitor.wait(1000);
        transactionStatus = bm.GetStatus(BillId);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false);
        context.makeBillEvent(TRANSACTION_OPEN, EXTERNAL_ERROR, *tariffRec, ev);
        statistics.registerSaccEvent(ev);
        return true;
    }

    switch (transactionStatus) 
    {
    case TRANSACTION_INVALID:
        smsc_log_error(logger,"Action '%s': billing transaction invalid", m_ActionName.c_str());
        SetBillingStatus(context, "billing transaction invalid", false);
        context.makeBillEvent(TRANSACTION_OPEN, INVALID_TRANSACTION, *tariffRec, ev);
        break;
    case TRANSACTION_NOT_STARTED:
        smsc_log_error(logger,"Action 'bill:open': billing transaction deny", m_ActionName.c_str());
        SetBillingStatus(context, "billing transaction deny", false);
        context.makeBillEvent(TRANSACTION_OPEN, REJECTED_BY_SERVER, *tariffRec, ev);
        break;
    case TRANSACTION_WAIT_ANSWER:
        smsc_log_error(logger,"Action '%s': billing transaction time out", m_ActionName.c_str());
        SetBillingStatus(context, "billing transaction time out", false);
        context.makeBillEvent(TRANSACTION_OPEN, SERVER_NOT_RESPONSE, *tariffRec, ev);
        break;
    default:
        //TRANSACTION_VALID
        try
        {
            operation->attachBill(BillId);
        } catch (SCAGException& e)
        {
            smsc_log_warn(logger,"Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
            context.makeBillEvent(TRANSACTION_OPEN, EXTERNAL_ERROR, *tariffRec, ev);
            SetBillingStatus(context, e.what(), false);
            break;
        }

        bm.sendReject(BillId);

        SetBillingStatus(context, "", true);
        context.makeBillEvent(TRANSACTION_OPEN, COMMAND_SUCCESSFULL, *tariffRec, ev);

        if (m_waitOperation) RegisterPending(context);

        smsc_log_warn(logger,"Action '%s' transaction successfully opened", m_ActionName.c_str());
        break;
    }

        
    statistics.registerSaccEvent(ev);
    return true;
}

}}

