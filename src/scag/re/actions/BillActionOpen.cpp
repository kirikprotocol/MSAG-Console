#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

BillActionOpen::BillActionOpen(bool waitOperation)  
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

    ft = CheckParameter(params, propertyObject, m_ActionName.c_str(), "result_number", false, false, m_sResNumber, m_ResNumExist);


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

    if (isOK) propertyStatus->setInt(0);

    if (m_ResNumExist) 
    {
        Property * propertyResNum = context.getProperty(m_sResNumber);

        if (!propertyResNum)
            smsc_log_debug(logger,"Action '%s' :: Invalid property %s for result_number", m_ActionName.c_str(), m_sResNumber.c_str());
        else
        {
            if ((isOK)&&(tariffRec)) 
                propertyResNum->setInt(tariffRec->ServiceNumber);
            else
                propertyResNum->setInt(0);
        }
    }

    if (!isOK) 
    {
        propertyStatus->setInt(1);

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

}


bool BillActionOpen::RunBeforePostpone(ActionContext& context)
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
            return false;
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
            return false;
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
        return false;
    }

    BillingManager& bm = BillingManager::Instance();

    BillOpenCallParams* params = new BillOpenCallParams();
    
    if(!context.getTariffRec(category, mediaType, params->tariffRec))
    {
        smsc_log_warn(logger,"Action '%s' cannot process. Delails: TariffRec is not valid", m_ActionName.c_str());
        SetBillingStatus(context, "TariffRec is not valid", false, &params->tariffRec);
        delete params;
        return false;
    }

    if (params->tariffRec.Price == 0)
        smsc_log_warn(logger, "Zero price in tariff matrix. ServiceNumber=%d, CategoryId=%d, MediaTypeId=%d", params->tariffRec.ServiceNumber, params->tariffRec.CategoryId, params->tariffRec.MediaTypeId);

    params->billingInfoStruct = context.getBillingInfoStruct();

    context.getSCAGCommand().getLongCallContext().setParams(params);    
    return true;
}

void BillActionOpen::ContinueRunning(ActionContext& context)
{
    LongCallContext& lcmCtx = context.getSCAGCommand().getLongCallContext();
    BillOpenCallParams* params = (BillOpenCallParams*)lcmCtx.getParams();
     
    if(params->exception.length()) 
    {
        smsc_log_error(logger,"Action '%s': Fatal error in action: %d", m_ActionName.c_str(), params->exception.c_str());
        SetBillingStatus(context, params->exception.c_str(), false, 0);
        lcmCtx.freeParams();
        return;
    }
    
    Operation * operation = context.GetCurrentOperation();

    if (!operation) 
    {
        smsc_log_error(logger,"Action '%s': Fatal error in action - operation from ActionContext is invalid", m_ActionName.c_str());
        SetBillingStatus(context, "operation is invalid", false, 0);
        lcmCtx.freeParams();
        return;
    }
    
    smsc_log_debug(logger,"Continue Action '%s'...", m_ActionName.c_str());

    try 
    {
        if (m_waitOperation) 
            RegisterPending(context, params->BillId);
        else
            operation->attachBill(params->BillId);
            
        SetBillingStatus(context, "", true, &params->tariffRec);
        smsc_log_debug(logger,"Action '%s' transaction successfully opened", m_ActionName.c_str());
    }
    catch (SCAGException& e)
    {
        smsc_log_warn(logger, "Action '%s' unable to process. Delails: %s", m_ActionName.c_str(), e.what());
        SetBillingStatus(context, e.what(), false, 0);
        if (!m_waitOperation) operation->detachBill();
        BillingManager::Instance().Rollback(params->BillId);
    }

    lcmCtx.freeParams();
}

}}}

