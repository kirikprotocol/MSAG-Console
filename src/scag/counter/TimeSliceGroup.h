#ifndef _SCAG_COUNTER_TIMESLICEGROUP_H
#define _SCAG_COUNTER_TIMESLICEGROUP_H

#include <list>
#include <algorithm>
#include "logger/Logger.h"
#include "TimeSliceManager.h"
#include "core/synchronization/Mutex.hpp"

namespace scag2 {
namespace counter {

class TimeSliceGroup
{
    friend class TimeSliceManager;
protected:
    TimeSliceGroup( usec_type slice ) :
    slice_(slice) {
        if (!log_) {
            MutexGuard mg(lock_);
            if (!log_) log_ = smsc::logger::Logger::getInstance("count.grp");
        }
    }

public:

    usec_type getSlice() const { return slice_; }

    void addItem( TimeSliceItem& item ) {
        smsc_log_debug(log_,"%u: addItem %p",TimeSliceManager::timeToSlice(slice_), &item);
        smsc::core::synchronization::MutexGuard mg(lock_);
        ItemList::iterator i = std::find( itemList_.begin(), itemList_.end(), &item );
        if ( i == itemList_.end() ) {
            itemList_.push_back(&item);
        }
    }

    void remItem( TimeSliceItem& item ) {
        smsc_log_debug(log_,"%u: remItem %p",TimeSliceManager::timeToSlice(slice_), &item);
        smsc::core::synchronization::MutexGuard mg(lock_);
        ItemList::iterator i = std::find( itemList_.begin(), itemList_.end(), &item );
        if ( i != itemList_.end() ) {
            itemList_.erase(i);
        }
    }

    usec_type advanceTime( usec_type curTime ) {
        smsc_log_debug(log_,"%u: advanceTime(%lld)",
                       TimeSliceManager::timeToSlice(slice_), curTime);
        smsc::core::synchronization::MutexGuard mg(lock_);
        for ( ItemList::iterator i = itemList_.begin(); i != itemList_.end(); ++i ) {
            (*i)->advanceTime( curTime );
        }
        return curTime + slice_;
    }

    // usec_type getWakeTime() const { return wakeTime_; }

    // bool operator < () ( const TimeSliceGroup& grp ) const {
    // return wakeTime_ < grp.wakeTime_;
    // }

private:
    static smsc::logger::Logger* log_;

private:
    typedef std::list< TimeSliceItem* > ItemList;
    smsc::core::synchronization::Mutex lock_;
    ItemList   itemList_;
    // usec_type  wakeTime_;
    usec_type  slice_;
};

/*

class TimeSliceGroup : public TimeSliceItem
{
public:
    // static int getStaticType() { return smsc::util::TypeInfo< AveragingGroup >::typeValue(); }
    TimeSliceGroup( const std::string& name,
                    AveragingManager&  mgr,
                    unsigned           averageEachMsec,
                    counttime_type     disposeDelayTime = 0 ) :
    Counter(name,disposeDelayTime),
    mgr_(mgr),
    startTime_(util::MsecTime::max_time), nextTime_(util::MsecTime::max_time),
    totalInterval_(0), lastInterval_(0),
    avgTime_(averageEachMsec)
    {
        mgr_.addGroup(*this,nextTime_);
        smsc_log_debug(loga_,"group '%s' created",getName().c_str());
    }

    virtual ~AveragingGroup() {
        smsc_log_debug(loga_,"destroying group '%s'",getName().c_str());
        mgr_.remGroup(*this);
    }

    // virtual int getType() const { return getStaticType(); }

    // reset all counters attached to the group
    // virtual void reset();

    virtual void increment( int64_t x = 1, int w = 1 ) {
    }

    virtual bool getValue( Valtype a, int64_t& x ) {
        return false;
    }

    /// average all counters attached to the group
    usec_type advanceTime( usec_type curTime );

    /// return the next averaging time.
    /// NOTE: if the time returned is 0, then no needs to average this group.
    util::MsecTime::time_type nextAverageTime() {
        smsc::core::synchronization::MutexGuard mg(countMutex_);
        return nextTime_;
    }

private:
    void addItem( AverageItem& counter );
    void remItem( AverageItem& counter );

private:
    typedef std::list< AverageItem* > AvgList;
    AvgList                       counters_;
    AveragingManager&             mgr_;
    util::MsecTime::time_type     startTime_; // starting time of the current stats
    util::MsecTime::time_type     nextTime_;  // the time of the next averaging
    util::MsecTime::time_type     totalInterval_; // the time interval of total stats
    util::MsecTime::time_type     lastInterval_;  // the time interval of last stats
    unsigned                      avgTime_;
};
 *
 */

}
}

#endif
