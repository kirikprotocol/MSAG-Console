#ifndef _SCAG_RE_ACTIONS_IMPL_TIMEFIELD_H
#define _SCAG_RE_ACTIONS_IMPL_TIMEFIELD_H

#include "scag/re/base/Action2.h"

namespace scag2 {
namespace re {
namespace actions {

struct TimeField
{
public:
    TimeField() : wait_(0) {}

    /// initialize and return exist flag
    bool init( const SectionParams& params,
               PropertyObject& propobj,
               const char* actionName,
               const char* paramName,
               bool isrequired,
               bool readonly );

    unsigned getTime( const char* actionName, ActionContext& context ) const;

private:
    FieldType   type_;
    std::string name_;
    unsigned    wait_;
};

}
}
}

#endif /*!_SCAG_RE_ACTIONS_IMPL_TIMEFIELD_H */
