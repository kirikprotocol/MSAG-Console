#include "ActionSubstr.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionSubstr::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    if ((!params.Exists("var")) || (!params.Exists("result"))) throw SCAGException("Action 'substr': missing 'var' or 'result' parameter");
    if ((!params.Exists("begin")) && (!params.Exists("end")))  throw SCAGException("Action 'substr': missing both of 'begin' and 'end' parameters.");


    wstrVariable = params["var"];
    strVariable = ConvertWStrToStr(wstrVariable);

    strResult = ConvertWStrToStr(params["result"]);

    if (params.Exists("begin")) 
    {
            beginIndex = atoi(ConvertWStrToStr(params["begin"]).c_str());
            if (beginIndex < 0) throw SCAGException("Action 'substr': invalid 'begin' parameter.");
    }

    if (params.Exists("end")) 
    {
            endIndex = atoi(ConvertWStrToStr(params["end"]).c_str());
            if (endIndex <= 0) throw SCAGException("Action 'substr': invalid 'end' parameter.");
    }

    FieldType ft;
    const char * name = 0;

    m_fVariableFieldType = ActionContext::Separate(strVariable,name); 

    AccessType at;

    if (m_fVariableFieldType == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw SCAGException("Action 'substr': cannot read property '%s' - no access",strVariable.c_str());
    } 


    ft = ActionContext::Separate(strResult,name);

    if (ft == ftUnknown) throw SCAGException("Action 'substr': cannot modify property '%s' - unknown variable prefix",strResult.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw SCAGException("Action 'substr': cannot modify property '%s' - no access",strResult.c_str());
    } else
        if (ft == ftConst) throw SCAGException("Action 'substr' cannot modify constant variable '%s'. Details: no access to write",strResult.c_str());


    smsc_log_debug(logger,"Action 'substr':: init");
}


bool ActionSubstr::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'substr'");

    std::string strArgument;

    if (m_fVariableFieldType == ftUnknown) 
    {
        strArgument = wstrVariable;
    } else
    {
        Property * property = context.getProperty(strVariable);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",strVariable.c_str());
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
        smsc_log_warn(logger,"Action 'substr':: 'begin' parameter is out of bound '%s' string", FormatWStr(strArgument).c_str());
        return true;
    } 

    if ((endIndex < 0)||((endIndex*2) >= strArgument.size())) end = strArgument.size() - 2;
    else end = endIndex * 2;

    if (begin > end) 
    {
        smsc_log_warn(logger,"Action 'substr':: 'begin' is greater than 'end'");
        return true;
    }
    
    Property * resultProperty = context.getProperty(strResult);
    if (!resultProperty) 
    {
        smsc_log_warn(logger,"Action 'substr':: invalid property '%s'",strResult.c_str());
        return true;
    }

    std::string temp;
    char buff[1024];
    memcpy(buff,strArgument.data() + begin, end - begin + 2);

    temp.assign(buff, end - begin + 2);
    resultProperty->setStr(temp);
    smsc_log_debug(logger,"Action 'substr':: substr result is '%s' (begin=%d, end=%d)", FormatWStr(temp).c_str(), begin, end);


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