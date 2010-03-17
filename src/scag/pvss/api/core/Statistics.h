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
        char buf[160];
        util::msectime_type e10ms;
        unsigned scale = prescale(e10ms);
        unsigned seconds = unsigned(elapsedTime/1000);
        const unsigned d = unsigned(e10ms/2);
        snprintf(buf,sizeof(buf),
                 "%02u:%02u:%02u  req/resp/err=%u/%u/%u ok/fail=%u/%u\n"
                 "       speed [1/s]: req/resp/err=%u/%u/%u ok/fail=%u/%u",
                 seconds/3600, (seconds/60) % 60, seconds % 60,
                 requests, responses, errors, successes, failures,
                 unsigned((requests*scale+d)/e10ms), unsigned((responses*scale+d)/e10ms),
                 unsigned((errors*scale+d)/e10ms),
                 unsigned((successes*scale+d)/e10ms), unsigned((failures*scale+d)/e10ms));
        return buf;
    }


    /// preparation for better calculation of speed
    unsigned prescale( util::msectime_type& e10ms ) const
    {
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
        return scale;
    }

    util::msectime_type accumulationTime;
    util::msectime_type startTime;
    util::msectime_type elapsedTime;
    unsigned            requests;     // requests
    unsigned            successes;    // server: sent packets, client: good resps
    unsigned            failures;     // server: failed to send, client: bad resps
    unsigned            responses;    // good responses
    unsigned            errors;       // errors
};

} // namespace core
} // namespace pvss
} // namespace scag2

#endif /* !_SCAG_PVSS_CORE_STATISTICS_H */
