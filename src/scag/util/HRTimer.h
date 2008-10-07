#ifndef _SCAG_UTIL_HRTIMER_H
#define _SCAG_UTIL_HRTIMER_H

#include "util/timeslotcounter.hpp"

namespace scag2 {
namespace util {

class HRTimer
{
public:
    HRTimer() : mark_(0) {}
    ~HRTimer() {}

    /// place a mark (timestamp)
    inline void mark() {
        mark_ = gethrtime();
    }

    /// get the increment from the previous mark (ns)
    /// and place a new mark
    /// NOTE: return 0 if previous mark is not set
    inline hrtime_t get() {
        register hrtime_t newtime = gethrtime();
        std::swap( newtime, mark_ );
        return newtime ? mark_ - newtime : newtime;
    }

private:
    hrtime_t mark_;
};

}
}

#endif /* !_SCAG_UTIL_HRTIMER_H */
