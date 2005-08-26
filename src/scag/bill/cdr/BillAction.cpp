#include "BillAction.h"


namespace scag { namespace bill {

bool BillAction::run(ActionContext& context)
{
    return true;
}

void BillAction::init(const SectionParams& params,PropertyObject propertyObject)
{
    if (params.Exists("service"))
    {
        if (!params.Exists("status")) throw SCAGException("BillAction 'open': missing 'status' parameter");
        if (!params.Exists("msg")) throw SCAGException("BillAction 'open': missing 'msg' parameter");
        m_sServiceName = params["service"];
        m_sStatus      = params["status"];
        m_sMessage     = params["msg"];
    }
    

    FieldType ft;
    const char * name = 0;
    ft = ActionContext::Separate(m_sServiceName,name);
    if (ft==ftUnknown) 
        throw SCAGException("BillAction '",m_sName.c_str(),"' : unrecognized variable prefix '",m_sServiceName.c_str(),"' for 'service' parameter");

    ft = ActionContext::Separate(m_sStatus,name);
    if (ft==ftUnknown) 
        throw SCAGException("BillAction '",m_sName.c_str(),"' : unrecognized variable prefix '",m_sStatus.c_str(),"' for 'status' parameter");

    ft = ActionContext::Separate(m_sMessage,name);
    if (ft==ftUnknown) 
        throw SCAGException("BillAction '",m_sName.c_str(),"' : unrecognized variable prefix '",m_sMessage.c_str(),"' for 'msg' parameter");


}


IParserHandler * BillAction::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    std::string msg = "BillAction '";
    msg.append(m_sName);
    msg.append("' cannot include child objects");
    throw SCAGException(msg.c_str());
}

bool BillAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}

