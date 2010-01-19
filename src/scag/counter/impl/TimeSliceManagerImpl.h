#ifndef _SCAG_COUNTER_IMPL_TIMESLICEMANAGERIMPL_H
#define _SCAG_COUNTER_IMPL_TIMESLICEMANAGERIMPL_H

#include <vector>
#include "logger/Logger.h"
#include "scag/counter/TimeSliceManager.h"
#include "scag/counter/TimeSliceGroup.h"
#include "core/buffers/IntHash64.hpp"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"


namespace scag2 {
namespace counter {
namespace impl {

class TimeSliceManagerImpl : public TimeSliceManager, protected smsc::core::threads::Thread
{
private:
    static const usec_type idleSlice = 1*minSlice*usecFactor;
    
public:
    TimeSliceManagerImpl( smsc::logger::Logger* logger,
                          TimeSliceItem*        observer = 0,
                          unsigned              slices = 1 );
    virtual ~TimeSliceManagerImpl();

    void start();
    void stop();

    /// process all groups up to curtime
    virtual int Execute();
    virtual TimeSliceGroup* addItem( TimeSliceItem& item, usec_type slices );

private:
    // typedef std::multimap< usec_type, TimeSliceGroup* > GroupMap;
    typedef smsc::core::buffers::IntHash64< TimeSliceGroup* > GroupHash;
    typedef std::vector< TimeSliceGroup* > GroupVector;
    
private:
    EventMonitor              mon_;
    smsc::logger::Logger*     log_;
    TimeSliceItem*            observer_;
    GroupHash                 groupHash_;
    GroupVector               groupVector_; // incoming
    usec_type                 lastTime_;
    usec_type                 observeSlice_;
    bool                      stopping_;
};

}
}
}

#endif /* !_SCAG_COUNTER_IMPL_TIMESLICEMANAGERIMPL_H */
