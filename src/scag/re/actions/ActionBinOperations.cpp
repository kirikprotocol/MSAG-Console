#include "ActionBinOperations.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

       
void ActionBinOperation::init(const SectionParams& params,PropertyObject propertyObject)
{
    logger = Logger::getInstance("scag.re");

    if (!params.Exists("var")) throw SCAGException("Action '%s': missing 'var' parameters", m_ActionName.c_str());
    if ((m_valueRequired)&&(!params.Exists("value"))) throw SCAGException("Action '%s': missing 'value' parameters", m_ActionName.c_str());

    strVariable = ConvertWStrToStr(params["var"]);


    m_hasValue = params.Exists("value");

    if (m_hasValue)
    {
        wstrValue = params["value"];
        strValue = ConvertWStrToStr(wstrValue);
    }
 
    FieldType ft;
    const char * name = 0;

    ft = ActionContext::Separate(strVariable,name); 
    if (ft==ftUnknown) 
        throw InvalidPropertyException("Action '%s': unrecognized variable prefix '%s' for 'var' parameter",m_ActionName.c_str(), strVariable.c_str());

    AccessType at;

    if (ft == ftField) 
    {
        at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
        if (!(at&atWrite)) 
            throw InvalidPropertyException("Action '%s': cannot set property '%s' - no access to write",m_ActionName.c_str(), strVariable.c_str());
    }

    if (m_hasValue) 
    {
        valueFieldType = ActionContext::Separate(strValue,name);
        if (valueFieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId,name,propertyObject.transport);
            if (!(at&atRead)) 
                throw InvalidPropertyException("Action '%s': cannot read property '%s' - no access", m_ActionName.c_str(), strValue.c_str());
        }
    }


    smsc_log_debug(logger,"Action 'set':: init");
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
        property->setInt(processOperation(property->getInt(), 1));
    else 
    {
            if (valueFieldType == ftUnknown) 
        {
            property->setInt(processOperation(property->getInt(), atoi(strValue.c_str())));
        }
        else
        {
            Property * val = context.getProperty(strValue);
    
            if (val) 
            {
                property->setInt(processOperation(property->getInt(), val->getInt()));
                //smsc_log_debug(logger,"Action '%s': property '%s' set to '%s'",strVariable.c_str(),strValue.c_str());
            }
            else 
                smsc_log_warn(logger,"Action '%s': cannot process binary operation with '%s' and '%s' value - no such property", m_ActionName.c_str(), strVariable.c_str(),FormatWStr(strValue).c_str());
                
        }
    }

    int r = property->getInt();
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