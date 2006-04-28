#include "ActionIndexof.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

void ActionIndexof::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    std::string temp;
    bool bExist;
    FieldType ft;

    m_fVariableFieldType = CheckParameter(params, propertyObject, "indexof", "var", true, true, wstrVariable, bExist);
    strVariable = ConvertWStrToStr(wstrVariable);

    m_fStringFieldType = CheckParameter(params, propertyObject, "indexof", "str", true, true, wstrString, bExist);
    strString = ConvertWStrToStr(wstrString);

    ft = CheckParameter(params, propertyObject, "indexof", "result", true, false, temp, bExist);
    strResult = ConvertWStrToStr(temp);

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