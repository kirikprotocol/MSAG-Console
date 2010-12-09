#ifndef _SMSC_INFOSME2_SPEEDCONTROL_H
#define _SMSC_INFOSME2_SPEEDCONTROL_H

#include <cassert>
#include "util/int.h"

namespace eyeline {
namespace informer {

template < typename T = unsigned, T TUPERSEC = 1000U > class SpeedControl
{
public:
    // the constructor of the speed object,
    // @param speed    -- the necessary speed
    // @param maxDelay -- the maximum delay in msec to the deltaTime
    SpeedControl( T speed = 1 ) :
    wouldSend_(0), speed_(speed), nextTime_(0) {
        assert( speed > 0 );
    }

    void setSpeed( T speed, T currentTime ) {
        speed_ = speed;
        suspend(currentTime);
    }

    inline T getSpeed() const { return speed_; }
    inline T getNextTime() const { return nextTime_; }

    // return 0 if object is ready to send, >0 -- the number of ms to sleep
    inline T isReady( T currentTime, T maxDelay ) {
        if ( currentTime >= nextTime_ ) {
            if ( currentTime > nextTime_ + maxDelay ) {
                // shift nextTime_ and wouldSend for very slow producer.
                suspend(currentTime);
            }
            return 0; 
        }
        {
            // NOTE: this is required for making a flip of startTime
            register const T mx = currentTime + maxDelay;
            if ( mx < nextTime_ ) { suspend(mx); }
        }
        return nextTime_ - currentTime;
    }

    // suspend execution until deltaTime
    inline void suspend( T currentTime ) {
        nextTime_ = currentTime;
        wouldSend_ = nextTime_*speed_;
    }

    // advance a nextTime in future
    inline void consumeQuant( int number = 1 ) {
        wouldSend_ += TUPERSEC * number;
        nextTime_ = wouldSend_ / speed_;
    }

    inline int compare( const SpeedControl& other ) const {
        if ( nextTime_ < other.nextTime_ ) return -1;
        if ( nextTime_ > other.nextTime_ ) return 1;
        if ( speed_ < other.speed_ ) return -1;
        if ( speed_ > other.speed_ ) return 1;
        return 0;
    }

protected:
    T      wouldSend_;
    T      speed_;     // in msg/sec
    T      nextTime_;  // the time when the object become ready
};

}
}

#endif
