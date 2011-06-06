#ifndef _INFORMER_ABSSPEEDLIMITER_H
#define _INFORMER_ABSSPEEDLIMITER_H

namespace eyeline {
namespace informer {

// limiting speed ala timeslotcounter
class AbsSpeedLimiter
{
public:
    AbsSpeedLimiter( unsigned speed,
                     unsigned nbins = 20,
                     unsigned interval = 1 ) :
    nbins_(nbins), bins_(new int[nbins]), 
    first_(0), last_(0), count_(0),
    bwid_(interval*tuPerSec/nbins),
    maxCount_(speed*interval)
    {
        bins_[0] = 0;
        lastTime_ = currentTimeMicro();
    }
        
        
    ~AbsSpeedLimiter() {
        delete[] bins_;
    }


    void setSpeed( unsigned speed ) {
        maxCount_ = speed*(((nbins+1)*bwid_-1)/tuPerSec);
    }


    // return a number of usec to wait until limit removed
    unsigned isReady(usectime_type currentTime)
    {
        consumeQuant(currentTime,0);
        if ( count_ < maxCount_ ) return 0;
        if ( currentTime < lastTime_ ) {
            return unsigned(lastTime_ - currentTime + bwid_/2);
        }
        int res = int(bwid_/2 - (currentTime - lastTime_));
        if ( res > 0 ) return unsigned(res);
        return unsigned(bwid_/2);
    }


    void consumeQuant(usectime_type currentTime, int inc)
    {
        usectime_type udiff = (currentTime-lastTime_);
        unsigned diff = unsigned((udiff+bwid_/2)/bwid_);
        if (diff==0) {
            bins_[last_] += inc;
            count_ += inc;
            return;
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
    }

private:
    unsigned      nbins_;
    int*          bins_;
    unsigned      first_, last_;
    unsigned      count_;
    usectime_type bwid_;
    usectime_type lastTime_;
    unsigned      maxCount_;
};

} // informer
} // smsc

#endif
