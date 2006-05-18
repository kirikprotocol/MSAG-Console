#include "ActionOperationWait.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionOperationWait::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'session:close': cannot have a child object");
}

bool ActionOperationWait::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionOperationWait::init(const SectionParams& params,PropertyObject propertyObject)
{

    logger = Logger::getInstance("scag.re");

    FieldType ft;
    std::string temp;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "operation:wait", "type", true, true, temp, bExist);
    if (ft!=ftUnknown) throw SCAGException("Action 'operation:wait': 'type' parameter must be a scalar constant type");
    std::string sType = ConvertWStrToStr(temp);

    m_ftTime = CheckParameter(params, propertyObject, "operation:wait", "time", true, true, temp, bExist);
    m_sTime = ConvertWStrToStr(temp);

    m_opType = Session::getOperationType(sType);
    
    m_eventHandlerType = propertyObject.HandlerId;
    m_transportType = propertyObject.transport;


    smsc_log_debug(logger,"Action 'operation:wait':: init");
}

bool ActionOperationWait::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'operation:wait'...");

    if (!context.checkIfCanSetPending(m_opType, m_eventHandlerType, m_transportType))
    {
        smsc_log_debug(logger,"Run Action 'operation:wait': cannot set pending operation (id=%d) for this type of command",m_opType);
        return true;
    }




    Property * property = 0;
    int wait_time;

    if (m_ftTime!=ftUnknown) 
    {
        property = context.getProperty(m_sTime);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'operation:wait': invalid property '%s' to set time", m_sTime.c_str());
            return true;
        }
        wait_time = property->getInt();
    }
    else
        wait_time = atoi(m_sTime.c_str());

    time_t pendingTime,now;
 
    time(&now);
    pendingTime = now + wait_time;

    context.AddPendingOperation(m_opType,pendingTime);
 
    return true;
}




}}}