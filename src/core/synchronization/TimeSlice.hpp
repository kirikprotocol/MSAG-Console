/* ************************************************************************** *
 * Helper class(es) for dealing with time periods measured in different units.
 * ************************************************************************** */
#ifndef __CORE_SYNCHRONIZATION_TIMEOUT_HPP__
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
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
    enum UnitType_e { //timeout measurement unit
        tuSecs = 0       //seconds
        , tuMSecs = 1    //milliseconds
        , tuUSecs = 2    //microseconds
        , tuNSecs = 3    //nanoseconds
    };

    static const char * nmUnit(UnitType_e tu_id)
    {
        switch (tu_id) {
        case tuNSecs: return "NSecs";
        case tuUSecs: return "USecs";
        case tuMSecs: return "MSecs";
        default:;
        }
        return "Secs";
    }

    static void getRealTime(struct timespec & tv)
    {   //Note: on Solaris requires -D__EXTENSIONS__
#ifdef __MACH__
        timeval ltv;
        gettimeofday(&ltv,0);
        tv.tv_sec=ltv.tv_sec;
        tv.tv_nsec=ltv.tv_usec*1000;
        
#else
        clock_gettime(CLOCK_REALTIME, &tv);
#endif
    }

protected:
    UnitType_e  unitId;
    long        tmoVal;

    //helper structure for comparison of two timeouts measured in different units
    struct TwoFraction {
        long vHigh;
        long vLow;

        TwoFraction(long vh = 0, long vl = 0)
            : vHigh(vh), vLow(vl)
        { }

        inline bool operator== (const struct TwoFraction & tm2) const
        {
            return ((vHigh == tm2.vHigh) && (vLow == tm2.vLow));
        }

        inline bool operator< (const struct TwoFraction& tm2) const
        {
            return (vHigh == tm2.vHigh) ? (vLow < tm2.vLow) : (vHigh < tm2.vHigh);
        }
    };

    static long _Factor(UnitType_e tu_id)
    {
        static const long _factorN[4] = { 1, 1000, 1000000L, 1000000000L};
        return _factorN[tu_id];
    }

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
    TimeSlice(long use_tmo = 0, UnitType_e use_unit = tuSecs)
        : unitId(use_unit), tmoVal(use_tmo)
    { }
    ~TimeSlice()
    { }

    const char * nmUnit(void) const { return nmUnit(unitId); }

    UnitType_e Units(void) const { return unitId; }
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

    TimeSlice & operator/ (unsigned use_divisor)
    {
        while ((unitId < tuNSecs) && (unsigned(tmoVal) < use_divisor)) {
            unitId = static_cast<UnitType_e>(unitId + 1);
            tmoVal *= 1000;
        }
        tmoVal = (tmoVal / use_divisor)
                    + ((tmoVal % use_divisor) + use_divisor - 1)/use_divisor;
        return *this;
    }
};

} //synchronization
} //core
} //smsc

#endif /* __CORE_SYNCHRONIZATION_TIMEOUT_HPP__ */

