#ifndef _SCAG_PERS_UTIL_PERSKEY_H
#define _SCAG_PERS_UTIL_PERSKEY_H

#include "util/int.h"

namespace scag2 {
namespace pers {
namespace util {

// NOTE: this class is deprecated and should not be used in a new code.
union PersKey
{
    const char* skey;
    uint32_t    ikey;
    PersKey() {}
    PersKey( const char* s ) : skey(s) {}
    PersKey( uint32_t i ) : ikey(i) {}
};

}
}
}

namespace scag {
namespace pers {
namespace util {
using scag2::pers::util::PersKey;
}
}
}

#endif /* !_SCAG_PERS_UTIL_PERSKEY_H */

