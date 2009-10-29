#ifndef _SCAG_RE_BASE_TIMEFIELD_H
#define _SCAG_RE_BASE_TIMEFIELD_H

#include "BaseField.h"

namespace scag2 {
namespace re {
namespace actions {

struct TimeField : public BaseField
{
public:
    TimeField( Action&     baseAction, 
               const char* paramName,
               bool        isRequired,
               bool        isReadonly ) :
    BaseField(baseAction,paramName,isRequired,isReadonly), timeValue_(0) {}

    /// time in seconds since epoch
    int64_t getSeconds( ActionContext& context ) const {
        if ( type_ == ftUnknown ) {
            return timeValue_;
        } else {
            return getTimeValue(getString(context));
        }
    }

protected:
    /// initialize and return exist flag
    virtual void postInit() {
        if ( type_ == ftUnknown ) {
            timeValue_ = getTimeValue(stringValue_.c_str());
        }
    }

    // conversion from string to time (seconds)
    int64_t getTimeValue( const char* tv ) const; // throw

private:
    int64_t timeValue_;
};

}
}
}

#endif /*!_SCAG_RE_ACTIONS_IMPL_TIMEFIELD_H */
