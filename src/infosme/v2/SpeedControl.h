#ifndef _SMSC_INFOSME2_SPEEDCONTROL_H
#define _SMSC_INFOSME2_SPEEDCONTROL_H

#include <cassert>

namespace smsc {
namespace infosme2 {

class SpeedControl
{
public:
    // the constructor of the speed object,
    // @param speed    -- the necessary speed
    // @param maxDelay -- the maximum delay in msec to the deltaTime
    SpeedControl( unsigned speed = 1, unsigned maxDelay = 1000 ) :
    speed_(speed), nextTime_(0), maxDelay_(maxDelay), wouldSend_(0) {
        assert( speed > 0 );
        assert( maxDelay >= 1000 );
    }

    inline unsigned getSpeed() const { return speed_; }
    inline unsigned getNextTime() const { return nextTime_; }

    // return 0 if object is ready to send, >0 -- the number of ms to sleep
    inline unsigned isReady( unsigned deltaTime ) {
        if ( deltaTime >= nextTime_ ) {
            if ( deltaTime > nextTime_ + maxDelay_ ) { suspend(deltaTime); }
            return 0; 
        }
        register const unsigned mx = deltaTime + maxDelay_;
        if ( mx < nextTime_ ) { suspend(mx); }
        return nextTime_ - deltaTime;
    }

    // suspend execution until deltaTime
    inline void suspend( unsigned deltaTime ) {
        nextTime_ = deltaTime;
        wouldSend_ = nextTime_ * speed_;
    }

    // advance a nextTime in future
    inline void consumeQuant() {
        wouldSend_ += 1000;
        nextTime_ = wouldSend_ / speed_;
    }

    /*
    inline void speedToString( std::string& s ) const {
        char buf[50];
        sprintf(buf," spd=%u next=%u",speed_,nextTime_);
        s.append(buf);
    }
     */

    inline int compare( const SpeedControl& other ) const {
        if ( nextTime_ < other.nextTime_ ) return -1;
        if ( nextTime_ > other.nextTime_ ) return 1;
        if ( speed_ < other.speed_ ) return -1;
        if ( speed_ > other.speed_ ) return 1;
        return 0;
    }

protected:
    unsigned speed_;     // in msg/sec
    unsigned nextTime_;  // the time when the object become ready
    unsigned maxDelay_;
    unsigned wouldSend_;
};

}
}

#endif
