/* ************************************************************************** *
 * TimeVal/TimeSpec structures operators. 
 * NOTE: prior to comparison the timevals should be normalized, i.e                                                .
 *       microsecs/nanosecs adjusted to second boundary.
 * ************************************************************************** */
#ifndef SMSC_CORE_TIMERS_TIMEVAL_OPS
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define SMSC_CORE_TIMERS_TIMEVAL_OPS

#include <sys/time.h>

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

//Returns absolute difference in seconds (rounded up)
inline long operator- (const struct timeval& tm1, const struct timeval& tm2)
{
  struct timeval tm;
  if (tm1 < tm2) {
    tm.tv_sec = tm2.tv_sec - tm1.tv_sec;
    tm.tv_usec = tm2.tv_usec - tm1.tv_usec;
  } else {
    tm.tv_sec = tm1.tv_sec - tm2.tv_sec;
    tm.tv_usec = tm1.tv_usec - tm2.tv_usec;
  }
  return (tm.tv_usec >= 500000L) ? tm.tv_sec : tm.tv_sec + 1;
}

inline bool operator== (const struct timespec& tm1, const struct timespec& tm2)
{
  return ((tm1.tv_sec == tm2.tv_sec) && (tm1.tv_nsec == tm2.tv_nsec));
}

inline bool operator< (const struct timespec& tm1, const struct timespec& tm2)
{
  if (tm1.tv_sec > tm2.tv_sec)
    return false;
  if (tm1.tv_sec < tm2.tv_sec)
    return true;
  //equal secs, compare microsecs
  return tm1.tv_nsec < tm2.tv_nsec;
}

#endif /* SMSC_CORE_TIMERS_TIMEVAL_OPS */

