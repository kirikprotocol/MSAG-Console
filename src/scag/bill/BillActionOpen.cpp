#include "BillActionOpen.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace bill {


void BillActionOpen::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    const char * name = 0;

    if (!logger) 
        logger = Logger::getInstance("scag.bill.actions");

    if (params.Exists("category")) 
        m_category = ConvertWStrToStr(params["category"]);
    else 
        throw SCAGException("BillAction 'bill:open' : missing 'category' parameter");

    if (params.Exists("content-type")) 
        m_mediaType = ConvertWStrToStr(params["content-type"]);
    else 
        throw SCAGException("BillAction 'bill:open' : missing 'content-type' parameter");


    ft = ActionContext::Separate(m_category,name);

    if (ft!=ftUnknown) 
        throw SCAGException("BillAction 'bill:open' : cannot use field for string parameter 'category'");


    ft = ActionContext::Separate(m_mediaType,name);

    if (ft!=ftUnknown) 
        throw SCAGException("BillAction 'bill:open' : cannot use field for string parameter 'content-type'");


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
        ft = ActionContext::Separate(m_sStatus,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction 'bill:open' : unrecognized variable prefix '%s' for 'status' parameter",m_sStatus.c_str());
    }

    if (params.Exists("msg")) 
    {
        m_sMessage = ConvertWStrToStr(params["msg"]);

        ft = ActionContext::Separate(m_sMessage,name);
        if (ft==ftUnknown) 
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

bool BillActionOpen::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'BillActionOpen'...");

    /////////////////////////////////////////////

    Statistics& statistics = Statistics::Instance();

    SACC_BILLING_INFO_EVENT_t ev;


    /////////////////////////////////////////////


    Operation * operation = context.GetCurrentOperation();
    if (!operation) 
    {
        smsc_log_error(logger,"BillActionOpen: Fatal error in action - operation from ActionContext is invalid");
        return false;
    }

    BillingManager& bm = BillingManager::Instance();


    TariffRec * tariffRec;
    try {
        tariffRec = context.getTariffRec(m_category, m_mediaType);
    } catch (SCAGException& e)
    {
        smsc_log_warn(logger,"BillAction 'bill:open' cannot process. Delails: %s", e.what()); 
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
        smsc_log_warn(logger,"BillAction 'bill:open' unable to make bill event. Delails: %s", e.what());
        //TODO: set to status - false
        return true;
    }


    statistics.registerSaccEvent(ev);

    return true;
}

}}

