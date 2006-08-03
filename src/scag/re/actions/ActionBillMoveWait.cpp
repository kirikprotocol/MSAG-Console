#include "ActionBillMoveWait.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionBillMoveWait::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'operation:bill_move_wait': cannot have a child object");
}

bool ActionBillMoveWait::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionBillMoveWait::init(const SectionParams& params, PropertyObject propertyObject)
{
    m_ActionName = "operation:bill_move_wait";

    logger = Logger::getInstance("scag.re");

    FieldType ft;
    bool bExist;

    ft = CheckParameter(params, propertyObject, m_ActionName.c_str(), "status", true, false, m_sStatus, bExist);
    m_ftMessage = CheckParameter(params, propertyObject, m_ActionName.c_str(), "msg", false, false, m_sMessage, m_MsgExist);

    InitParameters(params,propertyObject);
}

bool ActionBillMoveWait::run(ActionContext& context)
{
    
    bool status;
    try
    {
        status = RegisterPending(context, context.getCurrentOperationBillID());
    } catch (SCAGException& e)
    {
        smsc_log_error(logger, "Action '%s': Cannot process. Details: %s", m_ActionName.c_str(), e.what());
        return true;
    }

    Property * property;

    property = context.getProperty(m_sStatus);

    if (!property) 
    {
        smsc_log_error(logger, "Action '%s': Invalid property '%s'", m_ActionName.c_str(), m_sStatus.c_str());
        return true;
    }

    property->setBool(!status);

    if ((!status)&&(m_MsgExist))
    {
        property = context.getProperty(m_sMessage);
        if (!property) 
        {
            smsc_log_error(logger, "Action '%s': Invalid property '%s'", m_ActionName.c_str(), m_sMessage.c_str());
            return true;
        }
        property->setStr("Cannot register pending operation");
    }
     
    return true;
}




}}}