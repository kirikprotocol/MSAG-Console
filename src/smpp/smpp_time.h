/*
  $Id$
*/


#if !defined  __Cxx_Header__smpp_time_h__
#define  __Cxx_Header__smpp_time_h__

#include <time.h>
#include "util/debug.h"

namespace smsc{
namespace smpp{

static const int SMPP_TIME_BUFFER_LENGTH = 17;

inline bool cTime2SmppTime(time_t tval,char* buffer)
{
  __require__ ( buffer != NULL );
  buffer[0] = 0;
  buffer[16] = 0;
  if ( !tval ) return true;
  struct tm dtm;
  dtm = *gmtime(&tval); // *gmtime(tval);
  //__trace2__("input time: %s",asctime(&dtm));
  dtm.tm_mon+=1;
  dtm.tm_year-=100; // year = x-(1900+100)
  __ret0_if_fail__ ( dtm.tm_mon >= 1 && dtm.tm_mon <= 12 );
  __ret0_if_fail__ ( dtm.tm_year >= 0 && dtm.tm_mon <= 99 );
  __ret0_if_fail__ ( dtm.tm_mday >= 1 && dtm.tm_mday <= 31 );
  __ret0_if_fail__ ( dtm.tm_hour >= 0 && dtm.tm_hour <= 23 );
  __ret0_if_fail__ ( dtm.tm_min >= 0 && dtm.tm_min <= 59 );
  __ret0_if_fail__ ( dtm.tm_sec >=0 && dtm.tm_sec <= 59 );
  int writen =
  snprintf(buffer,SMPP_TIME_BUFFER_LENGTH,
//        YY MM DD hh mm ss t  nn p
        "%02d%02d%02d%02d%02d%02d%01d%02d%c",
        dtm.tm_year,
        dtm.tm_mon,
        dtm.tm_mday,
        dtm.tm_hour,
        dtm.tm_min,
        dtm.tm_sec,
        /*&ignore*/0,
        /*&utc*/0,
        /*&utcfix*/'+');
  //__trace2__("result time: %s\n",buffer);
  //__watch__(writen);
  //__require__(writen == 16);
  __ret0_if_fail__(writen == 16);
  return 1;
}

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
          //  00,00,00,00,00,28,0,00,R
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
  if(scaned!=9)
  {
    __trace2__("time(%d):%16s",scaned,dta);
  }
  if (utcfix == '+' || utcfix == '-' )
	{
		__trace2_if_fail__( scaned == 9, "!!!!! input time: %.16s\n",dta);
		__ret0_if_fail__ ( scaned == 9 );
		__ret0_if_fail__ ( dtm.tm_mon >= 1 && dtm.tm_mon <= 12 );
		dtm.tm_mon-=1;
		__ret0_if_fail__ ( dtm.tm_year >= 0 && dtm.tm_mon <= 99 );
		dtm.tm_year+=100; // year = x-1900
		__ret0_if_fail__ ( dtm.tm_mday >= 1 && dtm.tm_mday <= 31 );
		__ret0_if_fail__ ( dtm.tm_hour >= 0 && dtm.tm_hour <= 23 );
		__ret0_if_fail__ ( dtm.tm_min >= 0 && dtm.tm_min <= 59 );
		__ret0_if_fail__ ( dtm.tm_sec >=0 && dtm.tm_sec <= 59 );
		//__trace2__("result time: %s",asctime(&dtm));
		
		resultTime = mktime(&dtm);
		__ret0_if_fail__ ( resultTime != -1 );
		if ( utcfix == '+' ) resultTime-=utc*60*15;
		else resultTime+=utc*60*15;
	}
	else if ( utcfix == 'R' )
	{
		resultTime = time(0);
		resultTime += ((unsigned long)dtm.tm_year)*60*60*24*365 +
									((unsigned long)dtm.tm_mon)*60*60*24*30 +
									((unsigned long)dtm.tm_mday)*60*60*24 +
									((unsigned long)dtm.tm_hour)*60*60 +
									((unsigned long)dtm.tm_min)*60 +
									((unsigned long)dtm.tm_sec);
		__trace2__("input time: %.16s\n",dta);
		__trace2__("ctime: %s",ctime(&resultTime));
	}
	else
	{ 
		__trace2__("!!!!! input time: %.16s\n",dta);
		__warning__("incorrect time format (utcfix field)");
		return 0;
	}

  return resultTime;
}

};
};

#endif
