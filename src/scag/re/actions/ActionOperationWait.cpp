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

    if (!params.Exists("type")) throw SCAGException("Action 'operation:wait': missing 'type' parameter");
    if (!params.Exists("time")) throw SCAGException("Action 'operation:wait': missing 'time' parameter");


    std::string sType = ConvertWStrToStr(params["type"]);

    const char * name = 0;

    sTime = ConvertWStrToStr(params["time"]);

    FieldType ft;
    AccessType at;

    ft = ActionContext::Separate(sTime,name); 
    if (ft == ftUnknown) throw InvalidPropertyException("Action 'operation:wait': unrecognized variable prefix '%s' for 'time' parameter",sTime.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'operation:wait': cannot read property '%s' - no access",sTime.c_str());
    }

    m_opType = Session::getOperationType(sType);
    

    smsc_log_debug(logger,"Action 'operation:wait':: init...");
}

bool ActionOperationWait::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'operation:wait'...");

    Property * property = 0;
    property = context.getProperty(sTime);

    if (!property) 
    {
        smsc_log_warn(logger,"Action 'operation:wait': invalid property '%s' to set time", sTime.c_str());
        return true;
    }

    time_t pendingTime,now;
 

    time(&now);
    pendingTime = now + property->getInt();

    context.AddPendingOperation(m_opType,pendingTime);


    return true;
}




}}}