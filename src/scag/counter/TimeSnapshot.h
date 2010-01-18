#ifndef _SCAG_COUNTER_TIMESNAPSHOT_H
#define _SCAG_COUNTER_TIMESNAPSHOT_H

#include <cassert>
#include "Snapshot.h"
#include "util/TimeSource.h"

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
                  usec_type               width,
                  unsigned                nbins,
                  counttime_type disposeDelayTime = 0 ) :
    Snapshot(name,nbins,disposeDelayTime),
    group_(0)
    {
        assert( width > usec_type(nbins) );
        resol_ = (width + nbins - 1) / nbins;
    }

    virtual int getType() const { return getStaticType(); }

    virtual void increment( int64_t x = 1, int w = 1 ) {
        const int64_t bin = int64_t(TSource::getUSec()/resol_);
        Snapshot::accumulate(bin,w);
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

protected:
    using Snapshot::accumulate;
    using Snapshot::advanceTo;

    virtual TimeSliceGroup* getTimeSliceGroup() const { return group_; }
    virtual void doSetTimeSliceGroup( TimeSliceGroup* grp ) { group_ = grp; }
    virtual usec_type getTimeSliceWidth() const { return resol_*nbins_; }

protected:
    usec_type       resol_;
    TimeSliceGroup* group_;
};

}
}

#endif
