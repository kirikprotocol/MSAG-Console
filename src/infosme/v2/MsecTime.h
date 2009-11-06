#ifndef _SMSC_INFOSME2_MSECTIME_H
#define _SMSC_INFOSME2_MSECTIME_H

// gratefully stolen from scag/util/MsecTime.h

#include <sys/time.h>

namespace smsc {
namespace infosme2 {

class MsecTime
{
public:
    typedef int64_t msectime_type; // signed to allow negative time diffs
    
    MsecTime( msectime_type t0 ) {
        t0_.tv_sec = time_t(t0/1000);
        t0_.tv_usec = suseconds_t((t0%1000)*1000);
    }

    inline msectime_type msectime() const {
        struct timeval tv;
        ::gettimeofday(&tv,0);
        msectime_type t = msectime_type(tv.tv_sec - t0_.tv_sec)*1000 +
            (int(tv.tv_usec) - int(t0_.tv_usec)) / 1000;
        return t;
    }

private:
    struct timeval t0_;
};

typedef MsecTime::msectime_type msectime_type;
inline static msectime_type currentTimeMillis()
{
    static const MsecTime epoch(msectime_type(0));
    return epoch.msectime();
}

}
}

#endif
