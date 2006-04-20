#include "ActionSet.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionSet::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");


    if ((!params.Exists("var"))|| (!params.Exists("value"))) throw SCAGException("Action 'set': missing 'var' and 'value' parameters");


    strVariable = ConvertWStrToStr(params["var"]);

    wstrValue = params["value"];
    strValue = ConvertWStrToStr(wstrValue);

    FieldType ft;
    const char * name = 0;

    ft = ActionContext::Separate(strVariable,name); 
    if (ft==ftUnknown) 
        throw InvalidPropertyException("Action 'set': unrecognized variable prefix '%s' for 'var' parameter",strVariable.c_str());

    AccessType at;
    std::string msg = "Action 'set': cannot set property '";

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw InvalidPropertyException("Action 'set': cannot set property '%s' - no access to write",strVariable.c_str());
    } else
        if (ft == ftConst) throw InvalidPropertyException("Action 'set': cannot modify constant variable '%s' - no access to write",strVariable.c_str());


    valueFieldType = ActionContext::Separate(strValue,name);
    if (valueFieldType == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw InvalidPropertyException("Action 'set': cannot read property '%s' - no access",strValue.c_str());
    }


    smsc_log_debug(logger,"Action 'set':: init");
}


bool ActionSet::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'set'");
    Property * property = context.getProperty(strVariable);

    if (!property) 
    {
        smsc_log_warn(logger,"Action 'set':: invalid property '%s'",strVariable.c_str());
        return true;
    }

    if (valueFieldType == ftUnknown) 
    {
        property->setStr(wstrValue);

        smsc_log_debug(logger,"Action 'set': property '%s' set to '%s'",strVariable.c_str(),FormatWStr(wstrValue).c_str());
    }
    else
    {
        Property * val = context.getProperty(strValue);

        if (val) 
        {
            property->setStr(val->getStr());
            smsc_log_debug(logger,"Action 'set': property '%s' set to '%s'",strVariable.c_str(),strValue.c_str());
        }
        else 
            smsc_log_warn(logger,"Action 'set': cannot initialize '%s' with '%s' value - no such property",strVariable.c_str(),FormatWStr(strValue).c_str());
            
    }

    return true;
}

IParserHandler * ActionSet::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'set' cannot include child objects");
}

bool ActionSet::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}