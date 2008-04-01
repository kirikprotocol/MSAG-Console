#include "ActionSubstr.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionSubstr::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;  bool bExist;

    m_fVariableFieldType = CheckParameter(params, propertyObject, "substr", "var", true, true, m_strVariable, bExist);
    ft = CheckParameter(params, propertyObject, "substr", "result", true, false, m_strResult, bExist);
    m_ftBegin = CheckParameter(params, propertyObject, "substr", "begin", false, true, m_strBegin, m_bExistBegin);

    if(m_ftBegin==ftUnknown && m_bExistBegin) {
        beginIndex = atoi(m_strBegin.c_str());
        if (beginIndex < 0) throw SCAGException("Action 'substr': invalid 'begin' parameter.");
    }

    m_ftEnd = CheckParameter(params, propertyObject, "substr", "end", false, true, m_strEnd, m_bExistEnd);

    if(m_ftEnd == ftUnknown && m_bExistEnd) {
        endIndex = atoi(m_strEnd.c_str());
        if (endIndex <= 0) throw SCAGException("Action 'substr': invalid 'end' parameter.");
    }

    smsc_log_debug(logger,"Action 'substr':: init begin=%s end=%s", m_strBegin.c_str(), m_strEnd.c_str());
}


bool ActionSubstr::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'substr'");

    std::string strArgument;
    int begin = beginIndex;
    int end = endIndex;

    if (m_fVariableFieldType == ftUnknown) 
        strArgument = m_strVariable;
    else
    {
        Property * property = context.getProperty(m_strVariable);
        if (!property) {
            smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",m_strVariable.c_str());
            return true;
        }
        strArgument = property->getStr();
    }


    if (m_ftBegin != ftUnknown && m_bExistBegin) 
    {
        Property * property = context.getProperty(m_strBegin);
        if (!property) {
            smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",m_strBegin.c_str());
            return true;
        }
        begin = property->getInt();
    }

    if (m_ftEnd != ftUnknown && m_bExistEnd)
    {
        Property * property = context.getProperty(m_strEnd);
        if (!property) {
            smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",m_strEnd.c_str());
            return true;
        }
        end = property->getInt();
    }
    if (begin < 0) begin = 0;

    if (begin >= strArgument.size()) {
        smsc_log_warn(logger,"Action 'substr':: 'begin' parameter is out of bound '%s' string", strArgument.c_str());
        return true;
    } 

    if (end < 0 || end >= strArgument.size()) {
        if (m_bExistEnd) // Warn if end was specified
            smsc_log_warn(logger,"Action 'substr':: 'end' is out of bound '%d>%d' string", end, strArgument.size());
        end = strArgument.size() - 1;
    }

    if (begin > end) {
        smsc_log_warn(logger,"Action 'substr':: 'begin'=%d is greater than 'end'=%d", begin, end);
        return true;
    }
    
    Property * resultProperty = context.getProperty(m_strResult);
    if (!resultProperty) {
        smsc_log_warn(logger,"Action 'substr':: invalid property '%s'", m_strResult.c_str());
        return true;
    }

    std::string temp;
    auto_ptr<char> buff(new char[end - begin + 1]);
    memcpy(buff.get(),strArgument.data() + begin, end - begin + 1);

    temp.assign(buff.get(), end - begin + 1);

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