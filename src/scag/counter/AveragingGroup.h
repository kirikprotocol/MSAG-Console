#ifndef _SCAG_COUNTER_AVERAGINGGROUP_H
#define _SCAG_COUNTER_AVERAGINGGROUP_H

#include <list>
#include "Counter.h"
#include "AveragingManager.h"
#include "util/TypeInfo.h"
#include "scag/util/MsecTime.h"

namespace scag2 {
namespace counter {


class AveragingGroup : public Counter
{
    friend class AverageItem;

public:
    static int getStaticType() { return smsc::util::TypeInfo< AveragingGroup >::typeValue(); }

    AveragingGroup( const std::string& name,
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

    virtual int getType() const { return getStaticType(); }

    /// reset all counters attached to the group
    virtual void reset();

    /// average all counters attached to the group
    util::MsecTime::time_type average( util::MsecTime::time_type curTime );

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

}
}

#endif
