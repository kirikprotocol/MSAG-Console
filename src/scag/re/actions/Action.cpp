#include "Action.h"
#include "scag/re/CommandAdapter.h"


namespace scag { namespace re { namespace actions {

FieldType Action::CheckParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired, bool isReadOnly, std::string& wstrParameter, bool& exist)
{
    std::string temp;
    FieldType fieldType = ftUnknown;

    const char * name = 0;

    if (params.Exists(paramName))
    {
        exist = true;

        wstrParameter = params[paramName];
        temp = ConvertWStrToStr(wstrParameter);

        fieldType = ActionContext::Separate(temp,name);
        AccessType at;

        if (fieldType == ftField) 
        {
            at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);

            if (isReadOnly) 
            {
                if (!(at&atRead)) 
                    throw SCAGException("Action '%s': cannot read property '%s' for '%s' parameter - no access", actionName, temp.c_str(), paramName);
            } else
            {
                if (!(at&atWrite)) 
                    throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access", actionName, temp.c_str(), paramName);
            }
        } 
        else
        {
            if ((fieldType == ftConst)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify constant property '%s' for '%s' parameter - no access", actionName, temp.c_str(), paramName);

            if ((fieldType == ftUnknown)&&(!isReadOnly)) 
                throw SCAGException("Action '%s': cannot modify scalar constant '%s' for '%s' parameter", actionName, FormatWStr(wstrParameter).c_str(), paramName);
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