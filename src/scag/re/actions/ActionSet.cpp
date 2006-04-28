#include "ActionSet.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionSet::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    FieldType ft;
    std::string temp;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "set", "var", true, false, temp, bExist);
    strVariable = ConvertWStrToStr(temp);


    valueFieldType = CheckParameter(params, propertyObject, "set", "value", true, true, wstrValue, bExist);
    strValue = ConvertWStrToStr(wstrValue);

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