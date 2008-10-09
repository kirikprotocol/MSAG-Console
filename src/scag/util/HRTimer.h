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


/// This class is using HRTimer to create a chain of times spent in different parts of program.
/// NOTE: it is proposed to be run in one thread (not thread-safe).
/// Typical use:
/// fun1() {
///    std::string hrtresult;
///    HRTiming hrt( hrtresult );
///    ... // CPU-consuming part #1
///    hrt.mark( "cpu1" );
///    int x = fun2( args, ..., &hrt ); // pass hrt to a fun1
///    smsc_log_debug( log, "timing: %s", hrtresult.c_str() );
/// }
/// int fun2( args, ..., HRTiming* inhrt )
/// {
///    HRTiming hrt;                      // to use different timing sequence
///    if ( inhrt ) hrt.reset( inhrt );
///    ...
///    if ( inhrt ) hrt.mark( "cpu2" );
///    return xres;
/// }
class HRTiming
{
public:
    HRTiming( HRTiming* inhrt = 0 ) : result_(0) {
        if ( inhrt ) {
            result_ = inhrt->result_;
            resol_ = inhrt->resol_;
        }
    }

    HRTiming( std::string& res, unsigned resolution = 1000 /* ns */ ) : result_(&res), resol_(resolution) {}

    void reset( HRTiming* inhrt = 0 ) {
        if ( inhrt ) {
            timer_ = inhrt->timer_;
            result_ = inhrt->result_;
            resol_ = inhrt->resol_;
        } else {
            timer_.mark();
        }
    }

    void mark( const char* where ) {
        const size_t l = strlen( where ) + 30;
        char buf[ l ];
        unsigned tm = unsigned(timer_.get() / resol_);
        int written = snprintf( buf, l, " %s=%u", where, tm );
        if ( written > 0 ) {
            result_->append( buf, std::min(l-1,unsigned(written)) );
        }
    }

private:
    HRTimer      timer_;
    std::string* result_;  // where we keep results (not owned)
    unsigned     resol_;   // resolution (ns)
};

}
}

#endif /* !_SCAG_UTIL_HRTIMER_H */
