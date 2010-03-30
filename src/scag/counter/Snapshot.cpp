#include "Snapshot.h"

namespace scag2 {
namespace counter {

int64_t Snapshot::accumulate( int64_t x, int inc )
{
    smsc::core::synchronization::MutexGuard mg(countMutex_);
    const int64_t pv = integral_;
    int64_t diff = x - lasttime_;
    if ( diff == 0 ) {
        slot_[last_] += inc;
        integral_ += inc;
    } else if ( diff > nbins_ ) {
        first_ = 0;
        last_ = 0;
        slot_[0] = inc;
        integral_ = inc;
        lasttime_ = x;
    } else if ( diff < -int64_t(nbins_) ) {
        first_ = 0;
        last_ = 0;
        slot_[0] = inc;
        integral_ = inc;
        lasttime_ = x;
    } else if ( diff > 0 ) {
        while ( diff-- ) {
            ++last_;
            if ( last_ >= nbins_ ) last_ = 0;
            if ( first_ == last_ ) {
                integral_ -= slot_[first_];
                ++first_;
                if ( first_ >= nbins_ ) first_ = 0;
            }
            slot_[last_] = 0;
        }
        integral_ += inc;
        slot_[last_] += inc;
        lasttime_ = x;
    } else { // diff < 0
        unsigned cur = last_;
        while ( diff++ ) {
            if ( cur == first_ ) {
                if ( first_ == 0 ) first_ = nbins_;
                --first_;
                if ( first_ == last_ ) {
                    integral_ -= slot_[last_];
                    if ( last_ == 0 ) last_ = nbins_;
                    --last_;
                    --lasttime_;
                }
                slot_[first_] = 0;
                cur = first_;
            } else if ( cur == 0 ) {
                cur = nbins_-1;
            }
        }
        slot_[cur] += inc;
        integral_ += inc;
    }
    if ( observer_.get() && pv != integral_ ) {
        observer_->modified(getName().c_str(),oldsev_,getValue(),maxval_);
    }
    return integral_;
}


void Snapshot::getMaxDerivative( int64_t& value, unsigned& nbins )
{
    smsc::core::synchronization::MutexGuard mg(countMutex_);
    nbins = 1;
    value = slot_[last_];
    // smsc_log_debug(log_,"deriv, first_=%u last=%u, val=%u",first_,last_,unsigned(value));
    if ( last_ == first_ ) return;
    unsigned maxspeed = value;
    unsigned curval = value;
    unsigned curbins = 1;
    for ( int p = last_; p != first_; ) {
        if ( p == 0 ) p = nbins_;
        --p;
        ++curbins;
        curval += slot_[p];
        unsigned speed = curval / curbins;
        // smsc_log_debug(log_,"deriv, p=%u, val=%u, bins=%u, spd=%u",
        // p,curval,curbins,speed);
        if (speed>maxspeed) {
            maxspeed = speed;
            nbins = curbins;
            value = curval;
        }
    }
}

}
}
