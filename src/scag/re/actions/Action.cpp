#include "Action.h"
#include "scag/re/CommandAdapter.h"

namespace scag { namespace re { namespace actions {

FieldType Action::CheckParameter(const SectionParams& params, PropertyObject& propertyObject, const char * actionName, const char * paramName, bool isRequired, bool isReadOnly, std::string& strParameter, bool& exist)
{
    const char * name = 0;
    if(!(exist = params.Exists(paramName)))
    {
        if(isRequired)
            throw SCAGException("Action '%s' : missing '%s' parameter", actionName, paramName);
        return ftUnknown;
    }
    strParameter = params[paramName];
    FieldType ft = ActionContext::Separate(strParameter,name);
    if(ft == ftField) 
    {
        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);

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
    else if(!isReadOnly && (ft == ftConst || ft == ftUnknown || (ft == ftSession && Session::isReadOnlyProperty(name)))) 
        throw SCAGException("Action '%s': cannot modify constant property '%s' for '%s' parameter - no access", actionName, strParameter.c_str(), paramName);
    return ft;
}

}}}