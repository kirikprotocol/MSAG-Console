/*
	$Id$
*/


#if !defined  __Cxx_Header__smpp_time_h__
#define  __Cxx_Header__smpp_time_h__

#include <time.h>
#include "util/debug.h"

namespace smsc{
namespace smpp{

inline time_t smppTime2CTime(COStr& str)
{
  time_t resultTime = 0;
  int scaned;
  int ignore;
  int utc;
  char utcfix;
  struct tm dtm;
  const char* dta = str.cstr();
  __ret0_if_fail__ ( str != 0 );
  __ret0_if_fail__ ( strlen(dta) == 16 );
  scaned = 
    sscanf(dta,
  //        YY MM DD hh mm ss t  nn p
          "%2d%2d%2d%2d%2d%2d%1d%2d%c",
          &dtm.tm_year,
          &dtm.tm_mon,
          &dtm.tm_mday,
          &dtm.tm_hour,
          &dtm.tm_min,
          &dtm.tm_sec,
          &ignore,
          &utc,
          &utcfix);
  __ret0_if_fail__ ( scaned == 9 );
  __ret0_if_fail__ ( dtm.tm_mon >= 1 && dtm.tm_mon <= 12 ); 
  dtm.tm_mon-1;
  __ret0_if_fail__ ( dtm.tm_mday >= 1 && dtm.tm_mday <= 31 );
  __ret0_if_fail__ ( dtm.tm_hour >= 0 && dtm.tm_hour <= 23 );
  __ret0_if_fail__ ( dtm.tm_min >= 0 && dtm.tm_min <= 59 );
  __ret0_if_fail__ ( dtm.tm_sec >=0 && dtm.tm_sec <= 59 );

  resultTime = mktime(&dtm);
  __ret0_if_fail__ ( resultTime != -1 );
  
  return resultTime;
}

};
};

#endif


