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
template < class T = smsc::util::TimeSourceSetup::HRTime > class TimeSnapshot : public Snapshot
{
public:
    static const typename T::hrtime_type ticksPerSec = T::ticksPerSec;
    static int getStaticType() { return smsc::util::TypeInfo< TimeSnapshot< T > >::typeValue(); }

    TimeSnapshot( const std::string&      name,
                  typename T::hrtime_type width,
                  unsigned                nbins,
                  counttime_type disposeDelayTime = 0 ) :
    Snapshot(name,nbins,disposeDelayTime)
    {
        assert( width > static_cast<typename T::hrtime_type>(nbins) );
        resol_ = (width + nbins - 1) / nbins;
    }

    virtual int getType() const { return getStaticType(); }

    virtual int64_t accumulate( int64_t x = 0, int w = 1 ) {
        const int64_t bin = int64_t(T::getHRTime()/resol_);
        return Snapshot::accumulate(bin);
    }

    virtual int64_t advance( int64_t x ) {
        const int64_t bin = int64_t(T::getHRTime()/resol_);
        return Snapshot::advance(bin);
    }

protected:
    typename T::hrtime_type resol_;
};

}
}

#endif
