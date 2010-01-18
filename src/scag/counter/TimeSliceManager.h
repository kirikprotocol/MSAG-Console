#ifndef _SCAG_COUNTER_TIMESLICEMANAGER_H
#define _SCAG_COUNTER_TIMESLICEMANAGER_H

#include <string>
#include "util/TimeSource.h"

namespace scag2 {
namespace counter {

typedef smsc::util::TimeSourceSetup::AbsUSec TSource;
typedef TSource::usec_type usec_type;
static const unsigned usecFactor = 1000000U;

class TimeSliceGroup;

/// An item may be attached to the timeslice group
class TimeSliceItem 
{
    friend class TimeSliceManager;
public:
    virtual ~TimeSliceItem() {}
    virtual void advanceTime( usec_type curtime ) = 0;
    virtual TimeSliceGroup* getTimeSliceGroup() const = 0;
    virtual usec_type getTimeSliceWidth() const = 0;
protected:
    // to be invoked from TimeSliceManager, realization in TimeSliceGroup.cpp
    void setTimeSliceGroup( TimeSliceGroup* grp );
    virtual void doSetTimeSliceGroup( TimeSliceGroup* grp ) = 0;
};


/// A manager
class TimeSliceManager
{
public:
    static const unsigned minSlice = 1; // in sec
    static const usec_type minUSlice = minSlice*usecFactor;
    virtual ~TimeSliceManager() {}
    virtual void addItem( TimeSliceItem& item, unsigned slice ) = 0;
    inline static usec_type sliceToTime( unsigned slice ) {
        return slice*minUSlice;
    }
    inline static unsigned timeToSlice( usec_type tm ) {
        return unsigned(tm/minUSlice);
    }
    inline static unsigned roundSlice( usec_type slice ) {
        return unsigned((slice+minSlice*usecFactor-1)/(minSlice*usecFactor));
    }

protected:
    TimeSliceGroup* createGroup( usec_type slice );
    inline void setGroup( TimeSliceItem& item, TimeSliceGroup* group ) {
        item.setTimeSliceGroup(group);
    }
};

}
}

#endif
