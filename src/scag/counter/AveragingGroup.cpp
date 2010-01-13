#include <algorithm>
#include "AveragingGroup.h"

namespace scag2 {
namespace counter {

void AveragingGroup::reset()
{
    smsc_log_debug(log_,"resetting group '%s'",getName().c_str());
    smsc::core::synchronization::MutexGuard mg(countMutex_);
    if ( startTime_ == util::MsecTime::max_time ) return; // not initialized yet (no counters added)
    const util::MsecTime::time_type curtime = util::MsecTime::currentTimeMillis();
    for ( AvgList::const_iterator i = counters_.begin(); i != counters_.end(); ++i ) {
        (*i)->reset();
    }
    startTime_ = curtime;
    nextTime_ = curtime + avgTime_;
    totalInterval_ = 0;
    lastInterval_ = 0;
}


util::MsecTime::time_type AveragingGroup::average( util::MsecTime::time_type curtime )
{
    smsc_log_debug(log_,"averaging group '%s' at %lld",getName().c_str(),curtime);
    smsc::core::synchronization::MutexGuard mg(countMutex_);
    if ( startTime_ == util::MsecTime::max_time ) return startTime_;
    for ( AvgList::const_iterator i = counters_.begin(); i != counters_.end(); ++i ) {
        (*i)->average( curtime );
    }
    util::MsecTime::time_type dt = curtime - startTime_;
    totalInterval_ += dt;
    lastInterval_ = dt;
    if ( counters_.empty() ) {
        startTime_ = util::MsecTime::max_time;
        nextTime_ = util::MsecTime::max_time;
    } else {
        startTime_ = curtime;
        nextTime_ = curtime + avgTime_;
    }
    return nextTime_;
}


void AveragingGroup::addItem( AverageItem& counter )
{
    bool needAdd = false;
    util::MsecTime::time_type wakeTime;
    {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        if ( counters_.empty() && startTime_ == util::MsecTime::max_time ) {
            const util::MsecTime::time_type curtime = util::MsecTime::currentTimeMillis();
            startTime_ = curtime; // initialized
            wakeTime = nextTime_ = curtime + avgTime_;
            needAdd = true;
        }
        counters_.push_back(&counter);
    }
    if ( needAdd ) {
        mgr_.addGroup( *this, wakeTime );
    }
}


void AveragingGroup::remItem( AverageItem& counter )
{
    smsc::core::synchronization::MutexGuard mg(countMutex_);
    AvgList::iterator i = std::find(counters_.begin(), counters_.end(), &counter);
    if ( i != counters_.end() ) {
        counters_.erase(i);
    }
}

}
}
