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

    m_CategoryFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "category", false, true, m_category, bCatExist);
    if(!bCatExist)
    {
        m_CategoryFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "category-str", false, true, m_category, bExist);
        if(!bExist) throw SCAGException("Action '%s': category or category-str should present", m_ActionName.c_str());
    }
    if(m_CategoryFieldType == ftUnknown)
    {
        category = bCatExist ? atoi(m_category.c_str()) : BillingManager::Instance().getInfrastructure().GetCategoryID(m_category);
        if(!category) throw SCAGException("Action '%s': category should be integer or variable", m_ActionName.c_str());
    }

    m_MediaTypeFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "content-type", false, true, m_mediaType, bMtExist);
    if(!bMtExist)
    {
        m_MediaTypeFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "content-type-str", false, true, m_mediaType, bExist);
        if(!bExist) throw SCAGException("Action '%s': content-type or content-type-str should present", m_ActionName.c_str());
    }
    if(m_MediaTypeFieldType == ftUnknown)
    {
        mediaType = bMtExist ? atoi(m_mediaType.c_str()) : BillingManager::Instance().getInfrastructure().GetMediaTypeID(m_mediaType);
        if(!mediaType) throw SCAGException("Action '%s': content-type should be integer or variable", m_ActionName.c_str());
    }

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
    int cat = category, mt = mediaType, BillId = 0;

    smsc_log_debug(logger,"Run Action '%s'...", m_ActionName.c_str());

    Statistics& statistics = Statistics::Instance();
    BillingManager& bm = BillingManager::Instance();
    BillingInfoStruct billingInfoStruct = context.getBillingInfoStruct();

    if(m_MediaTypeFieldType != ftUnknown)
    {
        Property * property = context.getProperty(m_mediaType);
        if(!property)
        {
            SetBillingStatus(context, "Invalid property for content-type", false, 0);
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for content-type", m_ActionName.c_str(), m_mediaType.c_str());
            return true;
        }
        if(!bMtExist)
            billingInfoStruct.mediaType = property->getStr();

        mt = bMtExist ? property->getInt() : bm.getInfrastructure().GetMediaTypeID(billingInfoStruct.mediaType);
    }
    else if(!bMtExist)
        billingInfoStruct.mediaType = m_mediaType;

    if(m_CategoryFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_category);
        if (!property) 
        {
            SetBillingStatus(context, "Invalid property for category", false, 0);
            smsc_log_error(logger,"Action '%s' :: Invalid property %s for category", m_ActionName.c_str(), m_category.c_str());
            return true;
        }
        if(!bCatExist)
            billingInfoStruct.category = property->getStr();

        cat = bCatExist ? property->getInt() : bm.getInfrastructure().GetCategoryID(billingInfoStruct.category);
    }
    else if(!bCatExist)
        billingInfoStruct.category = m_category;

    if(!cat || !mt)
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
     
    TariffRec * tariffRec = 0;
    try {
        tariffRec = context.getTariffRec(cat, mt);
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

