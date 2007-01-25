#include "Action.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {
/*
class ActionParameter
{
protected:
    std::string m_strVariable;

    FieldType m_FieldType;
    bool m_bReadOnly;
    bool m_bExists;
    
public:
    bool prepareValue();
    bool isReadOnly() {return m_bReadOnly;}
    bool Exists() {return m_bExists;}

    void setStrValue(std::string& value);
    void setIntValue(int value);

    std::string& getStrValue();
    int getIntValue();
    ActionParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired);
};
 
*/

ActionParameter::ActionParameter(const SectionParams& params, PropertyObject& propertyObject, const std::string& actionName, const char * paramName, bool isRequired, bool readOnly, Logger * logger)
{
    m_FieldType = ftUnknown;
    m_bReadOnly = readOnly;
    m_pProperty = 0;
    m_pLogger = logger;
    m_actionName = actionName;

    const char * name = 0;

    if (params.Exists(paramName))
    {
        m_bExists = true;

        m_strVariable = params[paramName];
        m_FieldType = ActionContext::Separate(m_strVariable,name);
        AccessType at;

        if (m_FieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);

            if (isReadOnly) 
            {
                if (!(at&atRead)) 
                    throw SCAGException("Action '%s': cannot read property '%s' for '%s' parameter - no access", actionName.c_str(), m_strVariable.c_str(), paramName);
            } else
            {
                if (!(at&atWrite)) 
                    throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access", actionName.c_str(), m_strVariable.c_str(), paramName);
            }
        } 
        else
        {
            if ((m_FieldType == ftConst)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify constant property '%s' for '%s' parameter - no access", actionName.c_str(), m_strVariable.c_str(), paramName);

            if ((m_FieldType == ftUnknown)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify scalar constant '%s' for '%s' parameter", actionName.c_str(), m_strVariable.c_str(), paramName);

            if (m_FieldType == ftSession) 
            {
                bool bReadOnlyProperty = Session::isReadOnlyProperty(name);

                if ((bReadOnlyProperty)&&(!isReadOnly))
                    throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access", actionName.c_str(), m_strVariable.c_str(), paramName);
            }
        }

    }
    else 
    {
        if (isRequired) throw SCAGException("Action '%s' : missing '%s' parameter", actionName.c_str(), paramName);
        m_bExists = false;
    }
}

bool ActionParameter::prepareValue(ActionContext& context)
{
    if (m_FieldType == ftUnknown) return true;

    m_pProperty = context.getProperty(m_strVariable);

    if (!m_pProperty) 
    {
        smsc_log_warn(m_pLogger,"Action '%s':: invalid property '%s'",m_actionName.c_str(),m_strVariable.c_str());
        return false;
    }

    return true;
}

void ActionParameter::setStrValue(std::string& value)
{
    m_pProperty->setStr(value);
}

void ActionParameter::setIntValue(int value)
{
    m_pProperty->setInt(value);
}

std::string ActionParameter::getStrValue()
{
    if (m_FieldType == ftUnknown) return m_strVariable;

    return m_pProperty->getStr();
}

int ActionParameter::getIntValue()
{
    if (m_FieldType == ftUnknown) return atoi(m_strVariable.c_str());

    return m_pProperty->getInt();
}



FieldType Action::CheckParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired, bool isReadOnly, std::string& strParameter, bool& exist)
{
    FieldType fieldType = ftUnknown;

    const char * name = 0;

    if (params.Exists(paramName))
    {
        exist = true;

        strParameter = params[paramName];
        fieldType = ActionContext::Separate(strParameter,name);
        AccessType at;

        if (fieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);

            if (isReadOnly) 
            {
                if (!(at&atRead)) 
                    throw SCAGException("Action '%s': cannot read property '%s' for '%s' parameter - no access", actionName, strParameter.c_str(), paramName);
            } else
            {
                if (!(at&atWrite)) 
                    throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access", actionName, strParameter.c_str(), paramName);
            }
        } 
        else
        {
            if ((fieldType == ftConst)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify constant property '%s' for '%s' parameter - no access", actionName, strParameter.c_str(), paramName);

            if ((fieldType == ftUnknown)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify scalar constant '%s' for '%s' parameter", actionName, strParameter.c_str(), paramName);

            if (fieldType == ftSession) 
            {
                bool bReadOnlyProperty = Session::isReadOnlyProperty(name);

                if ((bReadOnlyProperty)&&(!isReadOnly))
                    throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access", actionName, strParameter.c_str(), paramName);
            }

        }

    }
    else 
    {
        if (isRequired) throw SCAGException("Action '%s' : missing '%s' parameter", actionName, paramName);
        exist = false;
    }
        
    return fieldType;
}

}}}