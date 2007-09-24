#include "ActionAbstractWait.h"

namespace scag { namespace re { namespace actions {

void ActionAbstractWait::InitParameters(const SectionParams& params,PropertyObject propertyObject, smsc::logger::Logger* logger)
{
    FieldType ft;
    bool bExist;
    std::string sType;

    ft = Action::CheckParameter(params, propertyObject, m_ActionName.c_str(), "type", true, true, sType, bExist);
    if (ft!=ftUnknown) throw SCAGException("Action '%s': 'type' parameter must be a scalar constant type", m_ActionName.c_str());

    m_ftTime = Action::CheckParameter(params, propertyObject, m_ActionName.c_str(), "time", true, true, m_sTime, bExist);

    m_opType = Session::getOperationType(sType);

    m_eventHandlerType = propertyObject.HandlerId;
    m_transportType = propertyObject.transport;

    smsc_log_debug(logger,"Action '%s':: init", m_ActionName.c_str());
}

void ActionAbstractWait::RegisterPending(ActionContext& context, smsc::logger::Logger* logger, unsigned int billID)
{
    smsc_log_debug(logger,"Action '%s': registering pending...", m_ActionName.c_str());

    if (!context.checkIfCanSetPending(m_opType, m_eventHandlerType, m_transportType))
        throw SCAGException("Cannot set pending operation (id=%d) for this type of command", m_opType);

    Property * property = 0;
    int wait_time;

    if (m_ftTime!=ftUnknown) 
    {
        property = context.getProperty(m_sTime);

        if (!property) 
            throw SCAGException("Invalid property '%s' to set time", m_sTime.c_str());
        wait_time = property->getInt();
    }
    else
        wait_time = atoi(m_sTime.c_str());

    time_t pendingTime,now;

    time(&now);
    pendingTime = now + wait_time;

    context.AddPendingOperation(m_opType, pendingTime, billID);
    smsc_log_debug(logger,"Action '%s': pending registered time=%d, type=%d, billId=%d", m_ActionName.c_str(), wait_time, m_opType, billID);
}

}}}
