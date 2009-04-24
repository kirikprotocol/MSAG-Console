/* ************************************************************************** *
 * Helper class(es) for dealing with time periods measured in different units.
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_TIMEOUT_HPP__
#ident "@(#)$Id$"
#define __CORE_SYNCHRONIZATION_TIMEOUT_HPP__

#ifdef __GNUC__
#include <time.h>
#endif
#include <sys/time.h>

namespace smsc {
namespace core {
namespace synchronization {

class TimeSlice {
public:
    enum UnitType { //timeout measurement unit
        tuSecs = 0       //seconds
        , tuMSecs = 1    //milliseconds
        , tuUSecs = 2    //microseconds
        , tuNSecs = 3    //nanoseconds
    };

    static const char * nmUnit(UnitType tu_id)
    {
        switch (tu_id) {
        case tuNSecs: return "NSecs";
        case tuUSecs: return "USecs";
        case tuMSecs: return "MSecs";
        default:;
        }
        return "Secs";
    }
    static long _Factor(UnitType tu_id)
    {
        static const long _factorN[4] = { 1, 1000, 1000000L, 1000000000L};
        return _factorN[tu_id];
    }

    static void getRealTime(struct timespec & tv)
    {   //Note: on Solaris requires -D__EXTENSIONS__
        clock_gettime(CLOCK_REALTIME, &tv);
    }

protected:
    UnitType    unitId;
    long        tmoVal;

    //helper structure for comparison of two timeouts measured in different units
    struct TwoFraction {
        long vHigh;
        long vLow;

        TwoFraction(long vh = 0, long vl = 0)
            : vHigh(vh), vLow(vl)
        { }

        inline bool operator== (const struct TwoFraction & tm2)
        {
            return ((vHigh == tm2.vHigh) && (vLow == tm2.vLow));
        }

        inline bool operator< (const struct TwoFraction& tm2)
        {
            if (vHigh < vHigh)
                return true;
            if (vHigh > tm2.vHigh)
                return false;
            //equal high fraction, compare lower one
            return vLow < tm2.vLow;
        }
    };

    void normalize(const TimeSlice & tmo2, TwoFraction & v1, TwoFraction & v2) const
    {
        if (Units() < tmo2.Units()) {
            long factor = _Factor(tmo2.Units())/_Factor(Units());
            v1.vHigh = tmoVal;
            v2.vHigh = tmo2.tmoVal/factor;
            v2.vLow = tmo2.tmoVal % factor;
        } else {
            long factor = _Factor(Units())/_Factor(tmo2.Units());
            v2.vHigh = tmo2.tmoVal;
            v1.vHigh = tmoVal/factor;
            v1.vLow = tmoVal % factor;
        }
    }

public:
    //timeout is either in millisecs or seconds
    TimeSlice(long use_tmo = 0, UnitType use_unit = tuSecs)
        : unitId(use_unit), tmoVal(use_tmo)
    { }
    ~TimeSlice()
    { }

    const char * nmUnit(void) const { return nmUnit(unitId); }

    UnitType Units(void) const { return unitId; }
    long     Value(void) const { return tmoVal; }

    //Note: It's a safe to call:
    //      base_time = adjust2Nano(&base_time);
    struct timespec adjust2Nano(const struct timespec * base_time = 0) const
    {
        struct timespec tv = {0, 0};
        if (base_time)
            tv = *base_time;
        else 
            getRealTime(tv);

        if (tmoVal) {
            tv.tv_sec += tmoVal/_Factor(unitId);
            tv.tv_nsec += (tmoVal % _Factor(unitId))*(_Factor(tuNSecs)/_Factor(unitId));
            if (tv.tv_nsec > _Factor(tuNSecs)) {
                ++tv.tv_sec;
                tv.tv_nsec -= _Factor(tuNSecs);
            }
        }
        return tv;
    }

    //Note: It's a safe to call:
    //      base_time = adjust2Nano(&base_time);
    struct timeval adjust2Micro(const struct timeval * base_time = 0) const
    {
        struct timespec tv = { 0, 0};
        if (base_time) {
            tv.tv_sec = base_time->tv_sec;
            tv.tv_nsec = base_time->tv_usec * 1000;
        } else
            getRealTime(tv);

        tv = adjust2Nano(&tv);
        struct timeval  tr;
        tr.tv_sec = tv.tv_sec;
        tr.tv_usec = tv.tv_nsec/1000;
        if ((tv.tv_nsec%1000) >= 500)
            ++tr.tv_usec;
        return tr;
    }

    bool operator== (const TimeSlice & tmo2) const
    {
        if (Units() == tmo2.Units())
            return tmoVal == tmo2.tmoVal;

        TwoFraction v1, v2;
        normalize(tmo2, v1, v2);
        return v1 == v2;
    }

    bool operator< (const TimeSlice & tmo2) const
    {
        if (Units() == tmo2.Units())
            return tmoVal < tmo2.tmoVal;

        TwoFraction v1, v2;
        normalize(tmo2, v1, v2);
        return v1 < v2;
    }
};

} //synchronization
} //core
} //smsc

#endif /* __CORE_SYNCHRONIZATION_TIMEOUT_HPP__ */

