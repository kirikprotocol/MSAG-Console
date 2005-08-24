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
        if (!params.Exists("status")) throw Exception("BillAction 'open': missing 'status' parameter");
        if (!params.Exists("msg")) throw Exception("BillAction 'open': missing 'msg' parameter");
        m_sServiceName = params["service"];
        m_sStatus      = params["status"];
        m_sMessage     = params["msg"];
    }
    

    FieldType ft;
    const char * name = 0;
    ft = ActionContext::Separate(m_sServiceName,name);
    if (ft==ftUnknown) 
    {
        std::string msg = "BillAction '";
        msg.append(m_sName);
        msg.append("' : unrecognized variable prefix for ");
        msg.append(m_sServiceName);
        throw Exception(msg.c_str());
    }

    ft = ActionContext::Separate(m_sStatus,name);
    if (ft==ftUnknown) 
    {
        std::string msg = "BillAction '";
        msg.append(m_sName);
        msg.append("' : unrecognized variable prefix for ");
        msg.append(m_sStatus);
        throw Exception(msg.c_str());
    }

    ft = ActionContext::Separate(m_sMessage,name);
    if (ft==ftUnknown) 
    {
        std::string msg = "BillAction '";
        msg.append(m_sName);
        msg.append("' : unrecognized variable prefix for ");
        msg.append(m_sMessage);
        throw Exception(msg.c_str());
    }


}


IParserHandler * BillAction::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    std::string msg = "BillAction '";
    msg.append(m_sName);
    msg.append("' cannot include child objects");
    throw Exception(msg.c_str());
}

bool BillAction::FinishXMLSubSection(const std::string& name)
{
    return true;
}


}}

