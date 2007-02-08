#include "ActionLength.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionLength::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    paramVar    = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "length", "var", true, true, logger));
    paramResult = std::auto_ptr<ActionParameter>(new ActionParameter(params, propertyObject, "length", "result", true, false, logger));
    
    smsc_log_debug(logger,"Action 'length':: init");
}


bool ActionLength::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'length'");

    
    if (!paramVar->prepareValue(context)) return true;
    if (!paramResult->prepareValue(context)) return true;

    paramResult->setIntValue(paramVar->getStrValue().size());

    smsc_log_debug(logger,"Action 'length':: result is '%d'", paramResult->getIntValue());

    return true;
}

IParserHandler * ActionLength::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'length' cannot include child objects");
}

bool ActionLength::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}