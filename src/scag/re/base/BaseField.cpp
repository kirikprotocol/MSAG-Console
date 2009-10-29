#include "BaseField.h"
#include "CommandAdapter2.h"
#include "scag/sessions/base/Session2.h"

namespace scag2 {
namespace re {
namespace actions {

/// initialize and return exist flag
bool BaseField::init( const SectionParams& params, PropertyObject& propertyObject ) // throw
{
    const char * name = 0;
    if(!(isFound_ = params.Exists(paramName_.c_str()))) {
        if(isRequired_) {
            throw SCAGException("Action '%s' : missing '%s' parameter",
                                action_->opname(), paramName_.c_str());
        }
        return false;
    }
    stringValue_ = params[paramName_.c_str()];
    type_ = ActionContext::Separate(stringValue_,name);
    if(type_ == ftField) {

        AccessType at = CommandAdapter::CheckAccess(propertyObject.HandlerId, name, propertyObject.transport);

        if (isReadonly_) {
            if (!(at&atRead)) {
                throw SCAGException("Action '%s': cannot read property '%s' for '%s' parameter - no access",
                                    action_->opname(), stringValue_.c_str(), paramName_.c_str());
            }
        } else {
            if (!(at&atWrite)) {
                throw SCAGException("Action '%s': cannot write property '%s' for '%s' parameter - no access",
                                    action_->opname(), stringValue_.c_str(), paramName_.c_str());
            }
        }
    } else if ( !isReadonly_ &&
                ( type_ == ftConst || type_ == ftUnknown || Session::isReadOnlyProperty(paramName_.c_str()) )) {
        throw SCAGException("Action '%s': cannot modify constant property '%s' for '%s' parameter - no access",
                            action_->opname(), stringValue_.c_str(), paramName_.c_str());
    }
    postInit();
    return true;
}


/// get the string representation of the parameter.
/// NOTE: that the string lives either in field itself or in the action context property,
/// so it is safe to return a const char*.
const char* BaseField::getString( ActionContext& context ) const
{
    if ( type_ == ftUnknown ) {
        return stringValue_.c_str();
    } else {
        Property* property = context.getProperty( stringValue_ );
        if ( ! property ) {
            throw SCAGException("Action '%s': Invalid property %s for %s",
                                action_->opname(), stringValue_.c_str(), paramName_.c_str());
        }
        return property->getStr().c_str();
    }
}

}
}
}
