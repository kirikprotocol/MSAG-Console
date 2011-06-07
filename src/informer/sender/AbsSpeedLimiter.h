#ifndef _INFORMER_ABSSPEEDLIMITER_H
#define _INFORMER_ABSSPEEDLIMITER_H

#include <assert.h>
#include "informer/io/Typedefs.h"

namespace eyeline {
namespace informer {

// limiting speed ala timeslotcounter
class AbsSpeedLimiter
{
public:
    AbsSpeedLimiter( unsigned speed,
                     unsigned speedup = 10 ) :       // in percent
                     // unsigned nbins = 20,
                     // unsigned interval = 1,
    // unsigned basespeed = 5 ) :  // in percent of maxspeedperbin
    // nbins_(nbins),
    // bins_(new int[nbins]), 
    // first_(0), last_(0), count_(0),
    // bwid_(interval*tuPerSec/nbins),
    // maxCountPerBin_(0),
    dtmin_(0),
    lastTime_(0),
    speedup_(speedup)
    // basespeed_(basespeed)
    {
        assert(speedup_>0);
        /*
        assert(nbins>10);
        assert(interval>0);
        assert(basespeed>0);
        assert(tuPerSec%nbins==0);
        assert(speed>0);
        bins_[0] = 0;
        lastTime_ = currentTimeMicro();
         */
        setSpeed(speed);
        // basespeed_ = maxCountPerBin_/10
    }
        
        
    ~AbsSpeedLimiter() {
        // delete[] bins_;
    }


    void setSpeed( unsigned speed ) {
        // maxCountPerBin_ = speed/nbins_;
        // increase by 20%
        // maxCountPerBin_ += std::max(maxCountPerBin_/5,2U);
        dtmin_ = tuPerSec*100/speed/(speedup_+100);
    }


    // return a number of usec to wait until limit removed
    unsigned isReady(usectime_type currentTime, usectime_type* udp = 0)
    {
        usectime_type udiff = currentTime - lastTime_;
        if (udp) *udp = udiff;
        if (udiff >= dtmin_) {
            return 0;
        }
        return unsigned(dtmin_ - udiff);
        /*
        const int lastBinValue = consumeQuant(currentTime,0);
        const int maxBinValue = std::min( ((count_ - lastBinValue)*speedup_ +
                                           maxCountPerBin_*basespeed_)/100 + 1,
                                          maxCountPerBin_ );
        if (mbv) { *mbv = maxBinValue; }
        if ( lastBinValue < maxBinValue ) return 0;
        return std::min(unsigned(lastTime_ - currentTime + bwid_/2),unsigned(bwid_));
         */
    }

    void consumeQuant(usectime_type currentTime, int inc)
    {
        lastTime_ = currentTime + std::max(dtmin_*(inc-1),usectime_type(0));
        /*
        const usectime_type udiff = (currentTime-lastTime_) + bwid_/2;
        if (udiff < 0) return bins_[last_];
        unsigned diff = unsigned(udiff)/unsigned(bwid_);
        if (diff==0) {
            bins_[last_] += inc;
            count_ += inc;
            return bins_[last_];
        }
        lastTime_ = lastTime_ + diff*bwid_;
        if (diff>nbins_) {
            first_ = 0;
            last_ = 0;
            bins_[0] = inc;
            count_ = inc;
        } else {
            while (diff--) {
                ++last_;
                if (last_>=nbins_) last_ = 0;
                if (first_ == last_) {
                    count_ -= bins_[first_];
                    ++first_;
                    if (first_>=nbins_) first_=0;
                }
                bins_[last_] = 0;
            }
            count_ += inc;
            bins_[last_] += inc;
        }
        return bins_[last_];
         */
    }

private:
    // unsigned      nbins_;
    // int*          bins_;
    // unsigned      first_, last_;
    // unsigned      count_;
    usectime_type dtmin_;
    usectime_type lastTime_;
    // unsigned      maxCountPerBin_;
    unsigned      speedup_;
    // unsigned      basespeed_;
};

} // informer
} // smsc

#endif
