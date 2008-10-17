#include "ActionIndexof2.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionIndexof::init(const SectionParams& params,PropertyObject propertyObject)
{
    bool bExist;
    FieldType ft;
    std::string temp;

    m_fVariableFieldType = CheckParameter(params, propertyObject, "indexof", "var", true, true, temp, bExist);
    strVariable.assign(temp.c_str(), temp.size());
    m_fStringFieldType = CheckParameter(params, propertyObject, "indexof", "value", true, true, temp, bExist);
    strString.assign(temp.c_str(), temp.size());
    ft = CheckParameter(params, propertyObject, "indexof", "result", true, false, strResult, bExist);

    smsc_log_debug(logger,"Action 'indexof':: init");
}


bool ActionIndexof::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'indexof'");

    Property::string_type strArgument1;
    Property::string_type strArgument2;

    if (m_fVariableFieldType == ftUnknown) 
    {
        strArgument1 = strVariable;
    } else
    {
        Property * property = context.getProperty(strVariable.c_str());

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'indexof':: invalid property '%s'",strVariable.c_str());
            return true;
        }
        strArgument1 = property->getStr();
    }


    if (m_fStringFieldType == ftUnknown) 
    {
        strArgument2 = strString;
    } else
    {
        Property * property = context.getProperty(strString.c_str());

        if (!property) 
        {
            smsc_log_warn(logger,"Action 'indexof':: invalid property '%s'",strString.c_str());
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

    int len1 = strArgument1.size();
    int len2 = strArgument2.size();

    if ((len1 >= len2)&&(len1 > 0)&&(len2 > 0)) 
    {
        for (i = 0; i < len1 - len2 + 1; i++) 
        {
            isEqual = true;

            for (int j = 0; j < len2; j++)
            {
                char ch1 = *(char *)(strArgument1.data()+i+j);
                char ch2 = *(char *)(strArgument2.data()+j);

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
    throw SCAGException("Action 'indexof' cannot include child objects");
}

bool ActionIndexof::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
