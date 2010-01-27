#ifndef _SCAG_COUNTER_TIMESNAPSHOT_H
#define _SCAG_COUNTER_TIMESNAPSHOT_H

#include <cassert>
#include "Snapshot.h"
#include "util/TimeSource.h"
#include "TimeSliceGroup.h"
#include "Manager.h"

namespace scag2 {
namespace counter {

/**
 * Time snapshot counter.
 * 
 * The only difference with 'Snapshot' is that it automatically takes T::getHRTime().
 * 
 */
class TimeSnapshot : public Snapshot, public TimeSliceItem
{
public:
    TimeSnapshot( const std::string&      name,
                  unsigned                nseconds,          // microseconds
                  unsigned                msecresol = 100,   // resolution in milliseconds
                  Observer*               observer = 0,
                  counttime_type disposeDelayTime = 0,
                  unsigned                maxval = 100 ) :
    Snapshot(name,unsigned(nseconds*1000ULL/msecresol),observer,disposeDelayTime,maxval),
    group_(0)
    {
        // assert( width/1000 > usec_type(msecresol) );
        resol_ = msecresol*1000LL;
        smsc_log_debug(loga_,"ctor %p %s '%s'",this,getTypeName(),getName().c_str());
    }

    virtual ~TimeSnapshot() {
        smsc_log_debug(loga_,"dtor %p %s '%s'",this,getTypeName(),getName().c_str());
    }

    virtual TimeSnapshot* clone( const std::string& name,
                                 counttime_type     disposeTime = 0,
                                 unsigned           maxval = 0 ) const
    {
        if (!maxval) maxval = maxval_;
        return new TimeSnapshot(name,unsigned(resol_*nbins_/usecFactor),
                                unsigned(resol_/1000),observer_.get(),disposeTime,maxval);
    }

    virtual int getType() const { return getStaticType(); }
    virtual const char* getTypeName() const { return "timesnapshot"; }

    virtual int64_t increment( int64_t x = 1, int w = 1 ) {
        const int64_t bin = int64_t(TSource::getUSec()/resol_);
        return Snapshot::accumulate(bin,w);
    }

    virtual void advanceTime( usec_type curtime ) {
        smsc_log_debug(log_,"'%s' advancing",getName().c_str());
        const int64_t bin = int64_t(curtime/resol_);
        Snapshot::advanceTo(bin);
    }

    virtual int64_t getValue() const
    {
        return integral_;
    }

    virtual bool getValue( Valtype a, int64_t& value ) const {
        switch (a) {
        case VALUE:
        case COUNT:
        case SUM: {
            value = integral_;
            return true;
        }
        default: break;
        }
        return false;
    }

    virtual unsigned getBaseInterval() const { return unsigned(nbins_*resol_/usecFactor); }

protected:
    using Snapshot::accumulate;
    using Snapshot::advanceTo;

    // virtual TimeSliceGroup* getTimeSliceGroup() const { return group_; }
    // virtual void doSetTimeSliceGroup( TimeSliceGroup* grp ) { group_ = grp; }
    virtual usec_type getTimeSliceWidth() const { return resol_*nbins_; }
    virtual void postRegister( Manager& mgr ) {
        group_ = mgr.getTimeManager().addItem(*this,getTimeSliceWidth());
    }
    virtual void preDestroy( Manager& mgr ) {
        if (group_) {
            group_->remItem(*this);
            group_ = 0;
        }
    }

protected:
    usec_type       resol_;
    TimeSliceGroup* group_;
};

}
}

#endif
