#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace bill {


void BillActionOpen::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    const char * name = 0;
    AccessType at;

    if (!logger) 
        logger = Logger::getInstance("scag.bill.actions");

    if (params.Exists("category"))
    {
        m_category = ConvertWStrToStr(params["category"]);
        m_CategoryFieldType = ActionContext::Separate(m_category,name);

        if (m_CategoryFieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
            if (!(at&atRead)) 
                throw SCAGException("Action 'bill:open': cannot read property '%s' - no access",m_category.c_str());
        }

    }
    else 
        throw SCAGException("BillAction 'bill:open' : missing 'category' parameter");

    if (params.Exists("content-type")) 
    {
        m_mediaType = ConvertWStrToStr(params["content-type"]);

        m_MediaTypeFieldType = ActionContext::Separate(m_mediaType,name);

        if (m_MediaTypeFieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
            if (!(at&atRead)) 
                throw SCAGException("Action 'bill:open': cannot read property '%s' - no access",m_mediaType.c_str());
        }

    }
    else 
        throw SCAGException("BillAction 'bill:open' : missing 'content-type' parameter");

    /*
    ft = ActionContext::Separate(m_category,name);

    if (ft!=ftUnknown) 
        throw SCAGException("BillAction 'bill:open' : cannot use field for string parameter 'category'");


    ft = ActionContext::Separate(m_mediaType,name);

    if (ft!=ftUnknown) 
        throw SCAGException("BillAction 'bill:open' : cannot use field for string parameter 'content-type'");
     */

/*
    if (params.Exists("service")) 
    {
        m_sServiceName = params["service"];
        ft = ActionContext::Separate(m_sServiceName,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction '%s' : unrecognized variable prefix '%s' for 'service' parameter",m_sName.c_str(),m_sServiceName.c_str());
    }
*/
    if (params.Exists("status")) 
    {
        m_sStatus = ConvertWStrToStr(params["status"]);
        m_StatusFieldType = ActionContext::Separate(m_sStatus,name);

        if (m_StatusFieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
            if (!(at&atWrite)) 
                throw SCAGException("Action 'bill:open': cannot read property '%s' - no access",m_sStatus.c_str());
        }

        if (m_StatusFieldType == ftConst) 
            throw SCAGException("BillAction 'bill:open' : cannot modify constant property '%s'",m_sStatus.c_str());

        if (m_StatusFieldType == ftUnknown) 
            throw SCAGException("BillAction 'bill:open' : unrecognized variable prefix '%s' for 'status' parameter",m_sStatus.c_str());
            
    }

    if (params.Exists("msg")) 
    {
        m_sMessage = ConvertWStrToStr(params["msg"]);

        m_MsgFieldType = ActionContext::Separate(m_sMessage,name);

        if (m_MsgFieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
            if (!(at&atWrite)) 
                throw SCAGException("Action 'bill:open': cannot read property '%s' - no access",m_sMessage.c_str());
        }

        if (m_MsgFieldType == ftConst) 
            throw SCAGException("BillAction 'bill:open' : cannot modify constant property '%s'",m_sMessage.c_str());

        if (m_MsgFieldType==ftUnknown) 
            throw SCAGException("BillAction 'bill:open' : unrecognized variable prefix '%s' for 'msg' parameter",m_sMessage.c_str());
    }

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

void BillActionOpen::SetBillingStatus(ActionContext& context, std::string& errorMsg, bool isOK)
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

    if (m_sMessage.size() > 0) 
    {
        property = context.getProperty(m_sMessage);

        if (!property) 
        {
            smsc_log_debug(logger,"BillAction 'bill:open' :: Invalid property %s for msg", m_sMessage.c_str());
            return;
        }
        property->setStr(errorMsg);
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

    std::string errorMsg;

    if (m_MediaTypeFieldType != ftUnknown) 
    {
        Property * property = context.getProperty(m_mediaType);
        if (!property) 
        {
            errorMsg = "Invalid property for content-type";
            SetBillingStatus(context, errorMsg, false);
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
            errorMsg = "Invalid property for category";

            SetBillingStatus(context, errorMsg, false);
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
        errorMsg = "operation from ActionContext is invalid";
        smsc_log_error(logger,"BillActionOpen: Fatal error in action - operation from ActionContext is invalid");
        SetBillingStatus(context, errorMsg, false);
        return true;
    }

    BillingManager& bm = BillingManager::Instance();


    TariffRec * tariffRec;
    try {
        tariffRec = context.getTariffRec(category, mediaType);
    } catch (SCAGException& e)
    {
        errorMsg = std::string(e.what());
        smsc_log_warn(logger,"BillAction 'bill:open' cannot process. Delails: %s", e.what()); 
        SetBillingStatus(context, errorMsg, false);
        return true;
    }

    if (!tariffRec) 
    {
        errorMsg = "Cannot find TariffRec";
        smsc_log_warn(logger,"BillAction 'bill:open' cannot process. Delails: Cannot find TariffRec"); 
        SetBillingStatus(context, errorMsg, false);
        return true;
    }

    smsc::inman::interaction::ChargeSms op;

    try 
    {
        context.fillChargeOperation(op, *tariffRec);
        int BillId = bm.ChargeBill(op, *tariffRec);

        operation->attachBill(BillId);

        context.makeBillEvent(TRANSACTION_OPEN, *tariffRec, ev);
    } catch (SCAGException& e)
    {
        errorMsg = std::string(e.what());
        smsc_log_warn(logger,"BillAction 'bill:open' unable to process bill:open. Delails: %s", e.what());
        SetBillingStatus(context, errorMsg, false);
        return true;
    }


    statistics.registerSaccEvent(ev);

    SetBillingStatus(context, errorMsg, true);
    return true;
}

}}

