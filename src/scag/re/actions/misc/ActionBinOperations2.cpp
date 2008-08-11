#include "ActionBinOperations2.h"
#include "scag/re/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

       
void ActionBinOperation::init(const SectionParams& params,PropertyObject propertyObject)
{
    std::string temp;
    bool bExist;
    FieldType ft;

    ft = CheckParameter(params, propertyObject, m_ActionName.c_str(), "var", true, false, strVariable, bExist);

    valueFieldType = CheckParameter(params, propertyObject, m_ActionName.c_str(), "value", false, true, strValue, m_hasValue);

    smsc_log_debug(logger,"Action '%s':: init", m_ActionName.c_str());
}

                                        
bool ActionBinOperation::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action '%s'", m_ActionName.c_str());
    Property * property = context.getProperty(strVariable);

    if (!property) 
    {
        smsc_log_warn(logger,"Action '%s':: invalid property '%s'", m_ActionName.c_str(), strVariable.c_str());
        return true;
    }

    if (!m_hasValue)
        property->setInt(processOperation(int(property->getInt()), 1));
    else 
    {
        if (valueFieldType == ftUnknown) 
        {
            property->setInt(processOperation(int(property->getInt()), atoi(strValue.c_str())));
        }
        else
        {
            Property * val = context.getProperty(strValue);
    
            if (val) 
            {
                property->setInt(processOperation(int(property->getInt()), int(val->getInt())));
                //smsc_log_debug(logger,"Action '%s': property '%s' set to '%s'",strVariable.c_str(),strValue.c_str());
            }
            else 
                smsc_log_warn(logger,"Action '%s': cannot process binary operation with '%s' and '%s' value - no such property", m_ActionName.c_str(), strVariable.c_str(),strValue.c_str());
                
        }
    }

    int r = int(property->getInt());
    smsc_log_debug(logger,"Action '%s': now result is '%d'",m_ActionName.c_str(),r);
    return true;
}

IParserHandler * ActionBinOperation::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action '%s' cannot include child objects", m_ActionName.c_str());
}

bool ActionBinOperation::FinishXMLSubSection(const std::string& name)
{
    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


}}}
