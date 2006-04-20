#include "ActionIndexof.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionIndexof::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    if (!params.Exists("var")) throw SCAGException("Action 'indexof': missing 'var' parameter");
    if (!params.Exists("str")) throw SCAGException("Action 'indexof': missing 'str' parameter");
    if (!params.Exists("result")) throw SCAGException("Action 'indexof': missing 'result' parameter");


    wstrVariable = params["var"];
    strVariable = ConvertWStrToStr(wstrVariable);

    wstrString = params["str"];
    strString = ConvertWStrToStr(wstrString);

    wstrResult = params["result"];
    strResult = ConvertWStrToStr(wstrResult);

    FieldType ft;
    const char * name = 0;

    m_fStringFieldType = ActionContext::Separate(strString,name); 

    AccessType at;

    if (m_fStringFieldType == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw SCAGException("Action 'indexof': cannot read property '%s' - no access",strString.c_str());
    } 

    m_fVariableFieldType = ActionContext::Separate(strVariable,name);
    if (m_fVariableFieldType == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw SCAGException("Action 'indexof': cannot read property '%s' - no access",strVariable.c_str());
    } 


    ft = ActionContext::Separate(strResult,name);

    if (ft == ftUnknown) throw SCAGException("Action 'indexof': cannot modify property '%s' - unknown variable prefix",strResult.c_str());

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw SCAGException("Action 'indexof': cannot modify property '%s' - no access",strResult.c_str());
    } else
        if (ft == ftConst) throw InvalidPropertyException("Action 'indexof' cannot modify constant variable '%s'. Details: no access to write",strResult.c_str());


    ft = ActionContext::Separate(strResult,name);
    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atRead)) 
            throw SCAGException("Action 'indexof': cannot read property '%s' - no access",strResult.c_str());
    }

    smsc_log_debug(logger,"Action 'indexof':: init");
}


bool ActionIndexof::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'indexof'");

    std::string strArgument1;
    std::string strArgument2;

    if (m_fVariableFieldType == ftUnknown) 
    {
        strArgument1 = wstrVariable;
    } else
    {
        Property * property = context.getProperty(strVariable);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'indexof':: invalid property '%s'",strVariable.c_str());
            return true;
        }
        strArgument1 = property->getStr();
    }


    if (m_fStringFieldType == ftUnknown) 
    {
        strArgument2 = wstrString;
    } else
    {
        Property * property = context.getProperty(wstrString);

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'indexof':: invalid property '%s'",wstrString.c_str());
            return true;
        }
        strArgument2 = property->getStr();
    }



    Property * resultProperty = context.getProperty(strResult);
    if (!resultProperty) 
    {
        smsc_log_warn(logger,"Action 'indexof':: invalid property '%s'",strResult.c_str());
        return true;
    }

    int result = -1;
    int i;
    bool isEqual;

    int len1 = strArgument1.size() /2;
    int len2 = strArgument2.size() /2;
/*
    for (i = 0; i < len1; i++) 
    {
        unsigned short ch1 = *(unsigned short *)(strArgument1.data()+i*2);
        smsc_log_debug(logger,"Action 'indexof':: %d", ch1);
    }

    smsc_log_debug(logger,"");

    for (i = 0; i < len2; i++) 
    {
        unsigned short ch1 = *(unsigned short *)(strArgument2.data()+i*2);
        smsc_log_debug(logger,"Action 'indexof':: %d", ch1);
    }
  */

    if ((len1 >= len2)&&(len1 > 0)&&(len2 > 0)) 
    {
        for (i = 0; i < len1 - len2 + 1; i++) 
        {
            isEqual = true;

            for (int j = 0; j < len2; j++)
            {
                unsigned short ch1 = *(unsigned short *)(strArgument1.data()+i*2+j*2);
                unsigned short ch2 = *(unsigned short *)(strArgument2.data()+j*2);

                if (ch1!=ch2) 
                {
                    isEqual = false;
                    break;
                }

            }
            if (isEqual) break;
        }
        if (isEqual) result = i;
    }

    resultProperty->setInt(result);

    smsc_log_debug(logger,"Action 'indexof':: result is '%d'", result);
    return true;
}

IParserHandler * ActionIndexof::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'substr' cannot include child objects");
}

bool ActionIndexof::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}