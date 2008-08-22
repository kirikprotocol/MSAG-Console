#include "ActionSet2.h"
#include "scag/re/base/CommandAdapter2.h"

namespace scag2 {
namespace re {
namespace actions {

void ActionSet::init(const SectionParams& params,PropertyObject propertyObject)
{
    FieldType ft;
    bool bExist;

    ft = CheckParameter(params, propertyObject, "set", "var", true, false, m_strVariable, bExist);


    m_valueFieldType = CheckParameter(params, propertyObject, "set", "value", true, true, m_strValue, bExist);

    smsc_log_debug(logger,"Action 'set':: init");
}


bool ActionSet::run(ActionContext& context)
{
    smsc_log_debug(logger,"Run Action 'set'");
    Property * property = context.getProperty(m_strVariable);

    if (!property) 
    {
        smsc_log_warn(logger,"Action 'set': invalid property '%s'",m_strVariable.c_str());
        return true;
    }

    if (m_valueFieldType == ftUnknown) 
    {
        property->setStr(m_strValue);

        smsc_log_debug(logger,"Action 'set': property '%s' set to '%s'",m_strVariable.c_str(),m_strValue.c_str());
    }
    else
    {
        Property * val = context.getProperty(m_strValue);

        if (val) 
        {
            property->setStr(val->getStr());
            smsc_log_debug(logger,"Action 'set': property '%s' set to val='%s' of '%s'",
                           m_strVariable.c_str(), val->getStr().c_str(), m_strValue.c_str() );
        }
        else 
            smsc_log_warn(logger,"Action 'set': cannot initialize '%s' with '%s' value - no such property",m_strVariable.c_str(),m_strValue.c_str());
            
    }

    return true;
}

IParserHandler * ActionSet::StartXMLSubSection(const std::string& name,const SectionParams& params,const ActionFactory& factory)
{
    throw SCAGException("Action 'set' cannot include child objects");
}

bool ActionSet::FinishXMLSubSection(const std::string& name)
{
    return true;
}

}}}
