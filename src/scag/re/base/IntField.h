#ifndef _SCAG_RE_BASE_INTFIELD_H
#define _SCAG_RE_BASE_INTFIELD_H

#include "BaseField.h"

namespace scag2 {
namespace re {
namespace actions {

struct IntField : public BaseField
{
public:
    IntField( Action&     baseAction, 
              const char* paramName,
              bool        isRequired,
              bool        isReadonly ) :
    BaseField(baseAction,paramName,isRequired,isReadonly), intValue_(0) {}

    int64_t getIntValue() const { return intValue_; }

    /// initialize and return exist flag
    virtual bool init( const SectionParams& params, PropertyObject& propobj ) {
        const bool exist = BaseField::init(params,propobj);
        if ( exist && type_ == ftUnknown ) {
            char* endptr;
            intValue_ = strtoll(stringValue_.c_str(),&endptr,0);
            if ( *endptr != '\0' ) {
                throw SCAGException("Action '%s': wrong %s field",
                                    action_->opname(),paramName_.c_str());
            }
        }
        return exist;
    }

    int64_t getValue( ActionContext& ctx ) const {
        if ( type_ == ftUnknown ) {
            return intValue_;
        } else {
            char* endptr;
            int64_t rv = strtoll(getString(ctx),&endptr,0);
            if ( *endptr != '\0' ) {
                throw SCAGException("Action '%s': wrong %s field",
                                    action_->opname(),paramName_.c_str());
            }
            return rv;
        }
    }

private:
    int64_t intValue_;
};


}
}
}

#endif /*!_SCAG_RE_BASE_INTFIELD_H */
