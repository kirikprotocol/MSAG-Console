#ifndef _SCAG_PVSS_CORE_STATISTICS_H
#define _SCAG_PVSS_CORE_STATISTICS_H

#include "scag/util/Time.h"

namespace scag2 {
namespace pvss {
namespace core {

struct Statistics
{
    Statistics( util::msectime_type accTime = 0 ) : accumulationTime(accTime) { reset(); }

    void reset() {
        // NOTE: it works until Statistics has no virtual members
        // register const util::msectime_type a = accumulationTime;
        memset(&startTime,0,((char*)&errors) - ((char*)&startTime) + sizeof(unsigned) );
        // accumulationTime = a;
    }

    /// check if elapsed time overgrows accumulation time (return true if yes).
    bool checkTime( util::msectime_type currentTime ) {
        if ( startTime == 0 ) {
            startTime = currentTime;
            elapsedTime = 0;
            return false;
        } else {
            elapsedTime = currentTime - startTime;
            if ( accumulationTime == 0 ) return false;
            return elapsedTime >= accumulationTime;
        }
    }

    std::string toString() const {
        char buf[128];
        util::msectime_type e10ms;
        unsigned scale;
        if ( elapsedTime > 20000 ) {
            // more than 10 s
            e10ms = elapsedTime/1000;
            scale = 1;
        } else {
            e10ms = elapsedTime/10;
            scale = 100;
        }
        if ( e10ms <= 0 ) e10ms = 1;
        snprintf(buf,sizeof(buf),
                 "elapsed=%lds req/resp/err=%u/%u/%u sent/fail=%u/%u, speed(1/s): req/resp/err=%u/%u/%u sent/fail=%u/%u",
                 long(elapsedTime/1000), requests, responses, errors, sent, failed,
                 unsigned(requests*scale/e10ms), unsigned(responses*scale/e10ms), unsigned(errors*scale/e10ms),
                 unsigned(sent*scale/e10ms), unsigned(failed*scale/e10ms)
                 );
        return buf;
    }

    util::msectime_type accumulationTime;
    util::msectime_type startTime;
    util::msectime_type elapsedTime;
    unsigned            requests;     // requests
    unsigned            sent;         // sent packets
    unsigned            failed;       // failed to send
    unsigned            responses;    // good responses
    unsigned            errors;       // errors
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_STATISTICS_H */
