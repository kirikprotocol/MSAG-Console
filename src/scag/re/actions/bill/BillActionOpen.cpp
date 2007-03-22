#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

BillActionOpen::BillActionOpen(bool waitOperation)  
{
    m_waitOperation = waitOperation;
    m_ActionName = waitOperation ? "operation:bill_wait" : "bill:open";
}


void BillActionOpen::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;
    m_CategoryFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "category", true, true, m_category, bExist);

    if(m_CategoryFieldType == ftUnknown && !(category = atoi(m_category.c_str())))
        throw SCAGException("Action '%s': category should be integer or variable", m_ActionName.c_str());

    m_MediaTypeFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "content-type", true, true, m_mediaType, bExist);
    if(m_MediaTypeFieldType == ftUnknown && !(mediaType = atoi(m_mediaType.c_str())))
        throw SCAGException("Action '%s': content-type should be integer or variable", m_ActionName.c_str());

    m_StatusFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "status", true, false, m_sStatus, bExist);
    m_MsgFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "msg", false, false, m_sMessage, m_MsgExist);

    CheckParameter(params, propertyObject, m_ActionName.c_str(), "bill_id", false, false, m_sBillId, m_BillIdExist);
    CheckParameter(params, propertyObject, m_ActionName.c_str(), "result_number", false, false, m_sResNumber, m_ResNumExist);

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

void BillActionOpen::SetBillingStatus(ActionContext& context, const char * errorMsg, bool isOK, const TariffRec * tariffRec)
{
    Property * propertyStatus = context.getProperty(m_sStatus);
    if (!propertyStatus) 
    {
        smsc_log_debug(logger,"Action '%s' :: Invalid property %s for status", m_ActionName.c_str(), m_sStatus.c_str());
        return;
    }

    propertyStatus->setInt(!isOK);

    if (m_ResNumExist) 
    {
        Property * propertyResNum = context.getProperty(m_sResNumber);

        if (!propertyResNum)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for result_number", m_ActionName.c_str(), m_sResNumber.c_str());
        else
            propertyResNum->setInt(isOK && tariffRec ? tariffRec->ServiceNumber : 0);
    }

    if (m_MsgExist) 
    {
        Property * propertyMsg = context.getProperty(m_sMessage);

        if (!propertyMsg) 
        {
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for msg", m_ActionName.c_str(), m_sMessage.c_str());
            return;
        }
        propertyMsg->setStr(errorMsg);
    }

}

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action '%s'...", m_ActionName.c_str());

    /////////////////////////////////////////////
 
    Statistics& statistics = Statistics::Instance();

    if (m_MediaTypeFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_mediaType);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for content-type", false, 0);
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
            SetBillingStatus(context, "Invalid property for category", false, 0);
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
        SetBillingStatus(context, "operation is invalid", false, 0);
        return true;
    }
     
    BillingManager& bm = BillingManager::Instance();

    TariffRec * tariffRec = 0;
    try {
        tariffRec = context.getTariffRec(category, mediaType);
        if (!tariffRec) throw SCAGException("TariffRec is not valid");
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false, tariffRec);
        return true;
    }


    if(tariffRec->billType == scag::bill::infrastruct::NONE)
    {
        smsc_log_warn(logger, "Billing desabled for this tariff entry. ServiceNumber=%d, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber, tariffRec->CategoryId, tariffRec->MediaTypeId);
        operation->attachBill((uint32_t)-1);
        return true;
    }

    if (tariffRec->Price == 0)
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%d, CategoryId=%d, MediaTypeId=%d", tariffRec->ServiceNumber, tariffRec->CategoryId, tariffRec->MediaTypeId);

    BillingInfoStruct billingInfoStruct = context.getBillingInfoStruct();

    unsigned int BillId = 0;

    try 
    {
        BillId = bm.Open(billingInfoStruct, *tariffRec);

    } catch (SCAGException& e)
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false, 0);
        return true;
    }

    try 
    {
        if (m_waitOperation) 
            RegisterPending(context, BillId);
        else
            operation->attachBill(BillId);
    }
    catch (SCAGException& e)
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false, 0);
        if (!m_waitOperation) operation->detachBill();

        bm.Rollback(BillId);
        return true;
    }

    if(m_BillIdExist) 
    {
        Property * p = context.getProperty(m_sBillId);

        if (!p)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for BillID", m_ActionName.c_str(), m_sBillId.c_str());
        else
            p->setInt(BillId);
    }
    
    SetBillingStatus(context, "", true, tariffRec);
    smsc_log_debug(logger,"Action '%s' transaction successfully opened (BillId=%d)", m_ActionName.c_str(),BillId);
    return true;
}

}}}

