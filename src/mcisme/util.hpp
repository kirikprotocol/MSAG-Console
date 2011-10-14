#ifndef __SMSC_MCISME_ADVERT_UTIL_HPP__
# define __SMSC_MCISME_ADVERT_UTIL_HPP__

# include <stdio.h>
# include <sys/time.h>
# include <assert.h>

namespace smsc {
namespace mcisme {

inline unsigned long long getTimeInMsec()
{
  struct timeval cur_time;
  int st=gettimeofday(&cur_time, NULL);
  assert(!st);
  return cur_time.tv_sec * 1000 + cur_time.tv_usec / 1000;
}

inline int parseTime(const char* str)
{
  int hour, minute, second;
  if (!str || str[0] == '\0' ||
      sscanf(str, "%d:%02d:%02d",
             &hour, &minute, &second) != 3) return -1;

  return hour*3600+minute*60+second;
}

}}

#endif
