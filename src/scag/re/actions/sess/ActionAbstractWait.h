#ifndef _ACTION_ABSTRACT_WAIT_
#define _ACTION_ABSTRACT_WAIT_

//#include <string>
#include "scag/re/actions/Action.h"


namespace scag { namespace re { namespace actions {

class ActionAbstractWait
{
protected:
    std::string m_sTime;
    int m_opType;
    FieldType m_ftTime;
    int m_eventHandlerType;
    TransportType m_transportType;

    std::string m_ActionName;

    virtual void InitParameters(const SectionParams& params,PropertyObject propertyObject, smsc::logger::Logger* logger);
    virtual void RegisterPending(ActionContext& context, smsc::logger::Logger* logger, unsigned int billID = 0);
public:
    ActionAbstractWait() : m_opType(-1) {};

};






}}}

#endif
