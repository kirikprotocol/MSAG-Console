#ifndef _EYELINE_SMPP_UTILITY_H
#define _EYELINE_SMPP_UTILITY_H

#include "util/int.h"

namespace eyeline {
namespace smpp {

// enum {
//     CANREADSM = 1,
//     CANWRITESM = 2
// };

typedef unsigned long long ulonglong;

// typedef uint32_t time_type;     // seconds GMT, since epoch
typedef int64_t  msectime_type; // milliseconds GMT, since epoch

static const unsigned msecPerSec = 1000U;

/// time units
msectime_type currentTimeMillis();
// inline msectime_type currentTimeSeconds() { return time_type(currentTimeMillis() / msecPerSec); }

}
}

#endif
