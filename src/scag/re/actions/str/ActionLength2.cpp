#include "ActionLength2.h"
#include "scag/re/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionLength::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft; bool bExist;
    m_varFieldType = CheckParameter(params, propertyObject, "length", "var", true, true, m_strVar, bExist);
    ft = CheckParameter(params, propertyObject, "length", "result", true, false, m_strResult, bExist);
    
    smsc_log_debug(logger,"Action 'length':: init");
}


bool ActionLength::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'length'");

    Property * property = context.getProperty(m_strResult);
    if (!property) {
        smsc_log_warn(logger,"Action 'length':: invalid property '%s'",m_strResult.c_str());
        return true;
    }
    
    if (m_varFieldType == ftUnknown)
    {
        property->setInt(m_strVar.size());
    }
    else
    {
        Property* var = context.getProperty(m_strVar);
        if (var) property->setInt(var->getStr().size());
        else {
            smsc_log_warn(logger,"Action 'length': cannot set '%s' with '%s' value - no such property",
                          m_strResult.c_str(), m_strVar.c_str());
        }
    }
    
    smsc_log_debug(logger,"Action 'length':: result is '%d'", property->getInt());
    return true;
}

IParserHandler * ActionLength::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'length' cannot include child objects");
}

bool ActionLength::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
