#ifndef _SCAG_RE_BASE_BASEFIELD_H
#define _SCAG_RE_BASE_BASEFIELD_H

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

struct BaseField
{
public:
    virtual ~BaseField() {}

    bool isFound() const { return isFound_; }
    const char* getName() const { return paramName_.c_str(); }
    FieldType getType() const { return type_; }
    const char* getStringValue() const { return stringValue_.c_str(); }
    void setStringValue( const char* val ) {
        if ( val ) {
            isFound_ = true;
            type_ = ftUnknown;
            stringValue_ = val;
            postInit();
        }
    }

    // used for writing
    Property* getProperty( ActionContext& ctx ) const { // throw
        if (!isFound_ || isReadonly_ || type_ == ftUnknown ) return 0;
        return ctx.getProperty(stringValue_);
    }

    /// initialize and return exist flag
    virtual bool init( const SectionParams& params, PropertyObject& propobj );

protected:
    BaseField( Action&     baseAction, 
               const char* paramName,
               bool        isRequired,
               bool        isReadonly ) :
    action_(&baseAction), paramName_(paramName),
    isRequired_(isRequired), isReadonly_(isReadonly),
    type_(ftUnknown), isFound_(false) {}

    /// get the string representation of the parameter.
    /// NOTE: that the string lives either in field itself or in the action context property,
    /// so it is safe to return a const char*.
    const char* getString( ActionContext& context ) const; // throw

    virtual void postInit() = 0;

private:
    BaseField();
    BaseField( const BaseField& );
    BaseField& operator = ( const BaseField& );

protected:
    Action*     action_;      // an action this field is attached to
    std::string paramName_;   // the name of the field
    bool        isRequired_;  // if the field is required
    bool        isReadonly_;  // if the field is readonly
    // -- found properties
    FieldType   type_;        // the type of the field
    bool        isFound_;     // if the field is found
    std::string stringValue_; // string value of the parameter
};

}
}
}

#endif /*!_SCAG_RE_BASE_BASEFIELD_H */
