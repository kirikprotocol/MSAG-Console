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
    // virtual TimeSliceGroup* getTimeSliceGroup() const = 0;

    /// return the width of the counter
    virtual usec_type getTimeSliceWidth() const = 0;

protected:
    // to be invoked from TimeSliceManager, realization in TimeSliceGroup.cpp
    // void setTimeSliceGroup( TimeSliceGroup* grp );
    // virtual void doSetTimeSliceGroup( TimeSliceGroup* grp ) = 0;
};


/// A manager
class TimeSliceManager
{
protected:
    static const unsigned minSlice = 1; // in sec
    static const usec_type minUSlice = minSlice*usecFactor;

public:
    virtual ~TimeSliceManager() {}
    // virtual void addItem( TimeSliceItem& item, usec_type slice ) = 0;

    inline static usec_type sliceToTime( unsigned slice ) {
        return slice*minUSlice;
    }
    inline static unsigned timeToSlice( usec_type tm ) {
        return unsigned(tm/minUSlice);
    }
    virtual TimeSliceGroup* addItem( TimeSliceItem& item, usec_type slice ) = 0;

protected:
    inline static unsigned roundSlice( usec_type slice ) {
        return unsigned((slice+minSlice*usecFactor-1)/(minSlice*usecFactor));
    }
    TimeSliceGroup* createGroup( usec_type slice );
};

}
}

#endif
