#ifndef _SCAG_UTIL_TIME_H
#define _SCAG_UTIL_TIME_H

#include "MsecTime.h"

namespace scag2 {
namespace util {

typedef MsecTime::time_type msectime_type;

inline msectime_type currentTimeMillis()
{
    return MsecTime::currentTimeMillis();
}

} // namespace util
} // namespace scag2

namespace scag {
namespace util {
using scag2::util::currentTimeMillis;
using scag2::util::msectime_type;
}
}

#endif /* !_SCAG_UTIL_TIME_H */
