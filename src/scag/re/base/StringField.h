#ifndef _SCAG_RE_BASE_STRINGFIELD_H
#define _SCAG_RE_BASE_STRINGFIELD_H

#include "BaseField.h"

namespace scag2 {
namespace re {
namespace actions {

struct StringField : public BaseField
{
public:
    StringField( Action&     baseAction, 
                 const char* paramName,
                 bool        isRequired,
                 bool        isReadonly ) :
    BaseField(baseAction,paramName,isRequired,isReadonly) {}

    const char* getValue( ActionContext& ctx ) const { // throw
        return getString( ctx );
    }

};


}
}
}

#endif /*!_SCAG_RE_BASE_STRINGFIELD_H */
