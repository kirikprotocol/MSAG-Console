#include "BillAction.h"


namespace scag { namespace bill {

bool BillAction::run(ActionContext& context)
{
    return true;
}

void BillAction::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    const char * name = 0;

/*
    if (params.Exists("service")) 
    {
        m_sServiceName = params["service"];
        ft = ActionContext::Separate(m_sServiceName,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction '%s' : unrecognized variable prefix '%s' for 'service' parameter",m_sName.c_str(),m_sServiceName.c_str());
    }

    if (params.Exists("status")) 
    {
        m_sStatus = params["status"];
        ft = ActionContext::Separate(m_sStatus,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction '%s' : unrecognized variable prefix '%s' for 'status' parameter",m_sName.c_str(),m_sStatus.c_str());
    }

    if (params.Exists("msg")) 
    {
        m_sMessage = params["msg"];
        ft = ActionContext::Separate(m_sMessage,name);
        if (ft==ftUnknown) 
            throw SCAGException("BillAction '%s' : unrecognized variable prefix '%s' for 'msg' parameter",m_sName.c_str(),m_sMessage.c_str());
    }
*/    
}


IParserHandler * BillAction::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("BillAction '%s' cannot include child objects",m_sName.c_str());
}

bool BillAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}

