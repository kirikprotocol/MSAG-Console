#include "ActionSubstr.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionSubstr::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    FieldType ft;
    bool bExist;
    std::string temp;

    m_fVariableFieldType = CheckParameter(params, propertyObject, "substr", "var", true, true, m_strVariable, bExist);

    ft = CheckParameter(params, propertyObject, "substr", "result", true, false, m_strResult, bExist);

    ft = CheckParameter(params, propertyObject, "substr", "begin", false, true, temp, bExist);
    if (ft!=ftUnknown) throw SCAGException("Action 'substr': 'begin' parameter must be a scalar constant type");

    if (bExist)  
    {
        beginIndex = atoi(temp.c_str());
        if (beginIndex < 0) throw SCAGException("Action 'substr': invalid 'begin' parameter.");
    }


    ft = CheckParameter(params, propertyObject, "substr", "end", false, true, temp, bExist);
    if (ft!=ftUnknown) throw SCAGException("Action 'substr': 'end' parameter must be a scalar constant type");

    if (bExist)  
    {
        endIndex = atoi(temp.c_str());
        if (endIndex <= 0) throw SCAGException("Action 'substr': invalid 'end' parameter.");
    }

    smsc_log_debug(logger,"Action 'substr':: init");
}


bool ActionSubstr::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'substr'");

    std::string strArgument;

    if (m_fVariableFieldType == ftUnknown) 
    {
        strArgument = m_strVariable;
    } else
    {
        Property * property = context.getProperty(m_strVariable);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",m_strVariable.c_str());
            return true;
        }
        strArgument = property->getStr();
    }

    int begin;
    int end;

    if (beginIndex < 0) begin = 0;
    else begin = beginIndex * 2;

    if (begin >= strArgument.size()) 
    {
        smsc_log_warn(logger,"Action 'substr':: 'begin' parameter is out of bound '%s' string", strArgument.c_str());
        return true;
    } 

    if ((endIndex < 0)||((endIndex*2) >= strArgument.size())) end = strArgument.size() - 2;
    else end = endIndex * 2;

    if (begin > end) 
    {
        smsc_log_warn(logger,"Action 'substr':: 'begin' is greater than 'end'");
        return true;
    }
    
    Property * resultProperty = context.getProperty(m_strResult);
    if (!resultProperty) 
    {
        smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",m_strResult.c_str());
        return true;
    }

    std::string temp;
    auto_ptr<char> buff(new char[end - begin + 2]);
    memcpy(buff.get(),strArgument.data() + begin, end - begin + 2);

    temp.assign(buff.get(), end - begin + 2);

    resultProperty->setStr(temp);
    smsc_log_debug(logger,"Action 'substr':: substr result is '%s' (begin=%d, end=%d)", temp.c_str(), begin, end);


    return true;
}

IParserHandler * ActionSubstr::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'substr' cannot include child objects");
}

bool ActionSubstr::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}