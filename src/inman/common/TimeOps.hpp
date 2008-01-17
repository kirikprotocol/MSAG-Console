#pragma ident "$Id$"
#ifndef _SMSC_CORE_TIME_OPS_HPP
#define _SMSC_CORE_TIME_OPS_HPP

#include <sys/time.h>

//NOTE: timevals should be normalized(microsecs adjusted) !
inline bool operator== (const struct timeval& tm1, const struct timeval& tm2)
{
    return ((tm1.tv_sec == tm2.tv_sec) && (tm1.tv_usec == tm2.tv_usec));
}

inline bool operator< (const struct timeval& tm1, const struct timeval& tm2)
{
    if (tm1.tv_sec > tm2.tv_sec)
        return false;
    if (tm1.tv_sec < tm2.tv_sec)
        return true;
    //equal secs, compare microsecs
    return tm1.tv_usec < tm2.tv_usec;
}

inline bool operator>= (const struct timeval& tm1, const struct timeval& tm2)
{
    if (tm1.tv_sec > tm2.tv_sec)
        return true;
    if (tm1.tv_sec < tm2.tv_sec)
        return false;
    //equal secs, compare microsecs
    return tm1.tv_usec >= tm2.tv_usec;
}

//NOTE: tm1 should be >= tm2
inline struct timeval& operator-= (struct timeval& tm1, const struct timeval& tm2)
{
    if (tm1 >= tm2) {
        if (tm1.tv_usec >= tm2.tv_usec)
            tm1.tv_usec -= tm2.tv_usec;
        else {
            tm1.tv_usec = 1000000L - (tm2.tv_usec - tm1.tv_usec);
            tm1.tv_sec--;
        }
        tm1.tv_sec -= tm2.tv_sec;
    } else
        tm1.tv_sec = tm1.tv_usec = 0;
    return tm1;
}

//Returns absolute difference in seconds
inline long operator- (const struct timeval& tm1, const struct timeval& tm2)
{
    struct timeval tm = tm1;
    if (tm1 >= tm2) {
        tm -= tm2;
    } else {
        tm = tm2;
        tm -= tm1;
    }
    return (tm.tv_usec >= 500000L) ? tm.tv_sec : tm.tv_sec + 1;
}
#endif /* _SMSC_CORE_TIME_OPS_HPP */

