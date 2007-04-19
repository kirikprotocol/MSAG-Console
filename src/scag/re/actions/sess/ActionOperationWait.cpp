#include "ActionOperationWait.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {

IParserHandler * ActionOperationWait::StartXMLSubSection(const std::string& name, const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'operation:wait': cannot have a child object");
}

bool ActionOperationWait::FinishXMLSubSection(const std::string& name)
{
    return true;
}

void ActionOperationWait::init(const SectionParams& params,PropertyObject propertyObject)
{
    m_ActionName = "operation:wait";

    InitParameters(params,propertyObject, logger);
}

bool ActionOperationWait::run(ActionContext& context)
{
    try 
    {
        RegisterPending(context, logger);
    } catch (SCAGException& e)
    {
        smsc_log_error(logger,"Run Action '%s': Cannot process. Details: %s", m_ActionName.c_str(), e.what());
    }

    return true;
}

}}}