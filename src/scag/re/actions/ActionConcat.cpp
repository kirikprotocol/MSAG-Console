#include "ActionConcat.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionConcat::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    if (!params.Exists("var")) throw SCAGException("Action 'concat': missing 'var' parameter");
    if (!params.Exists("str")) throw SCAGException("Action 'concat': missing 'str' parameter");


    wstrVariable = params["var"];
    strVariable = ConvertWStrToStr(params["var"]);
    

    wstrString = params["str"];
    strString = ConvertWStrToStr(params["str"]);

    FieldType ft;
    const char * name = 0;

    m_fStrFieldType = ActionContext::Separate(strString,name); 

    AccessType at;

    if (m_fStrFieldType == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw SCAGException("Action 'concat': cannot read property '%s' - no access", strString.c_str());
    } 


    ft = ActionContext::Separate(strVariable,name);

    if (ft == ftUnknown) throw SCAGException("Action 'concat': cannot modify property '%s' - unknown variable prefix",strVariable.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw SCAGException("Action 'concat': cannot modify property '%s' - no access",strVariable.c_str());
    } else
        if (ft == ftConst) throw SCAGException("Action 'concat' cannot modify constant variable '%s'. Details: no access to write",strVariable.c_str());


    smsc_log_debug(logger,"Action 'concat':: init");
}


bool ActionConcat::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'substr'");

    std::string strArgument;

    if (m_fStrFieldType == ftUnknown) 
    {
        strArgument = wstrString;
    } else
    {
        Property * property = context.getProperty(wstrString);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'concat':: invalid property '%s'",strString.c_str());
            return true;
        }
        strArgument = property->getStr();
    }

    Property * resultProperty = context.getProperty(strVariable);
    if (!resultProperty) 
    {
        smsc_log_warn(logger,"Action 'concat':: invalid property '%s'",strVariable.c_str());
        return true;
    }

    std::string temp = resultProperty->getStr();
    std::string result;

    result.append(temp.data(), temp.size());
    result.append(strArgument.data(),strArgument.size());

    resultProperty->setStr(result);


    smsc_log_debug(logger,"Action 'concat':: substr result is '%s'", FormatWStr(result).c_str());
    return true;
}

IParserHandler * ActionConcat::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'concat' cannot include child objects");
}

bool ActionConcat::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}