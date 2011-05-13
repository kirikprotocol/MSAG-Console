#include "SnapshotCounter.h"

namespace eyeline {
namespace informer {

SnapshotCounter::SnapshotCounter( usectime_type interval,
                                  unsigned nbins ) :
lock_(MTXWHEREAMI),
resolution_(interval/nbins),
lastTime_(0),
integral_(0),
nbins_(nbins), first_(0), last_(0),
slots_(0)
{
    slots_ = new int[nbins];
    slots_[0] = 0;
}


int SnapshotCounter::accumulate( usectime_type currentTime, int inc )
{
    smsc::core::synchronization::MutexGuard mg(lock_);
    currentTime /= resolution_;
    int diff = int(currentTime-lastTime_);
    if ( diff == 0 ) {
        slots_[last_] += inc;
        integral_ += inc;
    } else if ( diff > int(nbins_) ) {
        first_ = 0;
        last_ = 0;
        slots_[0] = inc;
        integral_ = inc;
        lastTime_ = currentTime;
    } else if ( diff < -int(nbins_) ) {
        first_ = 0;
        last_ = 0;
        slots_[0] = inc;
        integral_ = inc;
        lastTime_ = currentTime;
    } else if ( diff > 0 ) {
        while ( diff-- ) {
            ++last_;
            if ( last_ >= nbins_ ) last_ = 0;
            if ( first_ == last_ ) {
                integral_ -= slots_[first_];
                ++first_;
                if ( first_ >= nbins_ ) first_ = 0;
            }
            slots_[last_] = 0;
        }
        integral_ += inc;
        slots_[last_] += inc;
        lastTime_ = currentTime;
    } else { // diff < 0
        unsigned cur = last_;
        while ( diff++ ) {
            if ( cur == first_ ) {
                if ( first_ == 0 ) first_ = nbins_;
                --first_;
                if ( first_ == last_ ) {
                    integral_ -= slots_[last_];
                    if ( last_ == 0 ) last_ = nbins_;
                    --last_;
                    --lastTime_;
                }
                slots_[first_] = 0;
                cur = first_;
            } else if ( cur == 0 ) {
                cur = nbins_-1;
            }
        }
        slots_[cur] += inc;
        integral_ += inc;
    }
    return integral_;
}

}
}
