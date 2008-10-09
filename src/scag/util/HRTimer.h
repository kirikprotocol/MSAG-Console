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
    /// ctor for grandparent
    HRTiming( std::string& res, unsigned resolution = 1000 /* ns */ ) :
    parent_(0), result_(&res), resol_(resolution) {}

    /// ctor for child
    HRTiming( HRTiming* inhrt = 0 ) : parent_(inhrt), result_(0) {
        if ( inhrt ) {
            result_ = inhrt->result_;
            resol_ = inhrt->resol_;
            timer_ = inhrt->timer_;
        } else {
            resol_ = 1000;
        }
    }

    /*
    void reset( HRTiming* inhrt = 0 ) {
        parent_ = inhrt;
        if ( inhrt ) {
            timer_ = inhrt->timer_;
            result_ = inhrt->result_;
            resol_ = inhrt->resol_;
        } else if (result_) {
            result_->erase();
            timer_.mark();
        }
    }
     */

    /// reset for grandparent
    void reset( std::string& res, unsigned resol = 1000 ) {
        result_ = &res;
        resol_ = resol;
        result_->erase();
        timer_.mark();
    }

    inline bool isValid() const {
        return result_;
    }
    
    inline void mark( const char* where ) {
        if ( ! result_ ) return;
        const size_t l = strlen( where ) + 30;
        char buf[ l ];
        unsigned tm = unsigned(timer_.get() / resol_);
        int written = snprintf( buf, l, " %s=%u", where, tm );
        if ( written > 0 ) {
            result_->append( buf, std::min(l-1,unsigned(written)) );
        }
    }

    inline void comment( const char* what ) {
        if ( ! result_ ) return;
        result_->append( what );
    }

    /// NOTE: use if isValid() only
    inline const std::string& result() const {
        return *result_;
    }

    /// stop the chain timing
    inline void stop() {
        if ( parent_ ) parent_->stop();
        result_ = 0;
    }

private:
    HRTiming*    parent_;
    HRTimer      timer_;
    std::string* result_;  // where we keep results (not owned)
    unsigned     resol_;   // resolution (ns)
};

}
}

#endif /* !_SCAG_UTIL_HRTIMER_H */
