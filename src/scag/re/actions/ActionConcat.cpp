#include "ActionConcat.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionConcat::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    std::string temp;
    bool bExist;
    FieldType ft;

    ft = CheckParameter(params, propertyObject, "concat", "var", true, false, strVariable, bExist);
    m_fStrFieldType = CheckParameter(params, propertyObject, "concat", "str", true, true, strString, bExist);

    smsc_log_debug(logger,"Action 'concat':: init");
}


bool ActionConcat::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'concat'");

    std::string strArgument;

    if (m_fStrFieldType == ftUnknown) 
    {
        strArgument = strString;
    } else
    {
        Property * property = context.getProperty(strString);

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


    smsc_log_debug(logger,"Action 'concat':: concat result is '%s'", result.c_str());
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